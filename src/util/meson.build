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

util_spirv_sources = files(
  'simple_pipeline.c',
)

util_sources = files(
  'cru_cleanup.c',
  'cru_format.c',
  'cru_image.c',
  'cru_vk_image.c',
  'log.c',
  'misc.c',
  'cru_pixel_image.c',
  'cru_png_image.c',
  'cru_ktx_image.c',
  'cru_vec.c',
  'string.c',
  'xalloc.c',
)

foreach a : util_spirv_sources
  util_sources += c_to_spirv_h.process(a, preserve_path_from : src_root)
  util_sources += a
endforeach

util_sources += util_spirv_sources
