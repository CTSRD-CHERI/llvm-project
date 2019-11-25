/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Lawrence Esswood
 * Copyright (c) 2018 Alex Richardson
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract FA8750-10-C-0237
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef CHERI_COMPRESSED_CAP_H
#define CHERI_COMPRESSED_CAP_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h> /* for MIN() */

#ifndef cc128_debug_assert
#ifdef cheri_debug_assert
#define cc128_debug_assert(cond) cheri_debug_assert(cond)
#else
#define cc128_debug_assert(cond) assert(cond)
#endif
#endif

/* Use __uint128 to represent 65 bit length */
__extension__ typedef unsigned __int128 cc128_length_t;
__extension__ typedef signed __int128 cc128_offset_t;

// QEMU already provides cap_register_t but if used in other programs
// we want to define it here:
#ifndef HAVE_CAP_REGISTER_T
struct cap_register {
    /* offset = cursor - base */
    uint64_t _cr_cursor; /* Capability cursor */
    uint64_t cr_base;   /* Capability base addr */
    cc128_length_t _cr_top; /* Capability top */
    uint32_t cr_perms;  /* Permissions */
    uint32_t cr_uperms; /* User Permissions */
    uint64_t cr_pesbt_xored_for_mem;  /* Perms, E, Sealed, Bot, & Top bits (128-bit) */
    uint32_t cr_otype;  /* Object Type, 24 bits */
    uint8_t cr_tag;     /* Tag */
    uint8_t _sbit_for_memory; /* sealed flag */
#ifdef __cplusplus
    inline uint64_t base() const {
       return cr_base;
    }
    inline uint64_t address() const {
       return _cr_cursor;
    }
    inline cc128_offset_t offset() const {
       return (cc128_offset_t)_cr_cursor - (cc128_offset_t)cr_base;
    }
    inline cc128_length_t top() const {
       return _cr_top;
    }
    inline uint64_t top64() const {
        const cc128_length_t t = top();
        return t > UINT64_MAX ? UINT64_MAX : (uint64_t)t;
    }
    inline cc128_length_t length() const {
        return _cr_top - cr_base;
    }
    inline uint64_t length64() const {
        const cc128_length_t l = length();
        return l > UINT64_MAX ? UINT64_MAX : (uint64_t)l;
    }
    inline uint32_t software_permissions() const {
       return cr_uperms;
    }
    inline uint32_t permissions() const {
       return cr_perms;
    }
    inline uint32_t type() const {
       return cr_otype;
    }
#endif
};
typedef struct cap_register cap_register_t;

#endif

#ifdef __cplusplus
/* Some versions of GCC dont't like _Static_assert() in C++ mode */
#define CC128_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
#define CC128_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#define CAP_MAX_ADDRESS_PLUS_ONE ((cc128_length_t)1u << 64)

#define CC128_PERM_GLOBAL          (1 << 0)
#define CC128_PERM_EXECUTE         (1 << 1)
#define CC128_PERM_LOAD            (1 << 2)
#define CC128_PERM_STORE           (1 << 3)
#define CC128_PERM_LOAD_CAP        (1 << 4)
#define CC128_PERM_STORE_CAP       (1 << 5)
#define CC128_PERM_STORE_LOCAL     (1 << 6)
#define CC128_PERM_SEAL            (1 << 7)
#define CC128_PERM_CCALL           (1 << 8)
#define CC128_PERM_UNSEAL          (1 << 9)
#define CC128_PERM_ACCESS_SYS_REGS (1 << 10)
#define CC128_PERM_SETCID          (1 << 11)
#define CC128_PERM_RESERVED4       (1 << 12)
#define CC128_PERM_RESERVED5       (1 << 13)
#define CC128_PERM_RESERVED6       (1 << 14)

#define CC128_PERMS_ALL            (0xfff)     /* [0...11] */
#define CC128_UPERMS_ALL           (0xf)       /* [15...18] */
#define CC128_UPERMS_SHFT          (15)
#define CC128_UPERMS_MEM_SHFT      (12)
#define CC128_MAX_UPERM            (3)
#define CC128_NULL_TOP CAP_MAX_ADDRESS_PLUS_ONE
#define CC128_NULL_LENGTH CAP_MAX_ADDRESS_PLUS_ONE
#define CC128_MAX_LENGTH CAP_MAX_ADDRESS_PLUS_ONE
#define CC128_MAX_TOP CAP_MAX_ADDRESS_PLUS_ONE


/* For CHERI256 all permissions are shifted by one since the sealed bit comes first */
#define CC256_HWPERMS_COUNT           (12)
#define CC256_HWPERMS_RESERVED_COUNT  (3)
#define CC256_UPERMS_COUNT            (16)
#define CC256_PERMS_MEM_SHFT          (1)  /* sealed bit comes first */
#define CC256_UPERMS_MEM_SHFT         (CC256_PERMS_MEM_SHFT + CC256_HWPERMS_COUNT + CC256_HWPERMS_RESERVED_COUNT)
#define CC256_PERMS_ALL_BITS          ((1 << CC256_HWPERMS_COUNT) - 1) /* 12 bits */
#define CC256_PERMS_ALL_BITS_UNTAGGED ((1 << (CC256_HWPERMS_COUNT + CC256_HWPERMS_RESERVED_COUNT)) - 1) /* 15 bits */
#define CC256_UPERMS_ALL_BITS         ((1 << CC256_UPERMS_COUNT) - 1) /* 19 bits */
#define CC256_OTYPE_ALL_BITS          ((1 << 24) - 1)
#define CC256_OTYPE_MEM_SHFT          (32)
#define CC256_OTYPE_BITS              (24)
#define CC256_NULL_LENGTH ((cc128_length_t)UINT64_MAX)
#define CC256_NULL_TOP ((cc128_length_t)UINT64_MAX)

// Offsets based on capBitsToCapability() from cheri_prelude_128.sail
// (Note: we subtract 64 since we only access the high 64 bits when (de)compressing)
#define CC128_FIELD(name, last, start) \
    CC128_FIELD_ ## name ## _START = (start - 64), \
    CC128_FIELD_ ## name ## _LAST = (last - 64), \
    CC128_FIELD_ ## name ## _SIZE = CC128_FIELD_ ## name ## _LAST - CC128_FIELD_ ## name ## _START + 1, \
    CC128_FIELD_ ## name ## _MASK0 = (UINT64_C(1) << CC128_FIELD_ ## name ## _SIZE) - 1, \
    CC128_FIELD_ ## name ## _MASK64 = (uint64_t)CC128_FIELD_ ## name ## _MASK0 << CC128_FIELD_ ## name ## _START, \
    CC128_FIELD_ ## name ## _MAX_VALUE = CC128_FIELD_ ## name ## _MASK0

