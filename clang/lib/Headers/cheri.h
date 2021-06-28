/*===---- cheri.h - Header for CHERI capabilities -----------------------===*\
 *
 * Copyright (c) 2014 David Chisnall
 * Copyright (c) 2018 Alex Richardson
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

#pragma once

typedef unsigned short cheri_perms_t;
typedef unsigned short cheri_flags_t;
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
#ifdef WANT_CHERI_QUALIFIER_MACROS
#define capability __capability
#define output __cheri_output
#define input __cheri_input
#endif
#else
#define __IF_CAPS(x, y) y
typedef __INTPTR_TYPE__ intcap_t;
typedef __UINTPTR_TYPE__ uintcap_t;
#define __capability
#ifdef WANT_CHERI_QUALIFIER_MACROS
#define capability
#define output
#define input
#endif
#endif

#define __CHERI_GET(__name, __type, __get, __default)            \
  static inline __type                                                        \
  cheri_##__name##__get(void * __capability __cap)                              \
  {                                                                           \
    return __IF_CAPS(__builtin_cheri_##__name##__get(__cap), __default);     \
  }

#define __CHERI_SET(__name, __type, __set)                                     \
  static inline void *__capability cheri_##__name##__set(                      \
      void *__capability __cap, __type __val) {                                \
    return __IF_CAPS(__builtin_cheri_##__name##__set(__cap, __val),            \
                     (void *)__cap);                                           \
  }

#define __CHERI_ACCESSOR(__name, __type, __set, __get, __default)            \
  __CHERI_GET(__name, __type, __get, __default)                              \
  __CHERI_SET(__name, __type, __set)                                         \

__CHERI_GET(length, __SIZE_TYPE__, _get, __SIZE_MAX__)
__CHERI_GET(base, __SIZE_TYPE__, _get, __SIZE_MAX__)
__CHERI_ACCESSOR(offset, __SIZE_TYPE__, _set, _get, __SIZE_MAX__)
__CHERI_GET(type, cheri_type_t, _get, 0)
__CHERI_ACCESSOR(perms, cheri_perms_t, _and, _get, 0)
__CHERI_ACCESSOR(flags, cheri_flags_t, _set, _get, 0)
__CHERI_GET(tag, __cheri_bool, _get, 0)
__CHERI_GET(sealed, __cheri_bool, _get, 0)

static inline
void * __capability cheri_offset_increment(void *__capability __cap,
                                         __PTRDIFF_TYPE__ __offset) {
  return __IF_CAPS(__builtin_cheri_offset_increment(__cap, __offset),
                   ((char*)__cap) + __offset);
}

static inline
void * __capability cheri_tag_clear(void * __capability __cap) {
  return __IF_CAPS(__builtin_cheri_tag_clear(__cap), (void*)__cap);
}

static inline
void * __capability cheri_seal(void * __capability __cap,
                               const void * __capability __type) {
  return __IF_CAPS(__builtin_cheri_seal(__cap, __type), (void*)__cap);
}

static inline
void * __capability cheri_unseal(void * __capability __cap,
                                 const void * __capability __type) {
  return __IF_CAPS(__builtin_cheri_unseal(__cap, __type), (void*)__cap);
}

#ifndef __CHERI_PURE_CAPABILITY__
static inline
void * __capability
cheri_cap_from_pointer(const void * __capability __cap, void *__ptr) {
  return __IF_CAPS(__builtin_cheri_cap_from_pointer(__cap, __ptr),
                   (void *)__ptr);
}

static inline
void * cheri_cap_to_pointer(const void * __capability __cap,
                            void * __capability __offset) {
  return __IF_CAPS(__builtin_cheri_cap_to_pointer(__cap, __offset),
                   (void *)__offset);
}
#endif

static inline
void cheri_perms_check(const void * __capability __cap, cheri_perms_t __perms) {
  __IF_CAPS(__builtin_cheri_perms_check(__cap, __perms), );
}

static inline
void cheri_type_check(const void * __capability __cap,
                      const void * __capability __type) {
  __IF_CAPS(__builtin_cheri_type_check(__cap, __type), );
}

static inline
void * __capability cheri_global_data_get(void) {
  return __IF_CAPS(__builtin_cheri_global_data_get(), 0);
}

static inline
void * __capability cheri_program_counter_get(void) {
  return __IF_CAPS(__builtin_cheri_program_counter_get(), 0);
}

/* TODO: Should these be builtins to get better diagnostics? */

static inline __attribute__((always_inline)) __attribute__((warn_unused_result))
__PTRADDR_TYPE__
__cheri_low_bits_get(uintcap_t ptr, __PTRADDR_TYPE__ mask) {
  /*
   * We need to return a NULL-derived capability here, so we need to explicitly
   * cast the LHS to a non-capability integer.
   */
  return (__PTRADDR_TYPE__)ptr & mask;
}

