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

man_sources = [
  'crucible-bootstrap.1.txt',
  'crucible-dump-image.1.txt',
  'crucible-help.1.txt',
  'crucible-tutorial.7.txt',
  'crucible-ls-tests.1.txt',
  'crucible-run.1.txt',
  'crucible-version.1.txt',
]

a2x = find_program('a2x')

txt_to_man = generator(
  a2x,
  output : '@BASENAME@',
  arguments : ['--format', 'manpage', '-D', '@BUILD_DIR@', '@INPUT@'],
)

man_pages = []

foreach a : man_sources
  dst = a.substring(0, -4)
  man_page = custom_target(
    dst,
    input : a,
    output : dst,
    command : [a2x, '--format', 'manpage', '-D', '@OUTDIR@', '@INPUT@'],
  )

  man_pages += man_page
endforeach
