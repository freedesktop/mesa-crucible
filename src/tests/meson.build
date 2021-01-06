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

test_sources_without_spirv = [
  'bug/108909.c',
  'bug/108911.c',
  'bug/gitlab-4037.c',
  'bench/copy-buffer.c',
  'bench/descriptor-pool-reset.c',
  'bench/queue-submit.c',
  'example/basic.c',
  'example/images.c',
  'example/messages.c',
  'func/amd/amd_common.c',
  'func/buffer/buffer.c',
  'func/cmd-buffer/secondary.c',
  'func/copy/copy-buffer.c',
  'func/desc/binding.c',
  'func/event.c',
  'func/query/timestamp.c',
  'func/interleaved-cmd-buffers.c',
  'func/memory-budget.c',
  'func/renderpass/clear.c',
  'func/memory-fd.c',
  'stress/buffer_limit.c',
  'self/concurrent-output.c',
  'func/calibrated-timestamps.c',
]

test_sources_with_spirv = [
  'bench/multiview.c',
  'bug/104809.c',
  'func/4-vertex-buffers.c',
  'func/depthstencil/basic.c',
  'func/depthstencil/arrayed-clear.c',
  'func/depthstencil/stencil_triangles.c',
  'func/desc/dynamic.c',
  'func/draw-indexed.c',
  'func/gs/basic.c',
  'func/first.c',
  'func/bind-points.c',
  'func/compute.c',
  'func/compute-derivative.c',
  'func/compute-local-id.c',
  'func/compute-num-workgroups.c',
  'func/compute-shared-memory.c',
  'func/compute-subgroup-quad.c',
  'func/miptree/miptree.c',
  'func/multiview.c',
  'func/push-constants/basic.c',
  'func/amd/gcn_shader.c',
  'func/buffer_reference/atomic.c',
  'func/buffer_reference/simple.c',
  'func/shader/constants.c',
  'func/shader/fragcoord.c',
  'func/shader/pack_unpack.c',
  'func/shader_ballot/ext_shader_ballot.c',
  'func/shader_ballot/amd_shader_ballot.c',
  'func/shader_group_vote/ext_shader_subgroup_vote.c',
  'func/ssbo/interleave.c',
  'func/sync/semaphore-fd.c',
  'func/ubo/robust-push-ubo.c',
  'stress/lots-of-surface-state.c',
  'func/uniform-subgroup.c',
]

test_sources_with_gen = [
  'func/depthstencil/stencil_triangles.c',
  'func/miptree/miptree.c',
]

# Check that with & without spirv sources don't intersect

in_both_lists = false
foreach a : test_sources_without_spirv
  foreach b : test_sources_with_spirv
    if a == b
      warning('Test @0@ in both lists!'.format(a))
      in_both_lists = true
    endif
  endforeach
endforeach
if in_both_lists
  error('Found file in both with and without spirv lists!')
endif

# Process tests with generator. *_gen.py

foreach a : test_sources_with_gen
  gen_py = a.substring(0, -2) + '_gen.py'
  genh = gen_py_to_gen_h.process(gen_py, preserve_path_from : src_root)
  test_sources += genh
endforeach

test_sources += files(test_sources_with_gen)

# Process tests with spirv. Create *-spirv.h.

foreach a : test_sources_with_spirv
  test_sources += c_to_spirv_h.process(a, preserve_path_from : src_root)
endforeach

test_sources += files(test_sources_with_spirv)
test_sources += files(test_sources_without_spirv)
