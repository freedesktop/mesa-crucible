#! /usr/bin/env python3

import argparse
import io
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from textwrap import dedent

class ShaderCompileError(RuntimeError):
    def __init__(self, *args):
        super(ShaderCompileError, self).__init__(*args)

target_env_re = re.compile(r'QO_TARGET_ENV\s+(\S+)')
version_re = re.compile(r'QO_VERSION\s+(\S+)')

stage_to_glslang_stage = {
    'VERTEX': 'vert',
    'TESS_CONTROL': 'tesc',
    'TESS_EVALUATION': 'tese',
    'GEOMETRY': 'geom',
    'FRAGMENT': 'frag',
    'COMPUTE': 'comp',
    'TASK': 'task',
    'MESH': 'mesh',
    'RAYGEN': 'rgen',
    'ANY_HIT': 'rahit',
    'CLOSEST_HIT': 'rchit',
    'MISS': 'rmiss',
    'INTERSECTION': 'rint',
    'CALLABLE': 'rcall',
}

class Shader:
    def __init__(self, stage):
        self.glsl = None
        self.stream = io.StringIO()
        self.stage = stage
        self.dwords = None
        self.target_env = ""

    def add_text(self, s):
        self.stream.write(s)

    def finish_text(self, start_line, end_line):
        self.glsl = self.stream.getvalue()
        self.stream = None

        # Handle the QO_EXTENSION macro
        self.glsl = self.glsl.replace('QO_EXTENSION', '#extension')

        # Handle the QO_DEFINE macro
        self.glsl = self.glsl.replace('QO_DEFINE', '#define')

        m = target_env_re.search(self.glsl)
        if m:
            self.target_env = m.group(1)
        self.glsl = self.glsl.replace('QO_TARGET_ENV', '// --target-env')

        if self.glsl.find('QO_VERSION') != -1:
            self.glsl = self.glsl.replace('QO_VERSION', '#version')
        else:
            self.glsl = "#version 450\n" + self.glsl

        self.start_line = start_line
        self.end_line = end_line

    def __run_glslang(self, extra_args=[]):
        stage = stage_to_glslang_stage[self.stage]
        stage_flags = ['-S', stage]

        in_file = tempfile.NamedTemporaryFile(suffix='.'+stage)
        src = self.glsl.encode('utf-8')
        in_file.write(src)
        in_file.flush()
        out_file = tempfile.NamedTemporaryFile(suffix='.spirv')
        args = [glslang, '-H'] + extra_args + stage_flags
        if self.target_env:
            args += ['--target-env', self.target_env]
        args += ['-o', out_file.name, in_file.name]
        with subprocess.Popen(args,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE,
                              stdin = subprocess.PIPE) as proc:

            out, err = proc.communicate(timeout=30)
            in_file.close()

            if proc.returncode != 0:
                # Unfortunately, glslang dumps errors to standard out.
                # However, since we don't really want to count on that,
                # we'll grab the output of both
                message = out.decode('utf-8') + '\n' + err.decode('utf-8')
                raise ShaderCompileError(message.strip())

            out_file.seek(0)
            spirv = out_file.read()
            out_file.close()
            return (spirv, out)

    def compile(self):
        def dwords(f):
            while True:
                dword_str = f.read(4)
                if not dword_str:
                    return
                assert len(dword_str) == 4
                yield struct.unpack('I', dword_str)[0]

        (spirv, assembly) = self.__run_glslang()
        self.dwords = list(dwords(io.BytesIO(spirv)))
        self.assembly = str(assembly, 'utf-8')

    def _dump_glsl_code(self, f):
        # Dump GLSL code for reference.  Use // instead of /* */
        # comments so we don't need to escape the GLSL code.
        # We also ask the compiler to not complain about multi-line comments
        # that are created if a line of the GLSL ends in "\"
        f.write('#pragma GCC diagnostic push //"warning: multi-line comment"\n')
        f.write('#pragma GCC diagnostic ignored "-Wcomment"\n')
        f.write('// GLSL code:\n')
        f.write('//')
        for line in self.glsl.splitlines():
            f.write('\n// {0}'.format(line))
        f.write('\n\n')
        f.write('#pragma GCC diagnostic pop\n')

    def _dump_spirv_code(self, f, var_name):
        f.write('/* SPIR-V Assembly:\n')
        f.write(' *\n')
        for line in self.assembly.splitlines():
            f.write(' * ' + line + '\n')
        f.write(' */\n')

        f.write('static const uint32_t {0}[] = {{'.format(var_name))
        line_start = 0
        while line_start < len(self.dwords):
            f.write('\n    ')
            for i in range(line_start, min(line_start + 6, len(self.dwords))):
                f.write(' 0x{:08x},'.format(self.dwords[i]))
            line_start += 6
        f.write('\n};\n')

    def dump_c_code(self, f):
        f.write('\n\n')
        var_prefix = '__qonos_shader{0}'.format(self.end_line)

        self._dump_glsl_code(f)
        self._dump_spirv_code(f, var_prefix + '_spir_v_src')

        f.write(dedent("""\
            static const QoShaderModuleCreateInfo {0}_info = {{
                .spirvSize = sizeof({0}_spir_v_src),
                .pSpirv = {0}_spir_v_src,
            """.format(var_prefix)))

        if self.stage in ['RAYGEN', 'ANY_HIT', 'CLOSEST_HIT',
                          'MISS', 'INTERSECTION', 'CALLABLE']:
            f.write("    .stage = VK_SHADER_STAGE_{0}_BIT_KHR,\n".format(self.stage))
        elif self.stage in ['TASK', 'MESH']:
            f.write("    .stage = VK_SHADER_STAGE_{0}_BIT_NV,\n".format(self.stage))
        else:
            f.write("    .stage = VK_SHADER_STAGE_{0}_BIT,\n".format(self.stage))

        f.write('};\n')

        f.write('#define __qonos_shader{0}_info __qonos_shader{1}_info\n'\
                .format(self.start_line, self.end_line))

