/*===---- memcap.h - Header for memory capabilities -----------------------===*\
 *
 * Copyright (c) 2014 David Chisnall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
\*===----------------------------------------------------------------------===*/


typedef unsigned short cheri_perms_t;
typedef unsigned __INT32_TYPE__ cheri_type_t;
#ifdef __cplusplus
#define __cheri_bool bool
#else
#define __cheri_bool _Bool
#endif

#if __has_feature(capabilities)
#define __IF_CAPS(x, y) x
typedef __intcap_t intcap_t;
typedef __uintcap_t uintcap_t;
#define capability __capability
#define output __output
#define input __input
#else
#define __IF_CAPS(x, y) y
typedef __INTPTR_TYPE__ intcap_t;
typedef __INTPTR_TYPE__ uintcap_t;
#define capability
#define output 
#define input
#endif

#define __CHERI_GET(__name, __type, __get, __default)            \
  static inline __type                                                        \
  cheri_##__name##__get(capability void *__cap)                              \
  {                                                                           \
    return __IF_CAPS(__builtin_cheri_##__name##__get(__cap), __default);     \
  }

#define __CHERI_SET(__name, __type, __set)                                   \
  static inline capability void*                                              \
  cheri_##__name##__set(const capability void *__cap, __type __val)          \
  {                                                                           \
    return __IF_CAPS(__builtin_cheri_##__name##__set(__cap, __val),          \
                     (void*)__cap);                                           \
  }                                                                           \

#define __CHERI_ACCESSOR(__name, __type, __set, __get, __default)            \
  __CHERI_GET(__name, __type, __get, __default)                              \
  __CHERI_SET(__name, __type, __set)                                         \

__CHERI_GET(length, __SIZE_TYPE__, _get, __SIZE_MAX__)
__CHERI_GET(base, __SIZE_TYPE__, _get, __SIZE_MAX__)
__CHERI_ACCESSOR(offset, __SIZE_TYPE__, _set, _get, __SIZE_MAX__)
__CHERI_GET(type, cheri_type_t, _get, 0)
__CHERI_ACCESSOR(perms, cheri_perms_t, _and, _get, 0)
__CHERI_GET(tag, _Bool, _get, 0)
__CHERI_GET(sealed, _Bool, _get, 0)

static inline
capability void *cheri_offset_increment(const capability void *__cap,
                                         __PTRDIFF_TYPE__ __offset) {
  return __IF_CAPS(__builtin_cheri_offset_increment(__cap, __offset),
                   ((char*)__cap) + __offset);
}

static inline
capability void *cheri_tag_clear(const capability void *__cap) {
  return __IF_CAPS(__builtin_cheri_tag_clear(__cap), (void*)__cap);
}

static inline
capability void *cheri_seal(const capability void *__cap,
                             const capability void *__type) {
  return __IF_CAPS(__builtin_cheri_seal(__cap, __type), (void*)__cap);
}

static inline
capability void *cheri_unseal(const capability void *__cap,
                               const capability void *__type) {
  return __IF_CAPS(__builtin_cheri_unseal(__cap, __type), (void*)__cap);
}

static inline
void cheri_perms_check(const capability void *__cap, cheri_perms_t __perms) {
  __IF_CAPS(__builtin_cheri_perms_check(__cap, __perms), );
}

static inline
void cheri_type_check(const capability void *__cap,
                       const capability void *__type) {
  __IF_CAPS(__builtin_cheri_type_check(__cap, __type), );
}

static inline
capability void *cheri_global_data_get(void) {
  return __IF_CAPS(__builtin_cheri_global_data_get(), 0);
}

static inline
capability void *cheri_program_counter_get(void) {
  return __IF_CAPS(__builtin_cheri_program_counter_get(), 0);
}

#undef __CHERI_ACCESSOR

#undef __CHERI_ACCESSOR
#undef __CHERI_GET
#undef __CHERI_SET
#undef __cheri_bool
#undef __IF_CAPS
