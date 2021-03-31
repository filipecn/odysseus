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
///\file mem.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-06
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_MEMORY_MEM_H
#define ODYSSEUS_ODYSSEUS_MEMORY_MEM_H

#include <ponos/common/defs.h>
#include <odysseus/debug/debug.h>
#include <odysseus/debug/result.h>
#include <cstdint>
#include <vector>
#ifdef ODYSSEUS_DEBUG
#include <ponos/log/memory_dump.h>
#endif

namespace odysseus {

#ifdef ODYSSEUS_DEBUG
class StackAllocator;
class DoubleStackAllocator;
#endif

/// Object returned by all memory allocators
/// Each allocator puts a meaning into its value
struct MemHandle {
  /// handle identifier, a value of zero identifies an invalid memory handle
  const std::size_t id{0};
  [[nodiscard]] inline bool isValid() const { return id != 0; }
};

/// Memory Manager Singleton
/// This class is responsible for managing all memory used in the system by
/// allocating all memory first and controlling how the allocated memory is
/// used.
class mem {
public:
  ///
  enum class ContextType {
    HEAP,
    SINGLE_FRAME [[maybe_unused]],
  };
  /****************************************************************************
                               STATIC PUBLIC FIELDS
  ****************************************************************************/
  [[maybe_unused]] static u32 cache_l1_size;
  /****************************************************************************
                               INLINE STATIC METHODS
  ****************************************************************************/
  /// \param number_of_bytes
  /// \param align alignment size in number of bytes
  /// \return the actual amount of bytes necessary to store number_of_bytes
  /// under the alignment
  static inline std::size_t alignTo(std::size_t number_of_bytes, std::size_t align) {
    return number_of_bytes > 0 ? (1u + (number_of_bytes - 1u) / align) * align : 0;
  }
  /// \param address
  /// \param align
  /// \return
  static inline std::size_t leftAlignShift(uintptr_t address, std::size_t align) {
    const std::size_t mask = align - 1;
    ASSERT((align & mask) == 0);
    return address - (address & ~mask);
  }
  /// \param address
  /// \param align
  /// \return
  static inline std::size_t rightAlignShift(uintptr_t address, std::size_t align) {
    const std::size_t mask = align - 1;
    ASSERT((align & mask) == 0);
    return ((address + mask) & ~mask) - address;
  }
  /// Shifts **address** upwards if necessary to ensure it is aligned to
  /// **align** number of bytes.
  /// \param address **[in]** memory address
  /// \param align **[in]** number of bytes
  /// \return aligned address
  static inline uintptr_t alignAddress(uintptr_t address, std::size_t align) {
    const std::size_t mask = align - 1;
    ASSERT((align & mask) == 0);
    return (address + mask) & ~mask;
  }
  /// Shifts pointer **ptr** upwards if necessary to ensure it is aligned to
  /// **align** number of bytes.
  /// \tparam T data type
  /// \param ptr **[in]** pointer
  /// \param align **[in]** number of bytes
  /// \return aligned pointer
  template<typename T>
  static inline T *alignPointer(T *ptr, std::size_t align) {
    const auto addr = reinterpret_cast<uintptr_t>(ptr);
    const uintptr_t addr_aligned = alignAddress(addr, align);
    return reinterpret_cast<T *>(addr_aligned);
  }
  /// Allocates **size** bytes of memory aligned by **align** bytes.
  /// \param size **[in]** memory size in bytes
  /// \param align **[in]** number of bytes of alignment
  /// \return pointer to allocated memory
  static void *allocAligned(std::size_t size, std::size_t align);
  /// Frees memory allocated by allocAligned function
  /// \param p_mem pointer to aligned memory block
  static void freeAligned(void *p_mem);
  ///
  /// \return
  static mem &get() {
    static mem singleton;
    return singleton;
  }

  mem &operator=(const mem &) = delete;

  /****************************************************************************
                                METHODS
  ****************************************************************************/
  /// Allocates the memory that will be available for all allocators to use
  /// \param size_in_bytes
  /// \return
  static OdResult init(std::size_t size_in_bytes);

  static std::size_t availableSize();
  ///
  /// \tparam AllocatorType
  /// \param size_in_bytes
  /// \param context
  /// \return
  template<typename AllocatorType>
  static OdResult pushContext(std::size_t size_in_bytes) {
    auto &instance = get();
    // check if mem was initialized first
    if (!instance.buffer_ || !instance.size_)
      return OdResult::BAD_ALLOCATION;
    // check if there is room for the requested context size
    if (availableSize() < size_in_bytes + sizeof(AllocatorType))
      return OdResult::OUT_OF_BOUNDS;
    instance.contexts_.push_back({size_in_bytes + sizeof(AllocatorType), instance.next_});
    new(instance.next_) AllocatorType(size_in_bytes,
                                      instance.next_ + sizeof(AllocatorType));
#ifdef ODYSSEUS_DEBUG
    instance.odb_regions.push_back({
                                       reinterpret_cast<uintptr_t>(instance.next_)
                                           - reinterpret_cast<uintptr_t>(instance.buffer_),
                                       sizeof(AllocatorType),
                                       1,
                                       ponos::ConsoleColors::color(instance.odb_regions.size() + 1),
                                       AllocatorType::getRegions()
                                   });
    instance.odb_regions.push_back({
                                       reinterpret_cast<uintptr_t>(instance.next_)
                                           - reinterpret_cast<uintptr_t>(instance.buffer_)
                                           + sizeof(AllocatorType),
                                       size_in_bytes,
                                       1,
                                       ponos::ConsoleColors::color(instance.odb_regions.size() + 1),
                                       {}
                                   });
    // register allocator
    if (std::is_same_v<AllocatorType, StackAllocator>)
      instance.odb_context_allocators.push_back(
          {instance.odb_regions.size() - 1,
           ContextAllocatorType::STACK_ALLOCATOR,
           instance.next_});
#endif
    instance.next_ += sizeof(AllocatorType) + size_in_bytes;
    return OdResult::SUCCESS;
  }

  template<class AllocatorType>
  static AllocatorType &getContext(u32 context_index) {
    auto &instance = get();
    return *reinterpret_cast<AllocatorType *>(instance.contexts_[context_index].ptr);
  }

/****************************************************************************
                              DEBUG
****************************************************************************/
#ifdef ODYSSEUS_DEBUG
  static std::string dump(std::size_t start = 0, std::size_t size = 0);
  std::vector<ponos::MemoryDumper::Region> odb_regions;
  enum class ContextAllocatorType {
    STACK_ALLOCATOR,
    DOUBLE_STACK_ALLOCATOR,
    CUSTOM
  };
  struct ContextAllocatorInfo {
    std::size_t region_index;
    ContextAllocatorType type;
    byte *ptr;
  };
  std::vector<ContextAllocatorInfo> odb_context_allocators;
#endif

private:
  mem() = default;
  ~mem();

  struct ContextInfo {
    std::size_t size;
    byte *ptr;
  };

  std::vector<ContextInfo> contexts_;
  std::size_t size_{0};
  byte *buffer_{nullptr};
  byte *next_{nullptr};

};

}

#endif //ODYSSEUS_ODYSSEUS_MEMORY_MEM_H
