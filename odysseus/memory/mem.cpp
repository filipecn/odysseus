/// Copyright (c) 2021, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file mem.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-06
///
///\brief

#include <odysseus/memory/mem.h>
#include <ponos/log/memory_dump.h>
#include <odysseus/memory/stack_allocator.h>

namespace odysseus {

[[maybe_unused]] u32 mem::cache_l1_size = 64;

void *mem::allocAligned(size_t size, size_t align) {
  // allocate align more bytes to store shift value
  size_t actual_bytes = size + align;
  // allocate unaligned block
  u8 *p_raw_mem = new u8[actual_bytes];
  // align block
  u8 *p_aligned_mem = alignPointer(p_raw_mem, align);
  // if no alignment occurred, shift it up the full 'align' bytes to make room
  // to store the shift
  if (p_aligned_mem == p_raw_mem)
    p_aligned_mem += align;
  // determine the shift and store it
  ptrdiff_t shift = p_aligned_mem - p_raw_mem;
  // alignment can't be greater than 256
  ASSERT(shift > 0 && shift <= 256)
  p_aligned_mem[-1] = static_cast<u8>(shift & 0xFF);
  return p_aligned_mem;
}

void mem::freeAligned(void *p_mem) {
  if (p_mem) {
    u8 *p_aligned_mem = reinterpret_cast<u8 *>(p_mem);
    // extract the shift
    ptrdiff_t shift = p_aligned_mem[-1];
    if (shift == 0)
      shift = 256;
    // back up to the actual allocated address and array-delete it
    u8 *p_raw_mem = p_aligned_mem - shift;
    delete[] p_raw_mem;
  }
}

mem::~mem() {
  delete[] reinterpret_cast<u8 *>(buffer_);
}

OdResult mem::init(std::size_t size_in_bytes) {
  auto &instance = get();
  instance.buffer_ = new u8[size_in_bytes];
  if (!instance.buffer_)
    return OdResult::BAD_ALLOCATION;
  instance.next_ = instance.buffer_;
  instance.size_ = size_in_bytes;
  return OdResult::SUCCESS;
}

std::size_t mem::availableSize() {
  auto &instance = get();
  return instance.size_ - (reinterpret_cast<uintptr_t>(instance.next_) -
      reinterpret_cast<uintptr_t>(instance.buffer_));
}

#ifdef ODYSSEUS_DEBUG
std::string mem::dump(std::size_t start, std::size_t size) {
  auto &instance = get();
  for (auto context_allocator : instance.odb_context_allocators)
    if (context_allocator.type == ContextAllocatorType::STACK_ALLOCATOR)
      instance.odb_regions[context_allocator.region_index].sub_regions =
          reinterpret_cast<StackAllocator *>(
              context_allocator.ptr)->getDataRegions();

  return ponos::MemoryDumper::dump(instance.buffer_
                                       + start, size ? size : instance.size_, 16,
                                   ponos::memory_dumper_options::colored_output,
                                   instance.odb_regions);
}
#endif

}

