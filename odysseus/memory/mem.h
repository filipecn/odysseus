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
#include <odysseus/debug/assert.h>
#include <cstdint>

namespace odysseus {

/// Memory Manager Singleton
class mem {
public:
  /****************************************************************************
                               STATIC METHODS
  ****************************************************************************/

  /// Shift **address** upwards if necessary to ensure it is aligned to
  /// **align** number of bytes.
  /// \param address **[in]** memory address
  /// \param align **[in]** number of bytes
  /// \return aligned address
  static inline uintptr_t alignAddress(uintptr_t address, size_t align) {
    const size_t mask = align - 1;
    ASSERT((align & mask) == 0);
    return (address + mask) & ~mask;
  }
  /// Shift pointer **ptr** upwards if necessary to ensure it is aligned to
  /// **align** number of bytes.
  /// \tparam T data type
  /// \param ptr **[in]** pointer
  /// \param align **[in]** number of bytes
  /// \return aligned poitner
  template<typename T>
  static inline T *alignPointer(T *ptr, size_t align) {
    const auto addr = reinterpret_cast<uintptr_t>(ptr);
    const uintptr_t addr_aligned = alignAddress(addr, align);
    return reinterpret_cast<T *>(addr_aligned);
  }
  /// Allocate **size** bytes of memory aligned by **align** bytes.
  /// \param bytes **[in]** memory size in bytes
  /// \param align **[in]** number of bytes of alignment
  /// \return pointer to allocated memory
  static void *allocAligned(size_t size, size_t align);
  /// Free memory allocated by allocAligned function
  /// \param p_mem pointer to aligned memory block
  static void freeAligned(void *p_mem);
  ///
  /// \return
  static mem &get() {
    static mem singleton;
    return singleton;
  }

  mem &operator=(const mem &) = delete;

private:
  mem() = default;
  ~mem() = default;

};

}

#endif //ODYSSEUS_ODYSSEUS_MEMORY_MEM_H
