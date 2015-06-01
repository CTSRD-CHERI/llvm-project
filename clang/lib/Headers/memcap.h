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


typedef unsigned short memcap_perms_t;
typedef unsigned __INT32_TYPE__ memcap_type_t;
#ifdef __cplusplus
#define __memcap_bool bool
#else
#define __memcap_bool _Bool
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

#define __MEMCAP_GET(__name, __type, __get, __default)            \
  static inline __type                                                        \
  memcap_##__name##__get(capability void *__cap)                              \
  {                                                                           \
    return __IF_CAPS(__builtin_memcap_##__name##__get(__cap), __default);     \
  }

#define __MEMCAP_SET(__name, __type, __set)                                   \
  static inline capability void*                                              \
  memcap_##__name##__set(const capability void *__cap, __type __val)          \
  {                                                                           \
    return __IF_CAPS(__builtin_memcap_##__name##__set(__cap, __val),          \
                     (void*)__cap);                                           \
  }                                                                           \

#define __MEMCAP_ACCESSOR(__name, __type, __set, __get, __default)            \
  __MEMCAP_GET(__name, __type, __get, __default)                              \
  __MEMCAP_SET(__name, __type, __set)                                         \

__MEMCAP_ACCESSOR(length, __SIZE_TYPE__, _set, _get, __SIZE_MAX__)
__MEMCAP_ACCESSOR(base, __SIZE_TYPE__, _increment, _get, __SIZE_MAX__)
__MEMCAP_ACCESSOR(offset, __SIZE_TYPE__, _set, _get, __SIZE_MAX__)
__MEMCAP_GET(type, memcap_type_t, _get, 0)
__MEMCAP_ACCESSOR(perms, memcap_perms_t, _and, _get, 0)
__MEMCAP_GET(tag, _Bool, _get, 0)
__MEMCAP_GET(sealed, _Bool, _get, 0)

static inline
capability void *memcap_offset_increment(const capability void *__cap,
                                         __PTRDIFF_TYPE__ __offset) {
  return __IF_CAPS(__builtin_memcap_offset_increment(__cap, __offset),
                   ((char*)__cap) + __offset);
}
static inline
capability void *memcap_base_only_increment(const capability void *__cap,
                                         __PTRDIFF_TYPE__ __offset) {
  return __IF_CAPS(__builtin_memcap_base_only_increment(__cap, __offset),
                   (void*)__cap);
}

static inline
capability void *memcap_tag_clear(const capability void *__cap) {
  return __IF_CAPS(__builtin_memcap_tag_clear(__cap), (void*)__cap);
}

static inline
capability void *memcap_seal(const capability void *__cap,
                             const capability void *__type) {
  return __IF_CAPS(__builtin_memcap_seal(__cap, __type), (void*)__cap);
}

static inline
capability void *memcap_unseal(const capability void *__cap,
                               const capability void *__type) {
  return __IF_CAPS(__builtin_memcap_unseal(__cap, __type), (void*)__cap);
}

static inline
void memcap_perms_check(const capability void *__cap, memcap_perms_t __perms) {
  __IF_CAPS(__builtin_memcap_perms_check(__cap, __perms), );
}

static inline
void memcap_type_check(const capability void *__cap,
                       const capability void *__type) {
  __IF_CAPS(__builtin_memcap_type_check(__cap, __type), );
}

static inline
capability void *memcap_global_data_get(void) {
  return __IF_CAPS(__builtin_memcap_global_data_get(), 0);
}

static inline
capability void *memcap_program_counter_get(void) {
  return __IF_CAPS(__builtin_memcap_program_counter_get(), 0);
}

#undef __MEMCAP_ACCESSOR

#undef __MEMCAP_ACCESSOR
#undef __MEMCAP_GET
#undef __MEMCAP_SET
#undef __memcap_bool
#undef __IF_CAPS
