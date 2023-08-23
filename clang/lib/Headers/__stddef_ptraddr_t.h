/*===---- __stddef_ptraddr_t.h - Definition of ptraddr_t -------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#if !(defined(_PTRADDR_T) || defined(_PTRADDR_T_DECLARED)) ||                  \
    __has_feature(modules)
/* Always define ptraddr_t when modules are available. */
#if !__has_feature(modules)
#define _PTRADDR_T
#define _PTRADDR_T_DECLARED /* FreeBSD */
#endif
typedef __PTRADDR_TYPE__ ptraddr_t;
#endif