token_exp = re.compile(r'(qoShaderModuleCreateInfoGLSL|qoCreateShaderModuleGLSL|\(|\)|,)')

class Parser:
    def __init__(self, f):
        self.infile = f
        self.paren_depth = 0
        self.shader = None
        self.line_number = 1
        self.shaders = []

        def tokenize(f):
            leftover = ''
            for line in f:
                pos = 0
                while True:
                    m = token_exp.search(line, pos)
                    if m:
                        if m.start() > pos:
                            leftover += line[pos:m.start()]
                        pos = m.end()

                        if leftover:
                            yield leftover
                            leftover = ''

                        yield m.group(0)

                    else:
                        leftover += line[pos:]
                        break

                self.line_number += 1

            if leftover:
                yield leftover

        self.token_iter = tokenize(self.infile)

    def handle_shader_src(self):
        paren_depth = 1
        for t in self.token_iter:
            if t == '(':
                paren_depth += 1
            elif t == ')':
                paren_depth -= 1
                if paren_depth == 0:
                    return

            self.current_shader.add_text(t)

    def handle_macro(self, macro):
        t = next(self.token_iter)
        assert t == '('

        start_line = self.line_number

        if macro == 'qoCreateShaderModuleGLSL':
            # Throw away the device parameter
            t = next(self.token_iter)
            t = next(self.token_iter)
            assert t == ','

        stage = next(self.token_iter).strip()

        t = next(self.token_iter)
        assert t == ','

        self.current_shader = Shader(stage)
        self.handle_shader_src()
        self.current_shader.finish_text(start_line, self.line_number)

        self.shaders.append(self.current_shader)
        self.current_shader = None

    def run(self):
        for t in self.token_iter:
            if t in ('qoShaderModuleCreateInfoGLSL', 'qoCreateShaderModuleGLSL'):
                self.handle_macro(t)

def open_file(name, mode):
    if name == '-':
        if mode == 'w':
            return sys.stdout
        elif mode == 'r':
            return sys.stdin
        else:
            assert False
    else:
        return open(name, mode)

def parse_args():
    description = dedent("""\
        This program scrapes a C file for any instance of the
        qoShaderModuleCreateInfoGLSL and qoCreateShaderModuleGLSL macaros,
        grabs the GLSL source code, compiles it to SPIR-V.  The resulting
        SPIR-V code is written to another C file as an array of 32-bit
        words.

        If '-' is passed as the input file or output file, stdin or stdout
        will be used instead of a file on disc.""")

    p = argparse.ArgumentParser(
            description=description,
            formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('-o', '--outfile', default='-',
                        help='Output to the given file (default: stdout).')
    p.add_argument('--with-glslang', metavar='PATH',
                        default='glslangValidator',
                        dest='glslang',
                        help='Full path to the glslangValidator shader compiler.')
    p.add_argument('infile', metavar='INFILE')

    return p.parse_args()


args = parse_args()
infname = args.infile
outfname = args.outfile
glslang = args.glslang

with open_file(infname, 'r') as infile:
    parser = Parser(infile)
    parser.run()

for shader in parser.shaders:
    shader.compile()

with open_file(outfname, 'w') as outfile:
    outfile.write(dedent("""\
        /* ==========================  DO NOT EDIT!  ==========================
         *             This file is autogenerated by glsl_scraper.py.
         */

        #include <stdint.h>

        #define __QO_SHADER_INFO_VAR2(_line) __qonos_shader ## _line ## _info
        #define __QO_SHADER_INFO_VAR(_line) __QO_SHADER_INFO_VAR2(_line)

        #define qoShaderModuleCreateInfoGLSL(stage, ...)  \\
            __QO_SHADER_INFO_VAR(__LINE__)

        #define qoCreateShaderModuleGLSL(dev, stage, ...) \\
            __qoCreateShaderModule((dev), &__QO_SHADER_INFO_VAR(__LINE__))
        """))

    for shader in parser.shaders:
        shader.dump_c_code(outfile)
