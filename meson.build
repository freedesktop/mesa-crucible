# Copyright © 2021 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(
  'crucible',
  ['c'],
  license : 'MIT',
  meson_version : '>= 0.56',
  default_options : ['buildtype=debugoptimized', 'b_ndebug=if-release']
)

cc = meson.get_compiler('c')

pre_args = [
  '-D_DEFAULT_SOURCE',
  '-D_GNU_SOURCE',
  '-D_POSIX_C_SOURCE=200809L',
  '-D_XOPEN_SOURCE=700',
]

foreach a : pre_args
  add_project_arguments(a, language : ['c'])
endforeach

# Generator to run *_gen.py to create *_gen.h

src_root = meson.project_source_root()
prog_python = import('python').find_installation('python3')

gen_py_to_gen_h = generator(
  prog_python,
  output : '@BASENAME@.h',
  arguments : ['@INPUT@', '@OUTPUT@'],
)

test_sources = []

# Generator to create *-spirv.h.

prog_glslang = find_program('glslangValidator')
c_to_spirv_h = generator(
  prog_python,
  output : '@BASENAME@-spirv.h',
  arguments : [meson.project_source_root() + '/misc/glsl_scraper.py',
               '--with-glslang=' + prog_glslang.full_path(),
               '-o', '@OUTPUT@', '@INPUT@'],
)

subdir('data')
subdir('doc')
subdir('src/cmd')
subdir('src/framework')
subdir('src/qonos')
subdir('src/tests')
subdir('src/util')

inc_include = include_directories('include')

dep_libpng16 = dependency('libpng16')
dep_libxml2 = dependency('libxml-2.0')
dep_m = cc.find_library('m', required : false)
dep_thread = dependency('threads')
dep_vulkan = dependency('vulkan')

executable(
  'crucible',
  [command_sources, data_outputs, framework_sources, man_pages, qonos_sources,
   test_sources, util_sources],
  include_directories : [inc_include],
  dependencies: [dep_libpng16, dep_libxml2, dep_m, dep_thread, dep_vulkan],
)

# Dependency checks that will only cause problems at build time

has_python_opencv = run_command(prog_python, '-c', 'import cv2')
if has_python_opencv.returncode() != 0
  error('Python opencv module is required to build crucible.')
endif
