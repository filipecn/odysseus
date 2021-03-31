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
///\file stack_allocator.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-06
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_MEMORY_STACK_ALLOCATOR_H
#define ODYSSEUS_ODYSSEUS_MEMORY_STACK_ALLOCATOR_H

#include <odysseus/memory/mem.h>
#include <ponos/common/defs.h>

namespace odysseus {

/// RAII Stack Allocator
///
/// \note Handle Construction:
/// \note The most significant byte of the handle is used to store the alignment
/// shift and the rest is used to store the address offset + 1 of the first
/// byte of the allocated block. Suppose the alignment requires a shift of
/// 3 bytes and the allocated block would start at byte with offset 10.
/// A 32 bit handle id in this case will have the value of 0x3000000A.
class StackAllocator {
public:
  /****************************************************************************
                                 CONSTRUCTORS
  ****************************************************************************/
  /// \param size_in_bytes
  explicit StackAllocator(std::size_t size_in_bytes = 0);
  /// \param size_in_bytes total memory capacity
  /// \param buffer external allocated memory
  explicit StackAllocator(std::size_t size_in_bytes, byte *buffer);
  ///
  ~StackAllocator();
  /****************************************************************************
                                   SIZE
  ****************************************************************************/
  /// \return total stack capacity (in bytes)
  [[nodiscard]] std::size_t capacityInBytes() const;
  /// \return available size that can be allocated
  [[nodiscard]] std::size_t availableSizeInBytes() const;
  /// All previous data is deleted and markers get invalid
  /// \param size_in_bytes total memory capacity
  OdResult resize(std::size_t size_in_bytes);
  /****************************************************************************
                                    ALLOCATION
  ****************************************************************************/
  /// Allocates a new block from stack top
  /// \param block_size_in_bytes
  /// \return pointer to the new allocated block
  MemHandle allocate(std::size_t block_size_in_bytes, std::size_t align = 1);
  ///
  /// \tparam T
  /// \tparam P
  /// \param params
  /// \return
  template<typename T, class... P>
  MemHandle allocateAligned(P &&... params) {
    auto handle = allocate(sizeof(T), alignof(T));
    if (!handle.id)
      return handle;
    T *ptr = reinterpret_cast<T *>(data_ + ((handle.id & 0xffffff) - 1));
    new(ptr) T(std::forward<P>(params)...);
    return handle;
  }
  ///
  /// \tparam T
  /// \param handle
  /// \param value
  /// \return
  template<typename T>
  OdResult set(MemHandle handle, const T &value) {
    if (handle.id == 0 || handle.id >= capacity_)
      return OdResult::INVALID_INPUT;
    *reinterpret_cast<T *>(data_ + ((handle.id & 0xffffff) - 1)) = value;
    return OdResult::SUCCESS;
  }
  ///
  /// \tparam T
  /// \param handle
  /// \param value
  /// \return
  template<typename T>
  OdResult set(MemHandle handle, T &&value) {
    if (handle.id == 0 || handle.id >= capacity_)
      return OdResult::INVALID_INPUT;
    *reinterpret_cast<T *>(data_ + ((handle.id & 0xffffff) - 1)) = std::forward<T>(value);
    return OdResult::SUCCESS;
  }
  ///
  /// \tparam T
  /// \param handle
  /// \return
  template<typename T>
  T *get(MemHandle handle) {
    ASSERT(handle.id > 0 && ((handle.id & 0xffffff) - 1) < capacity_)
    return reinterpret_cast<T *>(data_ + ((handle.id & 0xffffff) - 1));
  }

  /// Roll the stack back to a previous marker point
  /// \param marker
  OdResult freeTo(MemHandle handle);
  /// Roll stack back to zero
  void clear();

  /****************************************************************************
                                   DEBUG
  ****************************************************************************/
#ifdef ODYSSEUS_DEBUG
  void dump(std::size_t start = 0, std::size_t size = 0) const;
  std::vector<ponos::MemoryDumper::Region> getDataRegions();
  [[nodiscard]]static std::vector<ponos::MemoryDumper::Region> getRegions();
#endif

private:
  byte *data_{nullptr};
  std::size_t capacity_{0};
  std::size_t marker_{0};
  bool using_extern_memory_{false};

#ifdef ODYSSEUS_DEBUG
  std::vector<std::size_t> db_handles;
  std::vector<ponos::MemoryDumper::Region> db_regions;
#endif
};

}

#endif //ODYSSEUS_ODYSSEUS_MEMORY_STACK_ALLOCATOR_H
