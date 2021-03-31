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
///\file double_stack_allocator.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-07
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_MEMORY_DOUBLE_STACK_ALLOCATOR_H
#define ODYSSEUS_ODYSSEUS_MEMORY_DOUBLE_STACK_ALLOCATOR_H

#include <odysseus/memory/mem.h>

namespace odysseus {

#define DSA_EXTRACT_MARKER(HANDLE) \
  ((HANDLE & 0xffffff) - 1)

#define DSA_BUILD_HANDLE(MARKER, SHIFT) \
  ((MARKER + 1u) | (SHIFT << 24u))

/// RAII Double Stack Allocator
///
/// Manages two stacks stored in a single memory block, the LOWER stack and
/// the UPPER stack. Each stack is stored in one end of the memory block,
/// the LOWER stack is stored in lower addresses while the UPPER stack is
/// stored in the upper end. Both stacks advance towards each other. The
/// following diagram depicts the double stack allocator structure:
///
///         0              L        T      U            N
///         [**** LOWER ***)--------|------(** UPPER ** ]
///
/// In a memory block of N + 1 bytes, the LOWER stack occupies the range
/// [0, L) and the UPPER stack occupies (U, N]. It is possible to define a
/// limiting threshold T to limit the individual capacity of both stacks.
class DoubleStackAllocator {
public:
  /****************************************************************************
                                 CONSTRUCTORS
  ****************************************************************************/
  /// \param capacity_in_bytes total memory capacity
  /// \param context
  explicit DoubleStackAllocator(std::size_t capacity_in_bytes = 0, byte *buffer = nullptr);
  ///
  ~DoubleStackAllocator();
  /****************************************************************************
                                    SIZE
  ****************************************************************************/
  /// \return total stack capacity (in bytes)
  [[nodiscard]] std::size_t capacityInBytes() const;
  /// \return available size that can be allocated in the lower stack
  [[nodiscard]] std::size_t availableLowerSizeInBytes() const;
  /// \return available size that can be allocated in the upper stack
  [[nodiscard]] std::size_t availableUpperSizeInBytes() const;
  /// All previous data is deleted and markers get invalid
  /// \param size_in_bytes total memory capacity
  OdResult resize(std::size_t size_in_bytes);
  /// \param lower_stack_size_in_bytes a value grater than capacity removes the
  /// threshold
  OdResult setThreshold(std::size_t lower_stack_size_in_bytes);
  /****************************************************************************
                                    ALLOCATION
  ****************************************************************************/
  /// Allocates a new block from lower stack top
  /// \param block_size_in_bytes
  /// \return pointer to the new allocated block
  MemHandle allocateLower(u64 block_size_in_bytes, std::size_t align = 1);
  ///
  /// \tparam T
  /// \tparam P
  /// \param params
  /// \return
  template<typename T, class... P>
  MemHandle allocateAlignedLower(P &&... params) {
    auto handle = allocateLower(sizeof(T), alignof(T));
    if (!handle.id)
      return handle;
    T *ptr = reinterpret_cast<T *>(data_ + ((handle.id & 0xffffff) - 1));
    new(ptr) T(std::forward<P>(params)...);
    return handle;
  }
  /// Allocates a new block from upper stack top
  /// \param block_size_in_bytes
  /// \return pointer to the new allocated block
  MemHandle allocateUpper(u64 block_size_in_bytes, std::size_t align = 1);
  ///
  /// \tparam T
  /// \tparam P
  /// \param params
  /// \return
  template<typename T, class... P>
  MemHandle allocateAlignedUpper(P &&... params) {
    auto handle = allocateUpper(sizeof(T), alignof(T));
    if (!handle.id)
      return handle;
    T *ptr = reinterpret_cast<T *>(data_ + ((handle.id & 0xffffff) - 1));
    new(ptr) T(std::forward<P>(params)...);
    return handle;
  }
  /// \param upper_marker
  OdResult freeToUpperMarker(MemHandle handle);
  /// \param lower_marker
  OdResult freeToLowerMarker(MemHandle handle);
  /// Roll stack back to zero
  void clear();
  ///
  /// \tparam T
  /// \param handle
  /// \param value
  /// \return
  template<typename T>
  OdResult set(MemHandle handle, const T &value) {
    std::cerr << " handle " << handle.id << std::endl;
    if (handle.id == 0 || handle.id >= capacity_)
      return OdResult::INVALID_INPUT;
    std::cerr << handle.id << std::endl;
    std::cerr << "====" << ((handle.id & 0xffffff) - 1) << " " <<  value << std::endl;
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

  /****************************************************************************
                                   DEBUG
  ****************************************************************************/
#ifdef ODYSSEUS_DEBUG
  void dump(std::size_t start = 0, std::size_t size = 0) const;
  std::vector<ponos::MemoryDumper::Region> getDataRegions();
  [[nodiscard]]static std::vector<ponos::MemoryDumper::Region> getRegions();
#endif

private:
  byte *data_{};
  std::size_t capacity_{0};
  std::size_t lower_marker_{0};
  std::size_t upper_marker_{0};
  std::size_t threshold_{0};
  bool using_extern_memory_{false};

#ifdef ODYSSEUS_DEBUG
  std::vector<std::size_t> odb_handles;
  std::vector<ponos::MemoryDumper::Region> odb_regions;
#endif
};

#undef DSA_BUILD_HANDLE
#undef DSA_EXTRACT_MARKER

}

#endif //ODYSSEUS_ODYSSEUS_MEMORY_DOUBLE_STACK_ALLOCATOR_H
