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
///\file assert.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-05
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_DEBUG_DEBUG_H
#define ODYSSEUS_ODYSSEUS_DEBUG_DEBUG_H

/****************************************************************************
                          COMPILATION WARNINGS
****************************************************************************/
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif
/****************************************************************************
                             DEBUG MODE
****************************************************************************/
#ifndef ODYSSEUS_DEBUG
#define ODYSSEUS_DEBUG
#endif

#ifdef ODYSSEUS_DEBUG
#define ODYSSEUS_DEBUG_CODE(CODE_CONTENT) {CODE_CONTENT}
#else
#define ODYSSEUS_DEBUG_CODE(CODE_CONTENT)
#endif
/****************************************************************************
                             CHECKS
****************************************************************************/
#if CHECKS_ENABLED

#define CHECK_EXP(expr) \
  if(expr) {}          \
  else {               \
    reportAssertionFailure(#expr, __FILE__, __LINE__);\
  }
#else

#define CHECK_EXP(expr)

#endif // CHECKS_ENABLED
/****************************************************************************
                             ASSERTION
****************************************************************************/
#if ASSERTIONS_ENABLED

#define debugBreak() asm { int 3 }

#define ASSERT(expr) \
  if(expr) {}          \
  else {               \
    reportAssertionFailure(#expr, __FILE__, __LINE__);\
    debugBreak();\
  }
#else

#define ASSERT(expr)

#endif // ASSERTIONS_ENABLED

#endif //ODYSSEUS_ODYSSEUS_DEBUG_DEBUG_H
