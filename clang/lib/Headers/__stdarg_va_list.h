/*===---- __stdarg_va_list.h - Definition of va_list -----------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#if !defined(_VA_LIST) && !defined(_VA_LIST_DECLARED)
#define _VA_LIST
#define _VA_LIST_DECLARED /* FreeBSD */
typedef __builtin_va_list va_list;
#endif