static inline __attribute__((always_inline)) __attribute__((warn_unused_result))
uintcap_t
__cheri_low_bits_or(uintcap_t ptr, __PTRADDR_TYPE__ bits) {
  /*
   * We want to return a LHS-derived capability here so using the default
   * uintcap_t semantics is fine.
   */
  return ptr | bits;
}

static inline __attribute__((always_inline)) __attribute__((warn_unused_result))
uintcap_t
__cheri_low_bits_clear(uintcap_t ptr, __PTRADDR_TYPE__ bits_mask) {
  /*
   * We want to return a LHS-derived capability here so using the default
   * uintcap_t semantics is fine.
   */
  return ptr & (~bits_mask);
}

#ifndef __cheri_usable_low_bits_mask
#define __cheri_usable_low_bits_mask 31
#endif
#define __static_assert_sensible_low_bits(bits)                                \
  __extension__({                                                              \
    _Static_assert(bits < (__cheri_usable_low_bits_mask + 1),                  \
                   "Using too many low pointer bits");                         \
    _Static_assert((bits & (bits + 1)) == 0, "Mask must be all ones");         \
    (__PTRADDR_TYPE__)(bits);                                                  \
  })
#define __runtime_assert_sensible_low_bits(bits, mask)                         \
  __extension__({                                                              \
    __PTRADDR_TYPE__ _bits = (__PTRADDR_TYPE__)(bits);                         \
    __PTRADDR_TYPE__ _mask = __static_assert_sensible_low_bits(mask);          \
    assert(((_bits & _mask) == _bits) && "Bits outside mask used!");           \
    _bits;                                                                     \
  })

/*
 * Get the low bits defined in @p mask from the capability/pointer @p ptr.
 * @p mask must be a compile-time constant less than 31.
 * TODO: should we allow non-constant masks?
 *
 * @param ptr the uintptr_t that may have low bits sets
 * @param mask the mask for the low pointer bits to retrieve
 * @return a size_t containing the the low bits from @p ptr
 *
 * Rationale: this function is needed because extracting the low bits using a
 * bitwise-and operation returns a LHS-derived capability with the offset
 * field set to LHS.offset & mask. This is almost certainly not what the user
 * wanted since it will always compare not equal to any integer constant.
 * For example lots of mutex code uses something like `if ((x & 1) == 1)` to
 * detect if the lock is currently contented. This comparison always returns
 * false under CHERI the LHS of the == is a valid capability with offset 3 and
 * the RHS is an untagged intcap_t with offset 3.
 * See https://github.com/CTSRD-CHERI/clang/issues/189
 */
#define cheri_low_bits_get(ptr, mask)                                          \
  __cheri_low_bits_get(ptr, __static_assert_sensible_low_bits(mask))

/*
 * Bitwise-OR of low bits in a uintptr_t
 *
 * @param ptr the uintptr_t that may have low bits sets
 * @param bits the value to bitwise-or with @p ptr.
 * @return a uintptr_t that has the low bits  @p bits
 *
 * @note this function is not strictly required since a plain bitwise or will
 * generally give the behaviour that is expected from other platforms.
 * However, we can't really make the warning "-Wcheri-bitwise-operations"
 * trigger based on of the right hand side expression since it may not be a
 * compile-time constant.
 */
#define cheri_low_bits_or(ptr, bits)                                           \
  __cheri_low_bits_or(ptr, __runtime_assert_sensible_low_bits(                 \
                               bits, __cheri_usable_low_bits_mask))

/*
 * Set low bits in a uintptr_t
 *
 * @param ptr the uintptr_t that may have low bits sets
 * @param mask the mask for the low pointer bits to be cleared before setting
 * them to @p bits.
 * @param bits the value to bitwise-or with @p ptr.
 * @return a uintptr_t that has the low bits defined in @p mask set to @p bits
 *
 * @note this function is not strictly required since a plain bitwise or will
 * generally give the behaviour that is expected from other platforms.
 * However, we can't really make the warning "-Wcheri-bitwise-operations"
 * trigger based on of the right hand side expression since it may not be a
 * compile-time constant.
 */
#define cheri_low_bits_set(ptr, mask, bits)                                    \
  __cheri_low_bits_or(cheri_low_bits_clear(ptr, mask),                         \
                      __runtime_assert_sensible_low_bits(bits, mask))

/*
 * Clear the bits in @p mask from the capability/pointer @p ptr. Mask must be
 * a compile-time constant less than 31
 *
 * TODO: should we allow non-constant masks?
 *
 * @param ptr the uintptr_t that may have low bits sets
 * @param mask this is the mask for the low pointer bits, not the mask for
 * the bits that should remain set.
 * @return a uintptr_t that has the low bits defined in @p mask set to zeroes
 *
 *
 */
#define cheri_low_bits_clear(ptr, mask)                                        \
  __cheri_low_bits_clear(ptr, __static_assert_sensible_low_bits(mask))

#undef __CHERI_ACCESSOR
#undef __CHERI_GET
#undef __CHERI_SET
#undef __cheri_bool
#undef __IF_CAPS
