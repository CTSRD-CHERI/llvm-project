/*===---- __stddef_wint.h - Definition of wint_t ---------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

/* Always define wint_t when modules are available. */
#if !(defined(_WINT_T) || defined(_WINT_T_DECLARED)) || __has_feature(modules)
#if !__has_feature(modules)
#define _WINT_T
#define _WINT_T_DECLARED
#endif
typedef __WINT_TYPE__ wint_t;
#endif
