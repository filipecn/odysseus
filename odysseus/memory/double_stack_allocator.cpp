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
///\file double_stack_allocator.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-07
///
///\brief

#include <odysseus/memory/double_stack_allocator.h>

namespace odysseus {

#define DSA_EXTRACT_MARKER(HANDLE) \
  ((HANDLE & 0xffffff) - 1)

#define DSA_BUILD_HANDLE(MARKER, SHIFT) \
  ((MARKER + 1u) | (SHIFT << 24u))

DoubleStackAllocator::DoubleStackAllocator(std::size_t capacity_in_bytes, byte *buffer) :
    data_{buffer}, capacity_{capacity_in_bytes}, upper_marker_{capacity_in_bytes},
    using_extern_memory_{buffer != nullptr} {
  if (!buffer)
    resize(capacity_in_bytes);
}

DoubleStackAllocator::~DoubleStackAllocator() {
  if (!using_extern_memory_)
    delete[] data_;
}

std::size_t DoubleStackAllocator::capacityInBytes() const {
  return capacity_;
}

std::size_t DoubleStackAllocator::availableLowerSizeInBytes() const {
  if (threshold_ < capacity_)
    return threshold_ - lower_marker_;
  return upper_marker_ - lower_marker_;
}

std::size_t DoubleStackAllocator::availableUpperSizeInBytes() const {
  if (threshold_ < capacity_)
    return upper_marker_ - threshold_;
  return upper_marker_ - lower_marker_;
}

OdResult DoubleStackAllocator::resize(std::size_t size_in_bytes) {
  if (!using_extern_memory_)
    delete[] data_;
  else
    return OdResult::BAD_OPERATION;
  if (size_in_bytes)
    data_ = new u8[size_in_bytes];
  capacity_ = size_in_bytes;
  threshold_ = capacity_ + 1;
  lower_marker_ = 0;
  upper_marker_ = size_in_bytes;
  ODYSSEUS_DEBUG_CODE(odb_handles.clear();
                          odb_regions.clear();)
  return OdResult::SUCCESS;
}

OdResult DoubleStackAllocator::setThreshold(std::size_t lower_stack_size_in_bytes) {
  if (capacity_ < lower_stack_size_in_bytes)
    return OdResult::OUT_OF_BOUNDS;
  threshold_ = lower_stack_size_in_bytes;
  return OdResult::SUCCESS;
}

MemHandle DoubleStackAllocator::allocateLower(u64 block_size_in_bytes, std::size_t align) {
  std::size_t actual_size =
      block_size_in_bytes + mem::rightAlignShift(reinterpret_cast<uintptr_t >(data_ ) + lower_marker_, align);
  std::size_t shift = actual_size - block_size_in_bytes;
  if (lower_marker_ + actual_size > threshold_ ||
      lower_marker_ + actual_size > capacity_)
    return {0};
  const auto marker = lower_marker_;
  lower_marker_ += actual_size;
  ODYSSEUS_DEBUG_CODE(odb_handles.emplace_back(marker);
                          odb_regions.push_back({
                                                    marker,
                                                    actual_size,
                                                    1,
                                                    ponos::ConsoleColors::color(odb_handles.size()),
                                                    {}
                                                });
  )
  return {DSA_BUILD_HANDLE(marker + shift, shift)};
}

MemHandle DoubleStackAllocator::allocateUpper(u64 block_size_in_bytes, std::size_t align) {
  if (block_size_in_bytes > upper_marker_ ||
      upper_marker_ - block_size_in_bytes < lower_marker_ ||
      (threshold_ < capacity_ && upper_marker_ - block_size_in_bytes < threshold_))
    return {0};
  std::size_t actual_size =
      block_size_in_bytes + mem::leftAlignShift(
          reinterpret_cast<uintptr_t >(data_ ) + upper_marker_ - block_size_in_bytes, align);
  std::size_t shift = actual_size - block_size_in_bytes;
  if (upper_marker_ < actual_size ||
      (threshold_ < capacity_ && upper_marker_ - actual_size < threshold_))
    return {0};
  upper_marker_ -= actual_size;
  ODYSSEUS_DEBUG_CODE(odb_handles.emplace_back(upper_marker_);
                          odb_regions.push_back({
                                                    upper_marker_,
                                                    actual_size,
                                                    1,
                                                    ponos::ConsoleColors::color(odb_handles.size()),
                                                    {}
                                                });
  )
  return {DSA_BUILD_HANDLE(upper_marker_ + shift, shift)};
}

OdResult DoubleStackAllocator::freeToUpperMarker(MemHandle handle) {
  if (upper_marker_ == capacity_)
    return OdResult::BAD_OPERATION;
  if (!handle.id)
    return OdResult::INVALID_INPUT;
  upper_marker_ = DSA_EXTRACT_MARKER(handle.id);
  return OdResult::SUCCESS;
}

OdResult DoubleStackAllocator::freeToLowerMarker(MemHandle handle) {
  if (!lower_marker_)
    return OdResult::BAD_OPERATION;
  if (!handle.id)
    return OdResult::INVALID_INPUT;
  lower_marker_ = DSA_EXTRACT_MARKER(handle.id);
  return OdResult::SUCCESS;
}

void DoubleStackAllocator::clear() {
  ODYSSEUS_DEBUG_CODE(odb_handles.clear();
                          odb_regions.clear();)
  lower_marker_ = 0;
  upper_marker_ = capacity_;
}

#ifdef ODYSSEUS_DEBUG
void DoubleStackAllocator::dump(std::size_t start, std::size_t size) const {
  ponos::MemoryDumper::dump(data_ + start, size ? size : capacity_ - start,
                            64, ponos::memory_dumper_options::colored_output
                                | ponos::memory_dumper_options::cache_align,
                            odb_regions);
}

std::vector<ponos::MemoryDumper::Region> DoubleStackAllocator::getRegions() {
  std::vector<ponos::MemoryDumper::Region> regions = {
      { // data_
          0,
          sizeof(data_),
          1,
          ponos::ConsoleColors::color(1),
          {}
      },
      { // capacity_
          sizeof(data_),
          sizeof(capacity_),
          1,
          ponos::ConsoleColors::color(2),
          {}
      },
      { // lower marker_
          offsetof(DoubleStackAllocator, lower_marker_),
          sizeof(lower_marker_),
          1,
          ponos::ConsoleColors::color(3),
          {}
      },
      { // upper marker_
          offsetof(DoubleStackAllocator, upper_marker_),
          sizeof(upper_marker_),
          1,
          ponos::ConsoleColors::color(4),
          {}
      },
      { // threshold_
          offsetof(DoubleStackAllocator, threshold_),
          sizeof(threshold_),
          1,
          ponos::ConsoleColors::color(5),
          {}
      },
      { // using_exten_memory_
          offsetof(DoubleStackAllocator, using_extern_memory_),
          sizeof(using_extern_memory_),
          1,
          ponos::ConsoleColors::color(6),
          {}
      },
      { // db_handles
          offsetof(DoubleStackAllocator, odb_handles),
          sizeof(odb_handles),
          1,
          ponos::ConsoleColors::color(7),
          {}
      },
      { // db_regions
          offsetof(DoubleStackAllocator, odb_regions),
          sizeof(odb_regions),
          1,
          ponos::ConsoleColors::color(8),
          {}
      },
  };
  return std::move(regions);
}

std::vector<ponos::MemoryDumper::Region> DoubleStackAllocator::getDataRegions() {
  return odb_regions;
}
#endif

#undef DSA_BUILD_HANDLE
#undef DSA_EXTRACT_MARKER

}