/*===---- __stddef_size_t.h - Definition of size_t -------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#if !(defined(_SIZE_T) || defined(_SIZE_T_DECLARED))
#define _SIZE_T
#define _SIZE_T_DECLARED /* FreeBSD */

typedef __SIZE_TYPE__ size_t;

#endif