#define CC128_ENCODE_FIELD(value, name) \
    ((uint64_t)((value) & CC128_FIELD_ ## name ## _MAX_VALUE) << CC128_FIELD_ ## name ## _START)

#define CC128_EXTRACT_FIELD(value, name) \
    cc128_getbits((value), CC128_FIELD_ ## name ## _START, CC128_FIELD_ ## name ## _SIZE)

/* ignore ISO C restricts enumerator values to range of 'int' */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
enum {
    CC128_FIELD(UPERMS, 127, 124),
    CC128_FIELD(HWPERMS, 123, 112),
    CC128_FIELD(RESERVED, 111, 109),
    CC128_FIELD(OTYPE, 108, 91),
    CC128_FIELD(INTERNAL_EXPONENT, 90, 90),
    CC128_FIELD(TOP_ENCODED, 89, 78),
    CC128_FIELD(BOTTOM_ENCODED, 77, 64),

    // Top/bottom offsets depending in INTERNAL_EXPONENT flag:
    // Without internal exponent:
    CC128_FIELD(EXP_ZERO_TOP, 89, 78),
    CC128_FIELD(EXP_ZERO_BOTTOM, 77, 64),
    // With internal exponent:
    CC128_FIELD(EXP_NONZERO_TOP, 89, 81),
    CC128_FIELD(EXPONENT_HIGH_PART, 80, 78),
    CC128_FIELD(EXP_NONZERO_BOTTOM, 77, 67),
    CC128_FIELD(EXPONENT_LOW_PART, 66, 64),
};
#pragma GCC diagnostic pop

#define CC128_OTYPE_BITS CC128_FIELD_OTYPE_SIZE
#define CC128_BOT_WIDTH CC128_FIELD_EXP_ZERO_BOTTOM_SIZE
#define CC128_BOT_INTERNAL_EXP_WIDTH CC128_FIELD_EXP_NONZERO_BOTTOM_SIZE
#define CC128_EXP_LOW_WIDTH CC128_FIELD_EXPONENT_LOW_PART_SIZE
/* These should match the definitions in sail! */
#define CC128_MANTISSA_WIDTH 14
#define CC128_CAP_SIZE 16
#define CC128_CAP_BITS 128
#define CC128_CAP_ADDR_WIDTH 64
#define CC128_CAP_ADDR_MAX UINT64_MAX
#define CC128_CAP_LEN_WIDTH 65
#define CC128_MAX_EXPONENT 52

#define CC_SPECIAL_OTYPE(name, subtract) \
    CAP_ ## name = (CAP_MAX_REPRESENTABLE_OTYPE - subtract ## u), \
    CC128_ ## name = (CC128_MAX_REPRESENTABLE_OTYPE - subtract ## u), \
    CC256_ ## name = (CC256_MAX_REPRESENTABLE_OTYPE - subtract ## u)

enum CC_OTypes {
    CC128_MAX_REPRESENTABLE_OTYPE = ((1u << CC128_OTYPE_BITS) - 1u),
    CC256_MAX_REPRESENTABLE_OTYPE = ((1u << CC256_OTYPE_BITS) - 1u),
#ifdef CHERI_128
    CAP_MAX_REPRESENTABLE_OTYPE = CC128_MAX_REPRESENTABLE_OTYPE,
#else
    CAP_MAX_REPRESENTABLE_OTYPE = CC256_MAX_REPRESENTABLE_OTYPE,
#endif
    CC_SPECIAL_OTYPE(OTYPE_UNSEALED, 0),
    CC_SPECIAL_OTYPE(OTYPE_SENTRY, 1),
    CC_SPECIAL_OTYPE(OTYPE_RESERVED2, 2),
    CC_SPECIAL_OTYPE(OTYPE_RESERVED3, 3),
    CC_SPECIAL_OTYPE(LAST_SPECIAL_OTYPE, 3),
    CC_SPECIAL_OTYPE(MAX_SEALED_OTYPE, 4)

};
// Rerserve the top four otypes for various special capabilities
#define CAP_FIRST_SPECIAL_OTYPE_SIGN_EXTENDED ((uint64_t)-1)
#define CAP_OTYPE_UNSEALED (CAP_MAX_REPRESENTABLE_OTYPE - 0u)
#define CAP_OTYPE_SENTRY (CAP_MAX_REPRESENTABLE_OTYPE - 1u)
#define CAP_OTYPE_RESERVED2 (CAP_MAX_REPRESENTABLE_OTYPE - 2u)
#define CAP_OTYPE_RESERVED3 (CAP_MAX_REPRESENTABLE_OTYPE - 3u)
#define CAP_LAST_SPECIAL_OTYPE CAP_OTYPE_RESERVED3
#define CAP_LAST_SPECIAL_OTYPE_SIGN_EXTENDED ((uint64_t)-3)
#define CAP_MAX_SEALED_OTYPE (CAP_LAST_SPECIAL_OTYPE - 1u)

// Rerserve the top four otypes for various special capabilities
#define CAP_FIRST_SPECIAL_OTYPE_SIGN_EXTENDED ((uint64_t)-1)
#define CAP_OTYPE_UNSEALED (CAP_MAX_REPRESENTABLE_OTYPE - 0u)
#define CAP_OTYPE_SENTRY (CAP_MAX_REPRESENTABLE_OTYPE - 1u)
#define CAP_OTYPE_RESERVED2 (CAP_MAX_REPRESENTABLE_OTYPE - 2u)
#define CAP_OTYPE_RESERVED3 (CAP_MAX_REPRESENTABLE_OTYPE - 3u)
#define CAP_LAST_SPECIAL_OTYPE CAP_OTYPE_RESERVED3
#define CAP_LAST_SPECIAL_OTYPE_SIGN_EXTENDED ((uint64_t)-3)
#define CAP_MAX_SEALED_OTYPE (CAP_LAST_SPECIAL_OTYPE - 1u)


/* Silence ISO C restricts enumerator values to range of 'int' */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
/* From sail:
let null_cap : Capability = struct {
  tag                    = false,
  uperms                 = zeros(),
  permit_set_CID         = false,
  access_system_regs     = false,
  permit_unseal          = false,
  permit_ccall           = false,
  permit_seal            = false,
  permit_store_local_cap = false,
  permit_store_cap       = false,
  permit_load_cap        = false,
  permit_store           = false,
  permit_load            = false,
  permit_execute         = false,
  global                 = false,
  reserved               = zeros(),
  internal_e             = true,
  E                      = resetE,
  sealed                 = false,
  B                      = zeros(),
  T                      = resetT,
  otype                  = ones(),
  address                = zeros()
}
 */
enum {
    CC128_RESET_EXP = 52, // bit 12 in top is set -> shift by 52 to get 1 << 64
    // For a NULL capability we use the internal exponent and need bit 12 in top set
    // to get to 2^65
    // let resetT = 0b01 @ 0x000 /* bit 12 set */
    CC128_RESET_TOP = 1u << (12 - CC128_FIELD_EXPONENT_HIGH_PART_SIZE),
    CC128_NULL_PESBT =
        CC128_ENCODE_FIELD(0, UPERMS) |
        CC128_ENCODE_FIELD(0, HWPERMS) |
        CC128_ENCODE_FIELD(0, RESERVED) |
        CC128_ENCODE_FIELD(1, INTERNAL_EXPONENT) |
        CC128_ENCODE_FIELD(CC128_OTYPE_UNSEALED, OTYPE) |
        CC128_ENCODE_FIELD(CC128_RESET_TOP, EXP_NONZERO_TOP) |
        CC128_ENCODE_FIELD(0, EXP_NONZERO_BOTTOM) |
        CC128_ENCODE_FIELD(CC128_RESET_EXP >> CC128_FIELD_EXPONENT_LOW_PART_SIZE, EXPONENT_HIGH_PART) |
        CC128_ENCODE_FIELD(CC128_RESET_EXP & CC128_FIELD_EXPONENT_LOW_PART_MAX_VALUE, EXPONENT_LOW_PART)
};
// Whatever NULL would encode to is this constant. We mask on store/load so this
// invisibly keeps null 0 whatever we choose it to be.
// #define CC128_NULL_XOR_MASK 0x1ffff8000000
#define CC128_NULL_XOR_MASK UINT64_C(0x00001ffffc018004)
#pragma GCC diagnostic pop


#ifdef __cplusplus
template<size_t a, size_t b>
static constexpr bool check_same() {
    static_assert(a == b, "");
    return true;
}
#define CC128_STATIC_ASSERT_SAME(a, b) static_assert(check_same<a, b>(), "")
#else
#define CC128_STATIC_ASSERT_SAME(a, b) _Static_assert(a == b, "")
#endif
CC128_STATIC_ASSERT_SAME(CC128_NULL_XOR_MASK, CC128_NULL_PESBT);
CC128_STATIC_ASSERT_SAME(CC128_MANTISSA_WIDTH, CC128_BOT_WIDTH);
CC128_STATIC_ASSERT_SAME(CC128_MANTISSA_WIDTH, CC128_FIELD_EXP_ZERO_BOTTOM_SIZE);

/* Avoid pulling in code that uses cr_pesbt_xored_for_mem when building QEMU256 */
#ifndef CC128_DEFINE_FUNCTIONS
#define CC128_DEFINE_FUNCTIONS 1
#endif
#ifndef CC256_DEFINE_FUNCTIONS
#define CC256_DEFINE_FUNCTIONS 1
#endif

#if CC128_DEFINE_FUNCTIONS != 0

/* Returns the index of the most significant bit set in x */
static inline uint32_t cc128_idx_MSNZ(uint64_t x) {
#if defined(__GNUC__) && !defined(__clang__)
#define CAP_HAVE_BUILTIN_CLZ
#elif defined(__has_builtin)
#if __has_builtin(__builtin_clzll)
#define CAP_HAVE_BUILTIN_CLZ
#endif
#endif

#ifndef CAP_HAVE_BUILTIN_CLZ
/* floor(log2(x)) != floor(log2(y)) */
#warning "__builtin_clzll not supported, using slower path"
#define ld_neq(x, y) (((x) ^ (y)) > ((x) & (y)))
    uint32_t r = ld_neq(x, x & 0x5555555555555555ull) + (ld_neq(x, x & 0x3333333333333333ull) << 1) +
                 (ld_neq(x, x & 0x0f0f0f0f0f0f0f0full) << 2) + (ld_neq(x, x & 0x00ff00ff00ff00ffull) << 3) +
                 (ld_neq(x, x & 0x0000ffff0000ffffull) << 4) + (ld_neq(x, x & 0x00000000ffffffffull) << 5);
#undef ld_neq
#else
    cc128_debug_assert(x != 0);
    uint32_t r = 63u - (uint32_t)__builtin_clzll(x);
    // printf("%s: %016llx = %d (__builtin_clzll(x) = %d, 63 - clz = %d)\n", __func__, x, r, __builtin_clzll(x), 63 -
    // __builtin_clzll(x));
#endif
    return r;
}

/*
 * e = idxMSNZ( (rlength + (rlength >> 6)) >> 19 )
 * where (rlength + (rlength >> 6)) needs to be a 65 bit integer
 */
static inline uint32_t _cc128_compute_e(uint64_t rlength, uint32_t bwidth) {
    if (rlength < (1u << (bwidth - 1)))
        return 0;

    return (cc128_idx_MSNZ(rlength) - (bwidth - 2));
}

static inline uint32_t cc128_get_exponent(cc128_length_t length) {
    const uint32_t bwidth = CC128_BOT_WIDTH;
    if (length > UINT64_MAX) {
        return 65 - (bwidth - 1);
    } else {
        return _cc128_compute_e((uint64_t)length, bwidth);
    }
}

static inline uint64_t cc128_getbits(uint64_t src, uint32_t str, uint32_t sz) {
    return ((src >> str) & ((UINT64_C(1) << sz) - UINT64_C(1)));
}

// truncates `value`, keeping only the _least_ significant `n` bits.
static inline uint64_t cc128_truncate64(uint64_t value, size_t n) {
    return value & ((UINT64_C(1) << n) - 1);
}

// truncates `value`, keeping only the _most_ significant `n` bits.
#define TRUNCATE_LSB_FUNC(type_width) \
static inline uint64_t cc128_truncateLSB##type_width(uint64_t value, size_t n) { \
    CC128_STATIC_ASSERT(type_width <= 64, ""); \
    return value >> (type_width - n);\
}

TRUNCATE_LSB_FUNC(14)
TRUNCATE_LSB_FUNC(23)
// TRUNCATE_LSB_FUNC(64)

#define	CC128_CONCAT1(x,y)	x ## y
#define	CC128_CONCAT(x,y)	CC128_CONCAT1(x,y)
// FIXME: only one level of expansion works here?
#define cc128_truncateLSB_generic(type_width) CC128_CONCAT(cc128_truncateLSB, type_width)

static inline void decompress_128cap_already_xored(uint64_t pesbt, uint64_t cursor, cap_register_t* cdp) {
    cdp->cr_perms = (uint32_t)CC128_EXTRACT_FIELD(pesbt, HWPERMS);
    cdp->cr_uperms = (uint32_t)CC128_EXTRACT_FIELD(pesbt, UPERMS);
    uint32_t BWidth = CC128_BOT_WIDTH;
    uint32_t BMask = (1u << BWidth) - 1;
    uint32_t TMask = BMask >> 2;

    bool IE = (bool)(uint32_t)CC128_EXTRACT_FIELD(pesbt, INTERNAL_EXPONENT);

    uint8_t E, L_msb;
    uint32_t B;
    uint32_t T;

    if (IE) {
        E = (uint8_t)(CC128_EXTRACT_FIELD(pesbt, EXPONENT_LOW_PART) |
            (CC128_EXTRACT_FIELD(pesbt, EXPONENT_HIGH_PART) << CC128_FIELD_EXPONENT_LOW_PART_SIZE));
        // Do not offset by 1! We also need to encode E=0 even with IE
        // Also allow nonsense values over 64 - BWidth + 2: this is expected by sail-generated tests
        // E = MIN(64 - BWidth + 2, E);
        B = (uint32_t)CC128_EXTRACT_FIELD(pesbt, EXP_NONZERO_BOTTOM) << CC128_FIELD_EXPONENT_LOW_PART_SIZE;
        T = (uint32_t)CC128_EXTRACT_FIELD(pesbt, EXP_NONZERO_TOP) << CC128_FIELD_EXPONENT_HIGH_PART_SIZE;
        L_msb = 1;
    } else {
        E = 0;
        L_msb = 0;
        B = (uint32_t)CC128_EXTRACT_FIELD(pesbt, EXP_ZERO_BOTTOM);
        T = (uint32_t)CC128_EXTRACT_FIELD(pesbt, EXP_ZERO_TOP);
    }
    cdp->cr_otype = (uint32_t)CC128_EXTRACT_FIELD(pesbt, OTYPE);
    /*
        Reconstruct top two bits of T given T = B + len and:
        1) the top two bits of B
        2) most significant two bits of length derived from format above
        3) carry out of B[20..0] + len[20..0] that is implied if T[20..0] < B[20..0]
    */
    uint8_t L_carry = T < (B & TMask) ? 1 : 0;
    uint64_t BTop2 = cc128_getbits(B, CC128_MANTISSA_WIDTH - 2, 2);
    uint8_t T_infer = (BTop2 + L_carry + L_msb) & 0x3;
    T |= ((uint32_t)T_infer) << (BWidth - 2);
    E = MIN(CC128_MAX_EXPONENT, E);

    /* Extract bits we need to make the top correction and calculate representable limit */
    // let a3 = truncate(a >> (E + 11), 3) in
    // let B3 = truncateLSB(c.B, 3) in
    // let T3 = truncateLSB(c.T, 3) in
#if 0
    unsigned a3 = (unsigned)cc128_truncate64(cursor >> (E + CC128_MANTISSA_WIDTH - 3), 3);
    unsigned B3 = (unsigned)cc128_truncateLSB_generic(CC128_MANTISSA_WIDTH)(B, 3);
    unsigned T3 = (unsigned)cc128_truncateLSB_generic(CC128_MANTISSA_WIDTH)(T, 3);
#else
    unsigned a3 = (unsigned)cc128_getbits(cursor, E + CC128_MANTISSA_WIDTH - 3, 3);
    unsigned B3 = (unsigned)cc128_getbits(B, CC128_MANTISSA_WIDTH - 3, 3);
    unsigned T3 = (unsigned)cc128_getbits(T, CC128_MANTISSA_WIDTH - 3, 3);
#endif
    // let R3 = B3 - 0b001 in /* wraps */
    unsigned R3 = (unsigned)cc128_truncate64(B3 - 1, 3); //B3 == 0 ? 7 : B3 - 1;
    /* Do address, base and top lie in the R aligned region above the one containing R? */
    // let aHi = if a3 <_u R3 then 1 else 0 in
    // let bHi = if B3 <_u R3 then 1 else 0 in
    // let tHi = if T3 <_u R3 then 1 else 0 in
    int aHi = a3 < R3 ? 1 : 0;
    int bHi = B3 < R3 ? 1 : 0;
    int tHi = T3 < R3 ? 1 : 0;

    /* Compute region corrections for top and base relative to a */
    // let correction_base = bHi - aHi in
    // let correction_top  = tHi - aHi in
    int correction_base = bHi - aHi;
    int correction_top = tHi - aHi;
    // let a_top = (a >> (E + mantissa_width)) in {
    // let a_top = (a >> (E + 14)) in
    // Note: shifting by 64 is a no-op and causes wrong results!
    const unsigned a_top_shift = E + CC128_MANTISSA_WIDTH;
    uint64_t a_top = a_top_shift >= CC128_CAP_ADDR_WIDTH ? 0 :  cursor >> a_top_shift;

    // base : CapLenBits = truncate((a_top + correction_base) @ c.B @ zeros(E), cap_len_width);
    cc128_length_t base = (uint64_t)((int64_t)a_top + correction_base);
    base <<= CC128_MANTISSA_WIDTH;
    base |= B;
    base <<= E;
    base &= ((cc128_length_t)1 << CC128_CAP_LEN_WIDTH) - 1;
    cc128_debug_assert((uint64_t)(base >> CC128_CAP_ADDR_WIDTH) <= 1); // max 65 bits
    // top  : truncate((a_top + correction_top)  @ c.T @ zeros(E), cap_len_width);
    cc128_length_t top = (uint64_t)((int64_t)a_top + correction_top);
    top <<= CC128_MANTISSA_WIDTH;
    top |= T;
    top <<= E;
    top &= ((cc128_length_t)1 << CC128_CAP_LEN_WIDTH) - 1;
    cc128_debug_assert((uint64_t)(top >> CC128_CAP_ADDR_WIDTH) <= 1); // max 65 bits

    // if (base[cap_addr_width] == bitone) then {
    bool base_high = (base >> CC128_CAP_ADDR_WIDTH);
    if (base_high) {
          /* If base[64] is set this indicates under or overflow i.e. a has
           * wrapped around the address space and been corrected. In this case
           * we need to correct top[64] because top is not quite modulo 2**64 due
           * to having max top == 2**64 in one particular case:
           */
        // top[cap_addr_width] = if (aHi == 1) & (tHi == 1) then bitone else bitzero;
        if (aHi && tHi) {
            top |= ((cc128_length_t)1 << 64);
        } else {
            top &= UINT64_MAX; // clear top bit
        }
        base = (uint64_t)base; // strip the high bit
    }
    cc128_debug_assert(base <= CC128_CAP_ADDR_MAX);

    /* The following is apparently equivalent to above and used by hw. */
    /*
    let base2 : bits(2) = bitzero @ base[63];
    let top2  : bits(2) = top[64..63];
    if (E < (unsigned(resetE) - 1)) & (unsigned(top2 - base2) > 1) then {
      top[64] = ~(top[64]);
    };
    */
    cc128_debug_assert((uint64_t)(top >> 64) <= 1); // should be at most 1 bit over

    // Note: top<base can happen for invalid capabilities with arbitrary bit patterns
    cdp->_cr_top = top;
    cdp->_cr_cursor = cursor;
    cdp->cr_base = (uint64_t)base;
}

/*
 * Decompress a 128-bit capability.
 */
static inline void decompress_128cap(uint64_t pesbt, uint64_t cursor, cap_register_t* cdp) {
    // TODO: add a bool tagged parameter and return false for invalid capabilities
    //  and also only store pesbt for untagged values
    cdp->cr_pesbt_xored_for_mem = pesbt;
    decompress_128cap_already_xored(pesbt ^ CC128_NULL_XOR_MASK, cursor, cdp);
}

static inline bool cc128_is_cap_sealed(const cap_register_t* cp) { return cp->cr_otype <= CC128_MAX_SEALED_OTYPE; }

/*
 * Compress a capability to 128 bits.
 */
static inline uint64_t compress_128cap_without_xor(const cap_register_t* csp) {
    CC128_STATIC_ASSERT(CC128_BOT_WIDTH == 14, "This code assumes 14-bit bot");
    CC128_STATIC_ASSERT(CC128_BOT_INTERNAL_EXP_WIDTH == 11, "This code assumes 14-bit bot");
    uint64_t base = csp->cr_base;
    const cc128_length_t top = csp->_cr_top;
    const cc128_length_t length = csp->_cr_top - csp->cr_base;

    uint8_t E = (uint8_t)cc128_get_exponent(length);
    const uint64_t length64 = (uint64_t)length;
    // from sail: need IE if length bit 12 is set: let ie = (e != 0) | length[12];
    // 0x1000 (bwidth - 2 bit set) is the first value that cannot be encoded
    // without the internal exponent:
    // Note: 12 = BWidth - 2  / BWidth with internal exponent plus one
    bool IE = (E != 0) || cc128_getbits(length64, CC128_BOT_INTERNAL_EXP_WIDTH + 1, 1);
    uint64_t Be;
    uint64_t Te;
    if (!IE) {
        // precisely representable -> just extract the bits
        cc128_debug_assert(top <= CAP_MAX_ADDRESS_PLUS_ONE); // must be 64 bits
        Be = cc128_truncate64(base, CC128_FIELD_BOTTOM_ENCODED_SIZE);
        Te = cc128_truncate64((uint64_t)top, CC128_FIELD_TOP_ENCODED_SIZE);
    } else {
        uint64_t bot_ie = cc128_truncate64(base >> (E + CC128_EXP_LOW_WIDTH), CC128_BOT_INTERNAL_EXP_WIDTH);
        uint64_t top_ie = cc128_truncate64((uint64_t)(top >> (E + CC128_EXP_LOW_WIDTH)), CC128_BOT_INTERNAL_EXP_WIDTH);
        //    /* Find out whether we have lost significant bits of base and top using a
        //       mask of bits that we will lose (including 3 extra for exp). */
        //    maskLo : bits(65) = zero_extend(replicate_bits(0b1, e + 3));
        //    z65    : bits(65) = zeros();
        //    lostSignificantBase = (base65 & maskLo) != z65;
        //    lostSignificantTop = (top & maskLo) != z65;
        // TODO: stop using cc128_length_t and just handle bit65 set specially?
        const cc128_length_t maskLo = (((cc128_length_t)1u) << (E + CC128_EXP_LOW_WIDTH)) - 1;
        const cc128_length_t zero65 = 0;
        bool lostSignificantBase = (base & maskLo) != zero65;
        bool lostSignificantTop = (top & maskLo) != zero65;
        //    if lostSignificantTop then {
        //      /* we must increment T to make sure it is still above top even with lost bits.
        //         It might wrap around but if that makes B<T then decoding will compensate. */
        //      T_ie = T_ie + 1;
        //    };
        if (lostSignificantTop) {
            top_ie = cc128_truncate64(top_ie + 1, CC128_BOT_INTERNAL_EXP_WIDTH);
        }
        //    /* Has the length overflowed? We chose e so that the top two bits of len would be 0b01,
        //       but either because of incrementing T or losing bits of base it might have grown. */
        //    len_ie = T_ie - B_ie;
        //    if len_ie[10] then {
        //      /* length overflow -- increment E by one and then recalculate
        //         T, B etc accordingly */
        //      incE = true;
        //
        //      lostSignificantBase = lostSignificantBase | B_ie[0];
        //      lostSignificantTop  = lostSignificantTop | T_ie[0];
        //
        //      B_ie = truncate(base >> (e + 4), 11);
        //      let incT : range(0,1) = if lostSignificantTop then 1 else 0;
        //      T_ie = truncate(top >> (e + 4), 11) + incT;
        //    };
        const uint64_t len_ie = cc128_truncate64(top_ie - bot_ie, CC128_BOT_INTERNAL_EXP_WIDTH);
        bool incE = false;
        if (cc128_getbits(len_ie, CC128_BOT_INTERNAL_EXP_WIDTH - 1, 1)) {
            incE = true;
            lostSignificantBase = lostSignificantBase || cc128_getbits(bot_ie, 0, 1);
            lostSignificantTop = lostSignificantTop || cc128_getbits(top_ie, 0, 1);
            bot_ie = cc128_truncate64(base >> (E + CC128_EXP_LOW_WIDTH + 1), CC128_BOT_INTERNAL_EXP_WIDTH);
            const bool incT = lostSignificantTop;
            top_ie = cc128_truncate64((uint64_t)(top >> (E + CC128_EXP_LOW_WIDTH + 1)), CC128_BOT_INTERNAL_EXP_WIDTH);
            if (incT) {
                top_ie = cc128_truncate64(top_ie + 1, CC128_BOT_INTERNAL_EXP_WIDTH);
            }
        }
        //    Bbits = B_ie @ 0b000;
        //    Tbits = T_ie @ 0b000;
        const uint64_t Bbits = bot_ie << CC128_FIELD_EXPONENT_LOW_PART_SIZE;
        const uint64_t Tbits = top_ie << CC128_FIELD_EXPONENT_LOW_PART_SIZE;
        const uint8_t newE = E + (incE ? 1 : 0);
        // Split E between T and B
        const uint64_t expHighBits = cc128_getbits(newE >> CC128_FIELD_EXPONENT_LOW_PART_SIZE , 0, CC128_FIELD_EXPONENT_HIGH_PART_SIZE);
        const uint64_t expLowBits = cc128_getbits(newE, 0, CC128_FIELD_EXPONENT_LOW_PART_SIZE);
        Te = Tbits | expHighBits;
        Be = Bbits | expLowBits;
    }
    uint64_t pesbt =
        CC128_ENCODE_FIELD(csp->cr_uperms, UPERMS) |
        CC128_ENCODE_FIELD(csp->cr_perms, HWPERMS) |
        CC128_ENCODE_FIELD(csp->cr_otype, OTYPE) |
        CC128_ENCODE_FIELD(IE, INTERNAL_EXPONENT) |
        CC128_ENCODE_FIELD(Te, TOP_ENCODED) |
        CC128_ENCODE_FIELD(Be, BOTTOM_ENCODED);
    // For untagged values we add the initially loaded reserved bits in the reserved field:
    if (!csp->cr_tag)
        pesbt |= CC128_ENCODE_FIELD(CC128_EXTRACT_FIELD(csp->cr_pesbt_xored_for_mem ^ CC128_NULL_XOR_MASK, RESERVED), RESERVED);
    return pesbt;
}

static inline uint64_t compress_128cap(const cap_register_t* csp) {
    return compress_128cap_without_xor(csp) ^ CC128_NULL_XOR_MASK;
}

/*
 * Define the following to do the is_representable() check by simply
 * compressing and decompressing the capability and checking to
 * see if it is the same.
 */
// #define SIMPLE_REPRESENT_CHECK

#ifndef SIMPLE_REPRESENT_CHECK
static inline bool cc128_all_ones(uint64_t offset, uint32_t e, uint32_t bwidth) {
    uint64_t Itop;
    uint32_t shift = e + bwidth;

    if (shift >= 63)
        return false;
    Itop = offset >> shift;
    return Itop == (0xfffffffffffffffful >> shift);
}

static inline bool cc128_all_zeroes(uint64_t offset, uint32_t e, uint32_t bwidth) {
    uint32_t shift = e + bwidth;
    uint64_t Itop;

    if (shift >= 63)
        Itop = 0ul;
    else
        Itop = offset >> shift;
    return Itop == 0ul;
}
#endif /* ! SIMPLE_REPRESENT_CHECK */

static bool fast_cc128_is_representable_new_addr(bool sealed, uint64_t base, cc128_length_t length, uint64_t cursor,
    uint64_t new_cursor);

/// Check that a capability is representable by compressing and recompressing
static bool cc128_is_representable_cap_exact(const cap_register_t* cap) {
    uint64_t pesbt = compress_128cap_without_xor(cap);
    cap_register_t decompressed_cap;
    decompress_128cap_already_xored(pesbt, cap->_cr_cursor, &decompressed_cap);
    // These fields must not change:
    cc128_debug_assert(decompressed_cap.cr_otype == cap->cr_otype);
    cc128_debug_assert(decompressed_cap.cr_uperms == cap->cr_uperms);
    cc128_debug_assert(decompressed_cap.cr_perms == cap->cr_perms);
    // If any of these fields changed then the capability is not representable:
    if (decompressed_cap.cr_base != cap->cr_base ||
        decompressed_cap._cr_top != cap->_cr_top ||
        decompressed_cap._cr_cursor != cap->_cr_cursor) {
        return false;
    }
    return true;
}

/*
 * Check to see if a memory region is representable by a compressed
 * capability. It is representable if:
 *
 *   representable = (inRange && inLimits) || (E >= 44)
 *
 * where:
 *
 *   E = compression exponent (see _cc128_compute_e() above)
 *
 *   inRange = -s < i < s  where i is the increment (or offset)
 *   (In other words, all the bits of i<63, E+20> are the same.)
 *
 *   inLimits = (i < 0) ? (Imid >= (R - Amid)) && (R != Amid) : (R - Amid - 1)
 *   where Imid = i<E+19, E>, Amid = a<E+19, E>, R = B - 2^12 and a =
 *   base + offset.
 */
static inline bool cc128_is_representable_new_addr(bool sealed, uint64_t base, cc128_length_t length, uint64_t cursor, uint64_t new_cursor) {
    // in-bounds capabilities are always representable
    if (__builtin_expect(cursor >= base && new_cursor < length, true)) {
        return true;
    }

#if defined(CC128_USE_FAST_REP_CHECK)
    const bool slow_representable_check = false;
#else
    const bool slow_representable_check = true;
#endif

    if (slow_representable_check) {
        cap_register_t c;
        memset(&c, 0, sizeof(c));
        /* Simply compress and uncompress to check. */
        c.cr_base = base;
        c._cr_top = (cc128_length_t)base + length;
        c._cr_cursor = new_cursor;
        c.cr_otype = sealed ? 42 : CC128_OTYPE_UNSEALED; // important to set as compress assumes this is in bounds
        return cc128_is_representable_cap_exact(&c);
    } else {
        return fast_cc128_is_representable_new_addr(sealed, base, length, cursor, new_cursor);
    }
}

static inline bool cc128_is_representable_with_addr(const cap_register_t* cap, uint64_t new_addr) {
    // in-bounds capabilities are always representable
    if (__builtin_expect(new_addr >= cap->cr_base && new_addr < cap->_cr_top , true)) {
        return true;
    }
    const cc128_length_t length = cap->_cr_top - cap->cr_base;
    return cc128_is_representable_new_addr(cc128_is_cap_sealed(cap), cap->cr_base, length, cap->_cr_cursor, new_addr);
}



static bool fast_cc128_is_representable_new_addr(bool sealed, uint64_t base, cc128_length_t length, uint64_t cursor,
                                        uint64_t new_cursor) {
    (void)sealed;
    uint32_t bwidth = CC128_BOT_WIDTH;
    uint32_t highest_exp = (64 - bwidth + 2);

    cc128_length_t top = base + length;
    // If top is 0xffff... we assume we meant it to be 1 << 64
    if (top == CC128_MAX_TOP && base == 0) {
        return true; // 1 << 65 is always representable
    }
    if (length == 0) {
        return true; // length 0 is always representable
    }

    uint32_t e = cc128_get_exponent(length);

    int64_t b, r, Imid, Amid;
    bool inRange, inLimits;
    int64_t inc = (int64_t)(new_cursor - cursor);

#define MOD_MASK ((UINT64_C(1) << bwidth) - UINT64_C(1))

    /* Check for the boundary cases. */

    b = (int64_t)((base >> e) & MOD_MASK);
    Imid = (int64_t)((uint64_t)(inc >> e) & MOD_MASK);
    Amid = (int64_t)(((cursor) >> e) & MOD_MASK);

    r = (int64_t)(((uint64_t)((b >> (bwidth - 3)) - 1) << (bwidth - 3)) & MOD_MASK);

    /* inRange, test if bits are all the same */
    inRange = cc128_all_ones((uint64_t)inc, e, bwidth) || cc128_all_zeroes((uint64_t)inc, e, bwidth);

    /* inLimits */
    if (inc >= 0) {
        inLimits = ((uint64_t)Imid < (((uint64_t)(r - Amid - 1l)) & MOD_MASK));
    } else {
        inLimits = ((uint64_t)Imid >= (((uint64_t)(r - Amid)) & MOD_MASK)) && (r != Amid);
    }
#undef MOD_MASK

    return ((inRange && inLimits) || (e >= highest_exp));
}

/* @return whether the operation was able to set precise bounds precise or not */
static inline bool cc128_setbounds_impl(cap_register_t* cap, uint64_t req_base, cc128_length_t req_top, uint64_t* alignment_mask) {
    cc128_debug_assert(cap->cr_tag && "Cannot be used on untagged capabilities");
    cc128_debug_assert(!cc128_is_cap_sealed(cap) && "Cannot be used on sealed capabilities");
    cc128_debug_assert(req_base <= req_top && "Cannot grow capabilities");
    /*
     * With compressed capabilities we may need to increase the range of
     * memory addresses to be wider than requested so it is
     * representable.
     */
    const uint64_t cursor = cap->_cr_cursor;
    cc128_debug_assert(req_base == cursor && "CSetbounds should set base to current cursor");
    cc128_length_t orig_length65 = cap->_cr_top - cap->cr_base;
    cc128_length_t req_length65 = req_top - cursor;
    cc128_debug_assert((orig_length65 >> 64) <= 1 && "Length cannot be bigger than 1 << 64");
    cc128_debug_assert((req_top >> 64) <= 1 && "New top cannot be bigger than 1 << 64");
    cc128_debug_assert(req_length65 <= orig_length65 && "Cannot grow capabilities");
    assert((cap->_cr_cursor < cap->_cr_top || (cap->_cr_cursor == cap->_cr_top && cap->_cr_top == cap->cr_base)) &&
           "Must be used on inbounds (or zero-length) caps");
    assert(cap->_cr_cursor >= cap->cr_base && "Must be used on inbounds caps");

    // function setCapBounds(cap, base, top) : (Capability, bits(64), bits(65)) -> (bool, Capability) = {
    //  /* {cap with base=base; length=(bits(64)) length; offset=0} */
    //  let base65 = 0b0 @ base;
    //  let length = top - base65;
    //  /* Find an exponent that will put the most significant bit of length
    //     second from the top as assumed during decoding. We ignore the bottom
    //     MW - 1 bits because those are handled by the ie = 0 format. */
    //  let e = 52 - CountLeadingZeros(length[64..13]);
    // To check that the symbolic constants match sail:
    CC128_STATIC_ASSERT(CC128_BOT_WIDTH == 14, "");
    CC128_STATIC_ASSERT(CC128_BOT_INTERNAL_EXP_WIDTH == 11, "");
    CC128_STATIC_ASSERT(CC128_EXP_LOW_WIDTH == 3, ""); // expected 6-bit exponent
    uint8_t E = (uint8_t)cc128_get_exponent(req_length65);
    const uint64_t req_length64 = (uint64_t)req_length65;
    // Use internal exponent if e is non-zero or if e is zero but
    // but the implied bit of length is not zero (denormal vs. normal case)
    //  let ie = (e != 0) | length[12];
    //
    const bool InternalExponent = E != 0 || cc128_getbits(req_length64, CC128_BOT_INTERNAL_EXP_WIDTH + 1, 1);
    bool exact = false;
    bool incE = false;
    cc128_length_t new_top;
    uint64_t new_base;
    if (!InternalExponent) {
        //  /* The non-ie e == 0 case is easy. It is exact so just extract relevant bits. */
        //  Bbits = truncate(base, 14);
        //  Tbits = truncate(top, 14);
        //  lostSignificantTop  : bool = false;
        //  lostSignificantBase : bool = false;
        //  incE : bool = false;
        exact = true;
        new_top = req_top;
        new_base = req_base;
        if (alignment_mask)
          *alignment_mask = UINT64_MAX; // no adjustment to base required
    } else {
        //  if ie then {
        //    /* the internal exponent case is trickier */
        //
        //    /* Extract B and T bits (we lose 3 bits of each to store the exponent) */
        //    B_ie = truncate(base >> (e + 3), 11);
        //    T_ie = truncate(top >> (e + 3), 11);
        //
        uint64_t bot_ie = cc128_truncate64(req_base >> (E + CC128_EXP_LOW_WIDTH), CC128_BOT_INTERNAL_EXP_WIDTH);
        if (alignment_mask) {
            *alignment_mask = UINT64_MAX << (E + CC128_EXP_LOW_WIDTH);
        }
        uint64_t top_ie = cc128_truncate64((uint64_t)(req_top >> (E + CC128_EXP_LOW_WIDTH)), CC128_BOT_INTERNAL_EXP_WIDTH);
        //    /* Find out whether we have lost significant bits of base and top using a
        //       mask of bits that we will lose (including 3 extra for exp). */
        //    maskLo : bits(65) = zero_extend(replicate_bits(0b1, e + 3));
        //    z65    : bits(65) = zeros();
        //    lostSignificantBase = (base65 & maskLo) != z65;
        //    lostSignificantTop = (top & maskLo) != z65;
        // TODO: stop using cc128_length_t and just handle bit65 set specially?
        const cc128_length_t maskLo = (((cc128_length_t)1u) << (E + CC128_EXP_LOW_WIDTH)) - 1;
        const cc128_length_t zero65 = 0;
        bool lostSignificantBase = (req_base & maskLo) != zero65;
        bool lostSignificantTop = (req_top & maskLo) != zero65;

        //    if lostSignificantTop then {
        //      /* we must increment T to make sure it is still above top even with lost bits.
        //         It might wrap around but if that makes B<T then decoding will compensate. */
        //      T_ie = T_ie + 1;
        //    };
        if (lostSignificantTop) {
            top_ie = cc128_truncate64(top_ie + 1, CC128_BOT_INTERNAL_EXP_WIDTH);
        }
        //    /* Has the length overflowed? We chose e so that the top two bits of len would be 0b01,
        //       but either because of incrementing T or losing bits of base it might have grown. */
        //    len_ie = T_ie - B_ie;
        //    if len_ie[10] then {
        //      /* length overflow -- increment E by one and then recalculate
        //         T, B etc accordingly */
        //      incE = true;
        //
        //      lostSignificantBase = lostSignificantBase | B_ie[0];
        //      lostSignificantTop  = lostSignificantTop | T_ie[0];
        //
        //      B_ie = truncate(base >> (e + 4), 11);
        //      let incT : range(0,1) = if lostSignificantTop then 1 else 0;
        //      T_ie = truncate(top >> (e + 4), 11) + incT;
        //    };
        const uint64_t len_ie = cc128_truncate64(top_ie - bot_ie, CC128_BOT_INTERNAL_EXP_WIDTH);
        if (cc128_getbits(len_ie, CC128_BOT_INTERNAL_EXP_WIDTH - 1, 1)) {
            incE = true;
            lostSignificantBase = lostSignificantBase || cc128_getbits(bot_ie, 0, 1);
            lostSignificantTop = lostSignificantTop || cc128_getbits(top_ie, 0, 1);
            bot_ie = cc128_truncate64(req_base >> (E + CC128_EXP_LOW_WIDTH + 1), CC128_BOT_INTERNAL_EXP_WIDTH);
            // If we had to adjust bot_ie (shift by one more) also update alignment_mask
            if (alignment_mask) {
                *alignment_mask = UINT64_MAX << (E + CC128_EXP_LOW_WIDTH + 1);
            }
            const bool incT = lostSignificantTop;
            top_ie = cc128_truncate64((uint64_t)(req_top >> (E + CC128_EXP_LOW_WIDTH + 1)), CC128_BOT_INTERNAL_EXP_WIDTH);
            if (incT) {
                top_ie = cc128_truncate64(top_ie + 1, CC128_BOT_INTERNAL_EXP_WIDTH);
            }
        }
        //
        //    Bbits = B_ie @ 0b000;
        //    Tbits = T_ie @ 0b000;
        const uint64_t Bbits = bot_ie << CC128_FIELD_EXPONENT_LOW_PART_SIZE;
        const uint64_t Tbits = top_ie << CC128_FIELD_EXPONENT_LOW_PART_SIZE;
        const uint8_t newE = E + (incE ? 1 : 0);

        //  };
        //  let exact = not(lostSignificantBase | lostSignificantTop);
        exact = !lostSignificantBase && !lostSignificantTop;

        // TODO: find a faster way to compute top and bot:
        // Split E between T and B
        const uint64_t expHighBits = cc128_getbits(newE >> CC128_FIELD_EXPONENT_LOW_PART_SIZE , 0, CC128_FIELD_EXPONENT_HIGH_PART_SIZE);
        const uint64_t expLowBits = cc128_getbits(newE, 0, CC128_FIELD_EXPONENT_LOW_PART_SIZE);
        const uint64_t Te = Tbits | expHighBits;
        const uint64_t Be = Bbits | expLowBits;
        const uint64_t pesbt =
            CC128_ENCODE_FIELD(0, UPERMS) |
            CC128_ENCODE_FIELD(0, HWPERMS) |
            CC128_ENCODE_FIELD(CC128_OTYPE_UNSEALED, OTYPE) |
            CC128_ENCODE_FIELD(1, INTERNAL_EXPONENT) |
            CC128_ENCODE_FIELD(Te, TOP_ENCODED) |
            CC128_ENCODE_FIELD(Be, BOTTOM_ENCODED);
        cap_register_t new_cap;
        decompress_128cap_already_xored(pesbt, req_base, &new_cap);
        new_base = new_cap.cr_base;
        new_top = new_cap._cr_top;
    }
    cc128_debug_assert(new_top >= new_base);
    cap->cr_base = new_base;
    cap->_cr_cursor = cursor; // ensure that the address is correct
    cap->_cr_top = new_top;
    // TODO: update pesbt?
    //  let newCap = {cap with address=base, E=to_bits(6, if incE then e + 1 else e), B=Bbits, T=Tbits, internal_e=ie};
    //  (exact, newCap)
    return exact;
}

/* @return whether the operation was able to set precise bounds precise or not */
static inline bool cc128_setbounds(cap_register_t* cap, uint64_t req_base, cc128_length_t req_top) {
    return cc128_setbounds_impl(cap, req_base, req_top, NULL);
}

/* @return the mask that needs to be applied to base in order to get a precisely representable capability */
static inline uint64_t cc128_get_alignment_mask(uint64_t req_length) {
  if (req_length == 0) {
    // With a lenght of zero we know it is precise so we can just return an
    // all ones mask.
    // This avoids undefined behaviour when counting most significant bit later.
    return UINT64_MAX;
  }
  // To compute the mask we set bounds on a maximum permissions capability and
  // return the mask that was used to adjust the length
  cap_register_t tmpcap;
  memset(&tmpcap, 0, sizeof(tmpcap));
  tmpcap.cr_tag = 1;
  tmpcap.cr_base = 0;
  tmpcap._cr_top = CC128_MAX_TOP;
  tmpcap.cr_otype = CC128_OTYPE_UNSEALED;
  uint64_t mask = 0;
  // Ensure that the base always needs rounding down by making it all ones until
  // one bit before the most significant bit in length
  // uint64_t req_base = UINT64_MAX & ~(UINT64_C(1) << cc128_idx_MSNZ(req_length));
  uint64_t req_base = UINT64_MAX - req_length;
  tmpcap._cr_cursor = req_base;
  cc128_setbounds_impl(&tmpcap, req_base, req_base + req_length, &mask);
  // base should have been rounded down using this mask:
  cc128_debug_assert((req_base & mask) == tmpcap.cr_base);
  return mask;
}

static inline uint64_t cc128_get_required_alignment(uint64_t req_length) {
    // To get the required alignment from the CRAM mask we can just invert
    // the bits and add one to get a power-of-two
    return ~cc128_get_alignment_mask(req_length) + 1;
}


#endif /* CC128_DEFINE_FUNCTIONS != 0 */


#if CC256_DEFINE_FUNCTIONS != 0
/* Also support decoding of the raw 256-bit capabilities */
typedef union _inmemory_chericap256 {
    uint8_t bytes[32];
    uint32_t u32s[8];
    uint64_t u64s[4];
} inmemory_chericap256;

static inline bool cc256_is_cap_sealed(const cap_register_t* cp) {
    return cp->cr_otype <= CC256_MAX_SEALED_OTYPE;
}


static inline void decompress_256cap(inmemory_chericap256 mem, cap_register_t* cdp, bool tagged) {
    /* See CHERI ISA: Figure 3.1: 256-bit memory representation of a capability */
    cdp->_sbit_for_memory = mem.u64s[0] & 1;
    uint32_t hwperms_mask = tagged ? CC256_PERMS_ALL_BITS: CC256_PERMS_ALL_BITS_UNTAGGED;
    cdp->cr_perms = (mem.u64s[0] >> CC256_PERMS_MEM_SHFT) & hwperms_mask;
    cdp->cr_uperms = (mem.u64s[0] >> CC256_UPERMS_MEM_SHFT) & CC256_UPERMS_ALL_BITS;
    cdp->cr_otype = (mem.u64s[0] >> CC256_OTYPE_MEM_SHFT) ^ CC256_OTYPE_UNSEALED;
    cdp->cr_base = mem.u64s[2];
    /* Length is xor'ed with -1 to ensure that NULL is all zeroes in memory */
    uint64_t length = mem.u64s[3] ^ CC256_NULL_LENGTH;
    cdp->_cr_top = (cc128_length_t)cdp->cr_base + (cc128_length_t)length;
    cdp->_cr_cursor = mem.u64s[1];
    cdp->cr_tag = tagged;
}

static inline void compress_256cap(inmemory_chericap256* buffer, const cap_register_t* csp) {

    bool sealed_bit = csp->cr_tag ? cc256_is_cap_sealed(csp) : csp->_sbit_for_memory;
    // When writing an untagged value, just write back the bits that were loaded (including the reserved HWPERMS)
    uint64_t hwperms_mask = csp->cr_tag ? CC256_PERMS_ALL_BITS: CC256_PERMS_ALL_BITS_UNTAGGED;
    buffer->u64s[0] =
        sealed_bit |
        ((csp->cr_perms & hwperms_mask) << CC256_PERMS_MEM_SHFT) |
        ((csp->cr_uperms & CC256_UPERMS_ALL_BITS) << CC256_UPERMS_MEM_SHFT) |
        ((uint64_t)(csp->cr_otype ^ CC256_OTYPE_UNSEALED) << CC256_OTYPE_MEM_SHFT);
    buffer->u64s[1] = csp->_cr_cursor;
    buffer->u64s[2] = csp->cr_base;
    cc128_length_t length65 = csp->_cr_top - csp->cr_base;
    uint64_t length64 = length65 > UINT64_MAX ? UINT64_MAX : (uint64_t)length65;
    buffer->u64s[3] = length64 ^ CC256_NULL_LENGTH;
}

#endif /* CC256_DEFINE_FUNCTIONS != 0 */

#endif /* CHERI_COMPRESSED_CAP_H */
