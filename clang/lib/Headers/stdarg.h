/*===---- stdarg.h - Variable argument handling ----------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __STDARG_H

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#endif

#ifdef __need___va_list
#undef __need___va_list
#else
#define __STDARG_H
#if !defined(_VA_LIST) && !defined(_VA_LIST_DECLARED)
typedef __builtin_va_list va_list;
#define _VA_LIST
#define _VA_LIST_DECLARED /* FreeBSD */
#endif

/* FIXME: This is using the placeholder dates Clang produces for these macros
   in C2x mode; switch to the correct values once they've been published. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L
/* C2x does not require the second parameter for va_start. */
#define va_start(ap, ...) __builtin_va_start(ap, 0)
#else
#ifndef va_start
/* Versions before C2x do require the second parameter. */
#define va_start(ap, param) __builtin_va_start(ap, param)
#endif
#endif
#ifndef va_end
#define va_end(ap)          __builtin_va_end(ap)
#endif
#ifndef va_arg
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#endif

/* GCC always defines __va_copy, but does not define va_copy unless in c99 mode
 * or -ansi is not specified, since it was not part of C90.
 */
#define __va_copy(d,s) __builtin_va_copy(d,s)

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||              \
    (defined(__cplusplus) && __cplusplus >= 201103L) ||                        \
    !defined(__STRICT_ANSI__)
#ifndef va_copy
#define va_copy(dest, src)  __builtin_va_copy(dest, src)
#endif
#endif

#endif /* __STDARG_H */

#endif /* not __STDARG_H */
