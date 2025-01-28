/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Lawrence Esswood
 * Copyright (c) 2018-2020 Alex Richardson
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract FA8750-10-C-0237
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory (Department of Computer Science and
 * Technology) under DARPA contract HR0011-18-C-0016 ("ECATS"), as part of the
 * DARPA SSITH research programme.
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

#include <stdbool.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
enum {
    // For the reset capability we use an internal exponent and need
    // 2^ADDR_WIDTH, which uses the max exponent.
    _CC_N(RESET_EXP) = _CC_N(MAX_EXPONENT),
#if _CC_N(FIELD_EF_USED) == 1
    _CC_N(RESET_T) = 0,
    // The value that is encoded in { L8, TE, BE }. Risc-v cheri stores the
    // difference between max exponent and the actual exponent.
    _CC_N(RESET_EXP_CODED) = (_CC_N(MAX_EXPONENT) - _CC_N(RESET_EXP)),
#else
    _CC_N(RESET_T) = 1u << (_CC_N(ADDR_WIDTH) - _CC_N(RESET_EXP) - _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE)),
    _CC_N(RESET_EXP_CODED) = _CC_N(RESET_EXP),
#endif

#ifdef CC_IS_MORELLO
    // Due to magic constant XOR aversion (i.e. fields are either entirely
    // inverted or not at all, rather than select bits within them like in
    // normal CHERI Concentrate), NULL is special in Morello.
    _CC_N(NULL_EXP) = _CC_N(MAX_ENCODABLE_EXPONENT),
    _CC_N(NULL_T) = 0,
    _CC_N(NULL_EXP_CODED) = _CC_N(NULL_EXP),
#else
    // NULL uses identical bounds encoding to the reset capability.
    _CC_N(NULL_EXP) = _CC_N(RESET_EXP),
    _CC_N(NULL_T) = _CC_N(RESET_T),
    _CC_N(NULL_EXP_CODED) = _CC_N(RESET_EXP_CODED),
#endif

    _CC_N(RESET_EBT) =
        _CC_ENCODE_EBT_FIELD(1, INTERNAL_EXPONENT) | _CC_ENCODE_EBT_FIELD(0, EF) |
        _CC_ENCODE_EBT_FIELD(_CC_N(RESET_T), EXP_NONZERO_TOP) |
        _CC_ENCODE_EBT_FIELD(0, EXP_NONZERO_BOTTOM) |
        _CC_ENCODE_EBT_FIELD(_CC_N(RESET_EXP_CODED) >> (_CC_N(FIELD_EXPONENT_HIGH_PART_SIZE) + _CC_N(FIELD_EXPONENT_LOW_PART_SIZE)), L8) |
        _CC_ENCODE_EBT_FIELD(_CC_N(RESET_EXP_CODED) >> _CC_N(FIELD_EXPONENT_LOW_PART_SIZE), EXPONENT_HIGH_PART) |
        _CC_ENCODE_EBT_FIELD(_CC_N(RESET_EXP_CODED) & _CC_N(FIELD_EXPONENT_LOW_PART_MAX_VALUE), EXPONENT_LOW_PART),
    /*
     * Please note that we don't have to exclude unused fields.
     * _CC_ENCODE_FIELD(value, field_name) is 0 when the field's size is 0.
     */
    _CC_N(NULL_PESBT) = _CC_ENCODE_FIELD(0, UPERMS) | _CC_ENCODE_FIELD(0, HWPERMS) | _CC_ENCODE_FIELD(0, RESERVED) |
                        _CC_ENCODE_FIELD(0, FLAGS) |
                        _CC_ENCODE_FIELD(0, CT) |
                        _CC_ENCODE_FIELD(0, CL) |
                        _CC_ENCODE_FIELD(1, INTERNAL_EXPONENT) | _CC_ENCODE_FIELD(0, EF) |
                        _CC_ENCODE_FIELD(_CC_N(OTYPE_UNSEALED), OTYPE) |
                        _CC_ENCODE_FIELD(_CC_N(NULL_T), EXP_NONZERO_TOP) | _CC_ENCODE_FIELD(0, EXP_NONZERO_BOTTOM) |
                        _CC_ENCODE_FIELD(_CC_N(NULL_EXP_CODED) >> (_CC_N(FIELD_EXPONENT_HIGH_PART_SIZE) + _CC_N(FIELD_EXPONENT_LOW_PART_SIZE)), L8) |
                        _CC_ENCODE_FIELD(_CC_N(NULL_EXP_CODED) >> _CC_N(FIELD_EXPONENT_LOW_PART_SIZE), EXPONENT_HIGH_PART) |
                        _CC_ENCODE_FIELD(_CC_N(NULL_EXP_CODED) & _CC_N(FIELD_EXPONENT_LOW_PART_MAX_VALUE), EXPONENT_LOW_PART),
    // We mask on store/load so this invisibly keeps null 0 whatever we choose
    // it to be.
    _CC_N(NULL_XOR_MASK) = _CC_N(NULL_PESBT),
};
#pragma GCC diagnostic pop

#define _cc_length_t _cc_N(length_t)
#define _cc_offset_t _cc_N(offset_t)
#define _cc_addr_t _cc_N(addr_t)
#define _cc_saddr_t _cc_N(saddr_t)

#define _CC_MANTISSA_WIDTH _CC_N(MANTISSA_WIDTH)
#define _CC_MAX_EXPONENT _CC_N(MAX_EXPONENT)
#define _CC_BOT_INTERNAL_EXP_WIDTH _CC_N(FIELD_EXP_NONZERO_BOTTOM_SIZE)
#define _CC_EXP_LOW_WIDTH _CC_N(FIELD_EXPONENT_LOW_PART_SIZE)
#define _CC_EXP_HIGH_WIDTH _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE)
#define _CC_ADDR_WIDTH _CC_N(ADDR_WIDTH)
#define _CC_LEN_WIDTH _CC_N(LEN_WIDTH)
#define _CC_MAX_ADDR _CC_N(MAX_ADDR)
#define _CC_MAX_TOP _CC_N(MAX_TOP)
#define _CC_CURSOR_MASK _CC_N(CURSOR_MASK)
// Check that the sizes of the individual fields match up
_CC_STATIC_ASSERT_SAME(_CC_N(FIELD_EBT_SIZE) + _CC_N(FIELD_OTYPE_SIZE) + _CC_N(FIELD_FLAGS_SIZE) +
                           _CC_N(FIELD_CT_SIZE) + _CC_N(FIELD_CL_SIZE) +
                           _CC_N(FIELD_M_SIZE) + _CC_N(FIELD_AP_SIZE) + _CC_N(FIELD_SDP_SIZE) +
                           _CC_N(FIELD_RESERVED_SIZE) + _CC_N(FIELD_RESERVED2_SIZE) +
                           _CC_N(FIELD_HWPERMS_SIZE) + _CC_N(FIELD_UPERMS_SIZE),
                       _CC_ADDR_WIDTH);
/*
 * Please note that only one of internal exponent or EF will be used at a
 * time. Unused fields have size 0, they don't have to be commented out.
 */
_CC_STATIC_ASSERT_SAME(_CC_N(FIELD_INTERNAL_EXPONENT_SIZE) + _CC_N(FIELD_EF_SIZE) +_CC_N(FIELD_L8_SIZE) +
                           _CC_N(FIELD_EXP_ZERO_TOP_SIZE) + _CC_N(FIELD_EXP_ZERO_BOTTOM_SIZE),
                       _CC_N(FIELD_EBT_SIZE));
_CC_STATIC_ASSERT_SAME(_CC_N(FIELD_INTERNAL_EXPONENT_SIZE) + _CC_N(FIELD_EF_SIZE) +_CC_N(FIELD_L8_SIZE) +
                           _CC_N(FIELD_TOP_ENCODED_SIZE) + _CC_N(FIELD_BOTTOM_ENCODED_SIZE),
                       _CC_N(FIELD_EBT_SIZE));
_CC_STATIC_ASSERT_SAME(_CC_N(FIELD_INTERNAL_EXPONENT_SIZE) + _CC_N(FIELD_EF_SIZE) +_CC_N(FIELD_L8_SIZE) +
                           _CC_N(FIELD_EXP_NONZERO_TOP_SIZE) + _CC_N(FIELD_EXP_NONZERO_BOTTOM_SIZE) +
                           _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE) + _CC_N(FIELD_EXPONENT_LOW_PART_SIZE),
                       _CC_N(FIELD_EBT_SIZE));

// Sanity-check the min/max otype macros:
_CC_STATIC_ASSERT(_CC_N(MIN_RESERVED_OTYPE) >= 0, "MIN_RESERVED_OTYPE is signed?");
_CC_STATIC_ASSERT(_CC_N(MIN_RESERVED_OTYPE) < _CC_N(MAX_RESERVED_OTYPE),
                  "MIN_RESERVED_OTYPE greater than MAX_RESERVED_OTYPE?");
_CC_STATIC_ASSERT(_CC_N(MIN_RESERVED_OTYPE) <= _CC_N(MAX_REPRESENTABLE_OTYPE), "MIN_RESERVED_OTYPE out of range?");
_CC_STATIC_ASSERT(_CC_N(MAX_RESERVED_OTYPE) <= _CC_N(MAX_REPRESENTABLE_OTYPE), "MAX_RESERVED_OTYPE out of range?");

// Forward-declare the accessors since we use them inside the struct body:
typedef struct _cc_N(cap) _cc_N(cap_t);
#define _cc_cap_t _cc_N(cap_t)
static inline uint8_t _cc_N(get_flags)(const _cc_cap_t* cap);
static inline uint32_t _cc_N(get_otype)(const _cc_cap_t* cap);
static inline uint32_t _cc_N(get_perms)(const _cc_cap_t* cap);
static inline uint16_t _cc_N(get_ap)(const _cc_cap_t* cap);
static inline uint32_t _cc_N(get_reserved)(const _cc_cap_t* cap);
static inline uint32_t _cc_N(get_uperms)(const _cc_cap_t* cap);
static inline uint8_t _cc_N(get_sdp)(const _cc_cap_t* cap);
static inline uint8_t _cc_N(get_ct)(const _cc_cap_t* cap);
static inline uint8_t _cc_N(get_cl)(const _cc_cap_t* cap);

// In order to allow vector loads and store from memory we can optionally reverse the first two fields.
struct _cc_N(cap) {
    /* offset = cursor - base */
#ifdef _CC_REVERSE_PESBT_CURSOR_ORDER
    /* Original PESBT from the decompressed capability. If you modify
     * other fields, you must be sure to either recalculate this field to match */
    _cc_addr_t cr_pesbt;
    _cc_addr_t _cr_cursor; /* Capability cursor */
#else
    _cc_addr_t _cr_cursor;
    _cc_addr_t cr_pesbt;
#endif
    _cc_length_t _cr_top;    /* Capability top */
    _cc_addr_t cr_base;      /* Capability base addr */
    uint8_t cr_tag;          /* Tag */
    uint8_t cr_bounds_valid; /* Set if bounds decode was given an invalid cap */
    uint8_t cr_exp;          /* Exponent */
    uint8_t cr_extra;        /* Additional data stored by the caller */
    uint16_t cr_arch_perm;   /* decoded architectural permissions (AP) */
    uint8_t cr_m;            /* decoded M bit (or a copy of the bit in pesbt) */
    uint8_t cr_lvbits;       /* lvbits for Zcherilevel (0 if unsupported) */
#ifdef __cplusplus
    inline _cc_addr_t base() const { return cr_base; }
    inline _cc_addr_t address() const { return _cr_cursor; }
    inline _cc_offset_t offset() const { return (_cc_offset_t)_cr_cursor - (_cc_offset_t)cr_base; }
    inline _cc_length_t top() const { return _cr_top; }
    inline _cc_addr_t top64() const {
        const _cc_length_t t = top();
        return t > _CC_MAX_ADDR ? _CC_MAX_ADDR : (_cc_addr_t)t;
    }
    inline _cc_length_t length() const { return _cr_top - cr_base; }
    inline _cc_addr_t length64() const {
        const _cc_length_t l = length();
        return l > _CC_MAX_ADDR ? _CC_MAX_ADDR : (_cc_addr_t)l;
    }
    inline uint32_t software_permissions() const {
#if _CC_N(FIELD_UPERMS_USED)
        return _cc_N(get_uperms)(this);
#else
        return _cc_N(get_sdp)(this);
#endif
    }
    inline uint32_t permissions() const {
#if _CC_N(FIELD_HWPERMS_USED)
        return _cc_N(get_perms)(this);
#else
        /*
         * This assumes that the capability is "decompressed", i.e.
         * cr_arch_perm is in sync with the encoded AP field.
         * (We can't change a struct member in a const function.)
         *
         * It seems that applications create capabilities via
         * _cc_N(make_max_perms_cap), _cc_N(make_null_derived_cap) or
         * _cc_N(decompress_mem). In thoses cases, we should be ok.
         */
        return cr_arch_perm;
#endif
    }
    inline uint8_t global() const {
#if _CC_N(FIELD_CL_USED)
        return _cc_N(get_cl)(this);
#else
        return (permissions() & _CC_N(PERM_GLOBAL)) != 0;
#endif
    }
    inline uint32_t type() const { return _cc_N(get_otype)(this); }
    inline bool is_sealed() const {
#if _CC_N(FIELD_OTYPE_USED) == 1
        return type() != _CC_N(OTYPE_UNSEALED);
#else
        return _cc_N(get_ct)(this);
#endif
    }
    inline uint32_t reserved_bits() const { return _cc_N(get_reserved)(this); }
    inline uint8_t flags() const { return _cc_N(get_flags)(this); }
    inline bool operator==(const _cc_N(cap) & other) const;
#endif
};

static inline bool _cc_N(exactly_equal)(const _cc_cap_t* a, const _cc_cap_t* b) {
    return a->cr_tag == b->cr_tag && a->_cr_cursor == b->_cr_cursor && a->cr_pesbt == b->cr_pesbt;
}

static inline bool _cc_N(raw_equal)(const _cc_cap_t* a, const _cc_cap_t* b) {
    return a->_cr_cursor == b->_cr_cursor && a->cr_pesbt == b->cr_pesbt && a->_cr_top == b->_cr_top &&
           a->cr_base == b->cr_base && a->cr_tag == b->cr_tag && a->cr_bounds_valid == b->cr_bounds_valid &&
           a->cr_exp == b->cr_exp && a->cr_extra == b->cr_extra;
}

/* Returns the index of the most significant bit set in x */
static inline uint32_t _cc_N(idx_MSNZ)(uint64_t x) {
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
    _cc_debug_assert(x != 0);
    uint32_t r = 63u - (uint32_t)__builtin_clzll(x);
#endif
    return r;
}

/*
 * e = idxMSNZ( (rlength + (rlength >> 6)) >> 19 )
 * where (rlength + (rlength >> 6)) needs to be a 65 bit integer
 */
static inline uint32_t _cc_N(compute_e)(_cc_addr_t rlength, uint32_t bwidth) {
    if (rlength < (1u << (bwidth - 1)))
        return 0;

    return (_cc_N(idx_MSNZ)(rlength) - (bwidth - 2));
}

static inline uint32_t _cc_N(get_exponent)(_cc_length_t length) {
    const uint32_t bwidth = _CC_MANTISSA_WIDTH;
    if (length > _CC_MAX_ADDR) {
        return _CC_LEN_WIDTH - (bwidth - 1);
    } else {
        return _cc_N(compute_e)((_cc_addr_t)length, bwidth);
    }
}

static inline uint64_t _cc_N(getbits)(uint64_t src, uint32_t start, uint32_t size) {
    return ((src >> start) & ((UINT64_C(1) << size) - UINT64_C(1)));
}

// truncates `value`, keeping only the _least_ significant `n` bits.
static inline uint64_t _cc_N(truncate_addr)(_cc_addr_t value, size_t n) { return value & (((_cc_addr_t)1 << n) - 1); }
static inline uint64_t _cc_N(truncate64)(uint64_t value, size_t n) { return value & ((UINT64_C(1) << n) - 1); }

// truncates `value`, keeping only the _most_ significant `n` bits.
#define TRUNCATE_LSB_FUNC(type_width)                                                                                  \
    static inline uint64_t _CC_CONCAT(_cc_N(truncateLSB_), type_width)(uint64_t value, size_t n) {                     \
        _CC_STATIC_ASSERT(type_width <= 64, "");                                                                       \
        return value >> (type_width - n);                                                                              \
    }
TRUNCATE_LSB_FUNC(_CC_MANTISSA_WIDTH)
TRUNCATE_LSB_FUNC(32)
TRUNCATE_LSB_FUNC(64)

#define _cc_truncateLSB_generic(type_width) _CC_CONCAT(_cc_N(truncateLSB_), _CC_EXPAND(type_width))
#define _cc_truncateLSB(type_width) _cc_N(_CC_CONCAT(truncateLSB_, type_width))

struct _cc_N(bounds_bits) {
    uint16_t B; // bottom bits (currently 14 bits)
    uint16_t T; // top bits (12 bits plus two implied bits)
    // for risc-v cheri, the exponent is max_exp - { L8 (if present), TE, BE },
    // this may become negative for invalid encodings
    int8_t E;
    union {
        bool IE; // cheri v9's internal exponent flag
        bool EF; // cheri risc-v cheri's exponent format
    };
};
#define _cc_bounds_bits struct _cc_N(bounds_bits)

#define ALL_WRAPPERS(X, FN, type)                                                                                      \
    static inline _cc_addr_t _cc_N(cap_pesbt_extract_##FN)(_cc_addr_t pesbt) { return _CC_EXTRACT_FIELD(pesbt, X); }   \
    static inline _cc_addr_t _cc_N(cap_pesbt_encode_##FN)(type value) { return _CC_ENCODE_FIELD(value, X); }           \
    static inline _cc_addr_t _cc_N(cap_pesbt_deposit_##FN)(_cc_addr_t pesbt, type value) {                             \
        _cc_debug_assert(value <= _CC_N(FIELD_##X##_MAX_VALUE));                                                       \
        return (pesbt & ~_CC_N(FIELD_##X##_MASK64)) | _CC_ENCODE_FIELD(value, X);                                      \
    }                                                                                                                  \
    static inline type _cc_N(get_##FN)(const _cc_cap_t* cap) { return _cc_N(cap_pesbt_extract_##FN)(cap->cr_pesbt); }  \
    static inline void _cc_N(update_##FN)(_cc_cap_t * cap, _cc_addr_t value) {                                         \
        cap->cr_pesbt = _cc_N(cap_pesbt_deposit_##FN)(cap->cr_pesbt, value);                                           \
    }
// M and AP accessors must not be called from outside this library. External
// access must go through cr_arch_perm and cr_m.
// TODO: Is there a way to enforce this? Do we need ALL_WRAPPERS_INTERNAL()?
ALL_WRAPPERS(M, m, uint8_t)
ALL_WRAPPERS(AP, ap, uint16_t)
ALL_WRAPPERS(SDP, sdp, uint8_t)
ALL_WRAPPERS(CL, cl, uint8_t)
ALL_WRAPPERS(HWPERMS, perms, uint32_t)
ALL_WRAPPERS(UPERMS, uperms, uint32_t)
ALL_WRAPPERS(OTYPE, otype, uint32_t)
ALL_WRAPPERS(CT, ct, uint8_t)
ALL_WRAPPERS(FLAGS, flags, uint8_t)
ALL_WRAPPERS(RESERVED, reserved, uint32_t)
ALL_WRAPPERS(RESERVED2, reserved2, uint32_t)
#undef ALL_WRAPPERS

/// Extract the bits used for bounds and infer the top two bits of T
static inline _cc_bounds_bits _cc_N(extract_bounds_bits)(_cc_addr_t pesbt) {
    _CC_STATIC_ASSERT(_CC_MANTISSA_WIDTH == _CC_N(BOT_WIDTH), "Wrong bot width?");
    uint32_t BWidth = _CC_MANTISSA_WIDTH;
    uint32_t BMask = (1u << BWidth) - 1;
    uint32_t TMask = BMask >> 2;
    _cc_bounds_bits result;
    _CC_STATIC_ASSERT(sizeof(result.B) * __CHAR_BIT__ >= _CC_MANTISSA_WIDTH, "B field too small");
    _CC_STATIC_ASSERT(sizeof(result.T) * __CHAR_BIT__ >= _CC_MANTISSA_WIDTH, "T field too small");
    _CC_STATIC_ASSERT(sizeof(result.E) * __CHAR_BIT__ >=
                          _CC_N(FIELD_EXPONENT_LOW_PART_SIZE) + _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE),
                      "E field too small");
    uint8_t L_msb;
#if _CC_N(FIELD_EF_USED) == 1
    result.EF = (bool)(uint32_t)_CC_EXTRACT_FIELD(pesbt, EF);
    if (!result.EF) {
        uint8_t e_enc = (uint8_t)(_CC_EXTRACT_FIELD(pesbt, EXPONENT_LOW_PART) |
                (_CC_EXTRACT_FIELD(pesbt, EXPONENT_HIGH_PART) << _CC_N(FIELD_EXPONENT_LOW_PART_SIZE)) |
                (_CC_EXTRACT_FIELD(pesbt, L8) << ( _CC_N(FIELD_EXPONENT_LOW_PART_SIZE) + _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE))));
        if (e_enc > _CC_MAX_EXPONENT) {
            /* The capability is malformed. */
            memset(&result, 0x0, sizeof(result));
            return result;
        }
        /*
         * This may become negative, such an exponent is invalid. Generally, we
         * extract the values here and check them in _cc_N(bounds_bits_valid).
         */
        result.E = _CC_MAX_EXPONENT - e_enc;
#else
    result.IE = (bool)(uint32_t)_CC_EXTRACT_FIELD(pesbt, INTERNAL_EXPONENT);
    if (result.IE) {
        result.E = (uint8_t)(_CC_EXTRACT_FIELD(pesbt, EXPONENT_LOW_PART) |
                             (_CC_EXTRACT_FIELD(pesbt, EXPONENT_HIGH_PART) << _CC_N(FIELD_EXPONENT_LOW_PART_SIZE)));
        // Do not offset by 1! We also need to encode E=0 even with IE
        // Also allow nonsense values over 64 - BWidth + 2: this is expected by sail-generated tests
        // E = MIN(64 - BWidth + 2, E);
#endif
        result.B = (uint16_t)_CC_EXTRACT_FIELD(pesbt, EXP_NONZERO_BOTTOM) << _CC_N(FIELD_EXPONENT_LOW_PART_SIZE);
        result.T = (uint16_t)_CC_EXTRACT_FIELD(pesbt, EXP_NONZERO_TOP) << _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE);
        L_msb = 1;
    } else {
        // So, I cheated by inverting E on memory load (to match the rest of CHERI), which Morello does not do.
        // This means parts of B and T are incorrectly inverted. So invert back again.
#ifdef CC_IS_MORELLO
        pesbt ^= _CC_N(NULL_XOR_MASK);
#endif
        result.E = 0;
        /* This returns 0 if the current format does not use an L8 field. */
        L_msb = _CC_EXTRACT_FIELD(pesbt, L8);
        result.B = (uint16_t)_CC_EXTRACT_FIELD(pesbt, EXP_ZERO_BOTTOM);
        result.T = (uint16_t)_CC_EXTRACT_FIELD(pesbt, EXP_ZERO_TOP);
    }
    /*
        Reconstruct top two bits of T given T = B + len and:
        1) the top two bits of B
        2) most significant two bits of length derived from format above
        3) carry out of B[20..0] + len[20..0] that is implied if T[20..0] < B[20..0]
    */
    uint8_t L_carry = result.T < (result.B & TMask) ? 1 : 0;
    uint64_t BTop2 = _cc_N(getbits)(result.B, _CC_MANTISSA_WIDTH - 2, 2);
    uint8_t T_infer = (BTop2 + L_carry + L_msb) & 0x3;
    result.T |= ((uint16_t)T_infer) << (BWidth - 2);
    return result;
}

// Certain bit patterns can result in invalid bounds bits. These values must never be tagged!
static inline bool _cc_N(bounds_bits_valid)(_cc_bounds_bits bounds) {
    _cc_addr_t Bmsb = _cc_N(getbits)(bounds.B, _CC_MANTISSA_WIDTH - 1, 1);
#if _CC_N(FIELD_EF_USED) == 0
    // https://github.com/CTSRD-CHERI/sail-cheri-riscv/blob/7a308ef3661e43461c8431c391aaece7fba6e992/src/cheri_properties.sail#L104
    _cc_addr_t Bmsb2 = _cc_N(getbits)(bounds.B, _CC_MANTISSA_WIDTH - 2, 2);
    _cc_addr_t Tmsb = _cc_N(getbits)(bounds.T, _CC_MANTISSA_WIDTH - 1, 1);
    if (bounds.E >= _CC_MAX_EXPONENT) {
        return Tmsb == 0 && Bmsb2 == 0;
    } else if (bounds.E == _CC_MAX_EXPONENT - 1) {
        return Bmsb == 0;
    }
#else
    /*
     * Perform the malformed capability bounds checks as defined in section
     * 2.2.6 of the cheri risc-v cheri specification.
     */
    if (!bounds.EF) {
        if (bounds.E < 0) {
            return false;
#if _CC_N(FIELD_L8_USED) == 1
        } else if (bounds.E == 0) {
            return false;
#endif
        } else if (bounds.E == _CC_MAX_EXPONENT - 1) {
            return Bmsb == 0;
        } else if (bounds.E == _CC_MAX_EXPONENT) {
            return bounds.B == 0;
        }
    }
#endif
    return true;
}

/// Returns the address with Morello flags (high address bits) removed and sign extended.
/// This is currently an no-op for non-Morello but that may change in the future.
static inline _cc_addr_t _cc_N(cap_bounds_address)(_cc_addr_t addr) {
    // Remove flags bits
    _cc_addr_t cursor = addr & _CC_CURSOR_MASK;
    // Sign extend
    if (cursor & ((_CC_CURSOR_MASK >> 1) + 1))
        cursor |= ~_CC_CURSOR_MASK;
    return cursor;
}

static inline bool _cc_N(compute_base_top)(_cc_bounds_bits bounds, _cc_addr_t cursor, _cc_addr_t* base_out,
                                           _cc_length_t* top_out) {
#ifdef CC_IS_MORELLO
    if (bounds.E > _CC_MAX_EXPONENT) {
        bool valid = bounds.E == _CC_N(MAX_ENCODABLE_EXPONENT);
        *base_out = 0;
        *top_out = _CC_N(MAX_TOP);
        return valid;
    }
#endif
    cursor = _cc_N(cap_bounds_address)(cursor);

    // For the remaining computations we have to clamp E to max_E
    //  let E = min(maxE, unsigned(c.E)) in
    // For risc-v cheri, a negative bounds.E is an error that'll be caught later.
    // We can use any E for the calculations, the result will be discarded.
    uint8_t E = bounds.E > 0 ?  _CC_MIN(_CC_MAX_EXPONENT, bounds.E) : 0;

#if _CC_N(FIELD_EF_USED) == 1
    // let a_mid = truncate(a >> E, cap_mantissa_width) in
    uint16_t a_mid = (uint16_t)_cc_N(truncate_addr)(cursor >> E, _CC_MANTISSA_WIDTH);
    // let R = c.B - (0b01 @ zeros(cap_mantissa_width - 2)) in /* wraps */
    uint16_t R = (int16_t)bounds.B - (1U << (_CC_MANTISSA_WIDTH - 2));
    R %= (1U << _CC_MANTISSA_WIDTH);

    // let aHi = if a_mid <_u R then 1 else 0 in
    int aHi = a_mid < R ? 1 : 0;
    // let bHi = if c.B   <_u R then 1 else 0 in
    int bHi = bounds.B < R ? 1 : 0;
    // let tHi = if c.T   <_u R then 1 else 0 in
    int tHi = bounds.T < R ? 1 : 0;
#else
    /* Extract bits we need to make the top correction and calculate representable limit */
    // let a3 = truncate(a >> (E + mantissa_width - 3), 3) in
    // let B3 = truncateLSB(c.B, 3) in
    // let T3 = truncateLSB(c.T, 3) in
    unsigned a3 = (unsigned)_cc_N(truncate64)(cursor >> (E + _CC_MANTISSA_WIDTH - 3), 3);
    unsigned B3 = (unsigned)_cc_truncateLSB(_CC_MANTISSA_WIDTH)(bounds.B, 3);
    unsigned T3 = (unsigned)_cc_truncateLSB(_CC_MANTISSA_WIDTH)(bounds.T, 3);
    // let R3 = B3 - 0b001 in /* wraps */
    unsigned R3 = (unsigned)_cc_N(truncate64)(B3 - 1, 3); // B3 == 0 ? 7 : B3 - 1;
    /* Do address, base and top lie in the R aligned region above the one containing R? */
    // let aHi = if a3 <_u R3 then 1 else 0 in
    // let bHi = if B3 <_u R3 then 1 else 0 in
    // let tHi = if T3 <_u R3 then 1 else 0 in
    int aHi = a3 < R3 ? 1 : 0;
    int bHi = B3 < R3 ? 1 : 0;
    int tHi = T3 < R3 ? 1 : 0;
#endif

    /* Compute region corrections for top and base relative to a */
    // let correction_base = bHi - aHi in
    // let correction_top  = tHi - aHi in
    int correction_base = bHi - aHi;
    int correction_top = tHi - aHi;
    // Note: shifting by 64 is UB and causes wrong results -> clamp the shift value!
    const unsigned a_top_shift = E + _CC_MANTISSA_WIDTH;
    // let a_top = (a >> (E + mantissa_width)) in
    _cc_addr_t a_top = a_top_shift >= _CC_ADDR_WIDTH ? 0 : cursor >> a_top_shift;

    // base : CapLenBits = truncate((a_top + correction_base) @ c.B @ zeros(E), cap_len_width);
    _cc_length_t base = (_cc_addr_t)((int64_t)a_top + correction_base);
    base <<= _CC_MANTISSA_WIDTH;
    base |= bounds.B;
    base <<= E;
    base &= ((_cc_length_t)1 << _CC_LEN_WIDTH) - 1;
    _cc_debug_assert((_cc_addr_t)(base >> _CC_ADDR_WIDTH) <= 1); // max 65/33 bits
    // top  : truncate((a_top + correction_top)  @ c.T @ zeros(E), cap_len_width);
    _cc_length_t top = (_cc_addr_t)((int64_t)a_top + correction_top);
    top <<= _CC_MANTISSA_WIDTH;
    top |= bounds.T;
    top <<= E;
    top &= ((_cc_length_t)1 << _CC_LEN_WIDTH) - 1;
    _cc_debug_assert((_cc_addr_t)(top >> _CC_ADDR_WIDTH) <= 1); // max 65 bits

    /* If the base and top are more than an address space away from each other,
       invert the MSB of top.  This corrects for errors that happen when the
       representable space wraps the address space. */
    //  let base2 : bits(2) = 0b0 @ [base[cap_addr_width - 1]];
    // Note: we ignore the top bit of base here. If we don't we can get cases
    // where setbounds/incoffset/etc. break monotonicity.
    unsigned base2 = _cc_N(truncate64)(base >> (_CC_ADDR_WIDTH - 1), 1);
    //  let top2  : bits(2) = top[cap_addr_width .. cap_addr_width - 1];
    unsigned top2 = _cc_N(truncate64)(top >> (_CC_ADDR_WIDTH - 1), 2);
    //  if (E < (maxE - 1)) & (unsigned(top2 - base2) > 1) then {
    //      top[cap_addr_width] = ~(top[cap_addr_width]);
    //  };
    if (E < (_CC_MAX_EXPONENT - 1) && (top2 - base2) > 1) {
        top = top ^ ((_cc_length_t)1 << _CC_ADDR_WIDTH);
    }

    _cc_debug_assert((_cc_addr_t)(top >> _CC_ADDR_WIDTH) <= 1); // should be at most 1 bit over
    // Check that base <= top for valid inputs
    if (_cc_N(bounds_bits_valid)(bounds)) {
        // Note: base can be > 2^64 for some (untagged) inputs with E near maxE
        // It can also be > top for some (untagged) inputs.
        _cc_debug_assert((_cc_addr_t)base <= top);
    } else {
        // _cc_debug_assert(!tagged && "Should not create invalid tagged capabilities");
#if _CC_N(FIELD_EF_USED) == 1
        /*
         * For cheri risc-v cheri, malformed bounds decode as zero (e.g. for the
         * gcbase and gclen instructions).
         */
        *base_out = 0;
        *top_out = 0;
        return false;
#endif
    }
    *base_out = (_cc_addr_t)base; // strip the (invalid) top bit
    *top_out = top;

    return true;
}

#define CAP_AP_C   (1 << 0)
#define CAP_AP_W   (1 << 1)
#define CAP_AP_R   (1 << 2)
#define CAP_AP_X   (1 << 3)
#define CAP_AP_ASR (1 << 4)
#define CAP_AP_LM  (1 << 5)
#define CAP_AP_EL  (1 << 6)
#define CAP_AP_SL  (1 << 7)

#if _CC_N(M_AP_FCTS) == M_AP_FCTS_NONE
static inline void _cc_N(m_ap_compress)(__attribute__((unused)) _cc_cap_t *cap)
{
}

static inline void _cc_N(m_ap_decompress)(__attribute__((unused)) _cc_cap_t *cap)
{
}
#elif _CC_N(M_AP_FCTS) == M_AP_FCTS_IDENT
static inline void _cc_N(m_ap_compress)(_cc_cap_t *cap)
{
    if ((cap->cr_lvbits == 0) && (cap->cr_arch_perm & (CAP_AP_SL | CAP_AP_EL))) {
      cap->cr_arch_perm &= ~(CAP_AP_SL | CAP_AP_EL);
    }

    _cc_N(update_ap)(cap, cap->cr_arch_perm);
    _cc_N(update_m)(cap, cap->cr_m);
}

static inline void _cc_N(m_ap_decompress)(_cc_cap_t *cap)
{
    cap->cr_arch_perm = _cc_N(get_ap)(cap);
    if ((cap->cr_lvbits == 0) && (cap->cr_arch_perm & (CAP_AP_SL | CAP_AP_EL))) {
      cap->cr_arch_perm &= ~(CAP_AP_SL | CAP_AP_EL);
    }
    cap->cr_m = _cc_N(get_m)(cap);
}
#elif _CC_N(M_AP_FCTS) == M_AP_FCTS_QUADR

#define CAP_AP_Q0 ((uint8_t)(0b00 <<3))
#define CAP_AP_Q1 ((uint8_t)(0b01 <<3))
#define CAP_AP_Q2 ((uint8_t)(0b10 <<3))
#define CAP_AP_Q3 ((uint8_t)(0b11 <<3))

#define CAP_AP_Q_MASK ((uint8_t)(0b11 <<3))

#define EL_OPT(cap) (((cap)->cr_lvbits == 0) ? 0 : CAP_AP_EL)
#define SL_OPT(cap) (((cap)->cr_lvbits == 0) ? 0 : CAP_AP_SL)

static inline void _cc_N(m_ap_compress)(_cc_cap_t *cap)
{
    uint8_t res = 0;

    if (cap->cr_arch_perm & CAP_AP_X) {
      res |= CAP_AP_Q1;
      if (cap->cr_m) {
          res |= 1;
      }
      switch (cap->cr_arch_perm &
              (CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)) {
          case CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR:
              res |= 0;
              break;
          case CAP_AP_R | CAP_AP_C | CAP_AP_LM:
              res |= 2;
              break;
          case CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM:
              res |= 4;
              break;
          case CAP_AP_R | CAP_AP_W:
              res |= 6;
              break;
          default:
              /* We set res = UINT8_MAX to indicate an error. */
              res = UINT8_MAX;
      }
    }
    else if (cap->cr_m) {
        /* M is valid only in Q1. Otherwise, M is reserved and must be 0. */
        res = UINT8_MAX;
    }
    else if ((cap->cr_arch_perm &
                (CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_X | CAP_AP_ASR)) ==
            (CAP_AP_R | CAP_AP_C | CAP_AP_LM | EL_OPT(cap))) {
      res |= CAP_AP_Q3;

      switch (cap->cr_arch_perm & (CAP_AP_W | SL_OPT(cap))) {
          case 0:
              res |= 3;
              break;
          case CAP_AP_W | CAP_AP_SL:
              res |= 6;
              break;
          case CAP_AP_W:
              res |= 7;
              break;
          default:
              res = UINT8_MAX;
      }
    }
    else if ((cap->cr_arch_perm &
                (CAP_AP_R | CAP_AP_C | CAP_AP_EL | CAP_AP_X | CAP_AP_ASR )) ==
            (CAP_AP_R | CAP_AP_C)) {
      res |= CAP_AP_Q2;
      switch (cap->cr_arch_perm & (CAP_AP_W | CAP_AP_LM | SL_OPT(cap))) {
          case 0:
              res |= 3;
              break;
          case CAP_AP_W | CAP_AP_LM | CAP_AP_SL:
              res |= 6;
              break;
          case CAP_AP_W | CAP_AP_LM:
              res |= 7;
              break;
          default:
              res = UINT8_MAX;
      }
    }
    else {
      res |= CAP_AP_Q0;

      if (cap->cr_arch_perm & (CAP_AP_C | CAP_AP_X | CAP_AP_ASR)) {
          res = UINT8_MAX;
      }
      else {
          switch (cap->cr_arch_perm & (CAP_AP_R | CAP_AP_W)) {
              case 0:
                  break;
              case CAP_AP_R:
                  res |= 1;
                  break;
              case CAP_AP_W:
                  res |= 4;
                  break;
              case CAP_AP_R | CAP_AP_W:
                  res |= 5;
                  break;
          }
      }
    }

    /*
     * TODO: We should warn about invalid permissions here.
     * _cc_debug_assert(res != UINT8_MAX) is too strong, we want to continue
     * running so the testsuite can test our error handling.
     */
    if (res == UINT8_MAX) {
        res = 0;
        cap->cr_arch_perm = 0;
    }
    _cc_N(update_ap)(cap, res);
}

/*
 * This shifts the lvbits value when we want to check both lvbits and the
 * permission encoding (without the quadrant info) at once.
 *
 * __builtin_ctz has been supported in gcc since v3.4.6 - it's tricky to
 * check if it is available (__has_builtin was added to gcc 10)
 * clang supports __builtin_ctz as well
 */
#define __LVBITS(x) ((x) << __builtin_ctz(CAP_AP_Q_MASK))

static inline void _cc_N(m_ap_decompress)(_cc_cap_t *cap)
{
    uint8_t perm_comp = _cc_N(get_ap)(cap);
    bool m_bit = false;
    uint8_t res = 0;

    /*
     * For the internal processing below, we assume that EL, SL are used.
     * If the caller does not use EL and SL, we clear those bits before we
     * return the bitmask.
     */

    switch (perm_comp & CAP_AP_Q_MASK) {
        case CAP_AP_Q0:
            switch (perm_comp & ~CAP_AP_Q_MASK) {
                case 0:
                    break;
                case 1:
                    res |= CAP_AP_R;
                    break;
                case 4:
                    res |= CAP_AP_W;
                    break;
                case 5:
                    res |= CAP_AP_R | CAP_AP_W;
                    break;
                default:
                    /*
                     * Unsupported encoding in quadrant 0. All permissions are
                     * implicitly denied.
                     */
                    res = 0;
            }
            break;
        case CAP_AP_Q1:
            res |= CAP_AP_X;
            if (perm_comp & 1) {
                m_bit = true;
            }
            switch ((perm_comp & ~CAP_AP_Q_MASK) >> 1) {
                case 0:
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_X | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL;
                    break;
                case 1:
                    res |= CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_X | CAP_AP_EL | CAP_AP_SL;
                    break;
                case 2:
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_X | CAP_AP_EL | CAP_AP_SL;
                    break;
                case 3:
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_X;
                    break;
                default:
                    /* TODO: this cannot happen */
                    res = 0;
                    break;
            }
            break;
        case CAP_AP_Q2:
            switch (__LVBITS(cap->cr_lvbits) | (perm_comp & ~CAP_AP_Q_MASK)) {
                case __LVBITS(0) | 3:
                case __LVBITS(1) | 3:
                    res |= CAP_AP_R | CAP_AP_C;
                    break;
                /* 4 and 5 are reserved for lvbits=2, we don't support this */
                case __LVBITS(1) | 6:
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_SL;
                    break;
                case __LVBITS(1) | 7:
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM;
                    break;
                default:
                    /*
                     * Unsupported encoding in quadrant 2. All permissions are
                     * implicitly granted.
                     */
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_X | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL;
            }
            break;
        case CAP_AP_Q3:
            switch (__LVBITS(cap->cr_lvbits) | (perm_comp & ~CAP_AP_Q_MASK)) {
                case __LVBITS(0) | 3:
                case __LVBITS(1) | 3:
                    res |= CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL;
                    break;
                /* 4 and 5 are reserved for lvbits=2, we don't support this */
                case __LVBITS(1) | 6:
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL;
                    break;
                case __LVBITS(0) | 7:
                case __LVBITS(1) | 7:
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL;
                    break;
                default:
                    /*
                     * Unsupported encoding in quadrant 3. All permissions are
                     * implicitly granted.
                     */
                    res |= CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_X | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL;
            }
            break;
    }

    if (cap->cr_lvbits == 0) {
        res &= ~(CAP_AP_EL | CAP_AP_SL);
    }
    cap->cr_arch_perm = res;
    cap->cr_m = m_bit ? 1 : 0;
}
#endif

/// Expand a PESBT+address+tag input to a _cc_cap_t, but don't check that the tagged value is derivable.
/// This is an internal helper and should not not be used outside of this header.
static inline void _cc_N(unsafe_decompress_raw)(_cc_addr_t pesbt, _cc_addr_t cursor, bool tag, uint8_t lvbits, _cc_cap_t* cdp) {
    memset(cdp, 0, sizeof(*cdp));
    cdp->cr_tag = tag;
    cdp->_cr_cursor = cursor;
    cdp->cr_pesbt = pesbt;
    cdp->cr_lvbits = lvbits;

    _cc_bounds_bits bounds = _cc_N(extract_bounds_bits)(pesbt);
    bool valid = _cc_N(compute_base_top)(bounds, cursor, &cdp->cr_base, &cdp->_cr_top);
    cdp->cr_bounds_valid = valid;
    cdp->cr_exp = bounds.E;
    _cc_N(m_ap_decompress)(cdp);
}

static inline void _cc_N(decompress_raw__)(_cc_addr_t pesbt, _cc_addr_t cursor, bool tag, uint8_t lvbits,_cc_cap_t* cdp) {
    _cc_N(unsafe_decompress_raw)(pesbt, cursor, tag, lvbits, cdp);
    if (tag) {
        _cc_debug_assert(cdp->cr_base <= _CC_N(MAX_ADDR));
#ifndef CC_IS_MORELLO
        // Morello is perfectly happy using settag to create capabilities with length greater than 2^64.
        _cc_debug_assert(cdp->_cr_top <= _CC_N(MAX_TOP));
        _cc_debug_assert(cdp->cr_base <= cdp->_cr_top);
#endif
        _cc_debug_assert(_CC_EXTRACT_FIELD(pesbt, RESERVED) == 0);
    }
}

static inline void _cc_N(decompress_raw)(_cc_addr_t pesbt, _cc_addr_t cursor, bool tag, _cc_cap_t* cdp) {
    _cc_N(decompress_raw__)(pesbt, cursor, tag, 0, cdp);
}

/*
 * Decompress a 128-bit capability.
 */
static inline void _cc_N(decompress_mem)(uint64_t pesbt, uint64_t cursor, bool tag, _cc_cap_t* cdp) {
    _cc_N(decompress_raw__)(pesbt ^ _CC_N(NULL_XOR_MASK), cursor, tag, 0, cdp);
}

static inline bool _cc_N(is_cap_sealed)(const _cc_cap_t* cp) {
#if _CC_N(FIELD_OTYPE_USED) == 1
    return _cc_N(get_otype)(cp) != _CC_N(OTYPE_UNSEALED);
#else
    return _cc_N(get_ct)(cp);
#endif
}

/// Check that the expanded bounds match the compressed cr_pesbt value.
static inline bool _cc_N(pesbt_is_correct)(const _cc_cap_t* csp) {
    _cc_cap_t tmp;
    // NB: We use the unsafe decompression function here to handle non-derivable caps without asserting.
    _cc_N(unsafe_decompress_raw)(csp->cr_pesbt, csp->_cr_cursor, csp->cr_tag, 0, &tmp);
    tmp.cr_extra = csp->cr_extra; // raw_equal also compares, cr_extra but we don't care about that here.
    if (!_cc_N(raw_equal)(&tmp, csp)) {
        return false;
    }
    return true;
}

// Update ebt bits in pesbt
static inline void _cc_N(update_ebt)(_cc_cap_t* csp, _cc_addr_t new_ebt) {
    csp->cr_pesbt = (csp->cr_pesbt & ~_CC_N(FIELD_EBT_MASK64)) | new_ebt;
    csp->cr_exp = _cc_N(extract_bounds_bits)(new_ebt).E;
}

/*
 * Compress a capability to 128 bits.
 * Note: if you have not been manually modifying fields, just access csp->cr_pesbt.
 * cap_set_decompressed_X will set fields and keep pesbt in sync.
 */
static inline _cc_addr_t _cc_N(compress_raw)(const _cc_cap_t* csp) {
    _cc_debug_assert((!csp->cr_tag || _cc_N(get_reserved)(csp) == 0) &&
                     "Unknown reserved bits set it tagged capability");
    _cc_debug_assert(_cc_N(pesbt_is_correct)(csp) && "capability bounds were modified without updating pesbt");
    return csp->cr_pesbt;
}

static inline _cc_addr_t _cc_N(compress_mem)(const _cc_cap_t* csp) {
    return _cc_N(compress_raw)(csp) ^ _CC_N(NULL_XOR_MASK);
}

static bool _cc_N(fast_is_representable_new_addr)(const _cc_cap_t* cap, _cc_addr_t new_addr);

/// Check that a capability is representable by compressing and recompressing
static inline bool _cc_N(is_representable_cap_exact)(const _cc_cap_t* cap) {
    _cc_addr_t pesbt = _cc_N(compress_raw)(cap);
    _cc_cap_t decompressed_cap;
    // NB: We use the unsafe decompression function here to handle non-derivable caps without asserting.
    _cc_N(unsafe_decompress_raw)(pesbt, cap->_cr_cursor, cap->cr_tag, 0, &decompressed_cap);
    // These fields must not change:
    _cc_debug_assert(decompressed_cap._cr_cursor == cap->_cr_cursor);
    _cc_debug_assert(decompressed_cap.cr_pesbt == cap->cr_pesbt);
    // If any of these fields changed then the capability is not representable:
    if (decompressed_cap.cr_base != cap->cr_base || decompressed_cap._cr_top != cap->_cr_top) {
        return false;
    }
    return true;
}

static inline uint32_t _cc_N(compute_ebt)(_cc_addr_t req_base, _cc_length_t req_top, _cc_addr_t* alignment_mask,
                                          bool* exact) {
#ifdef CC_IS_MORELLO
    if (req_base == 0 && req_top == _CC_N(MAX_TOP)) {
        *exact = true;
        if (alignment_mask)
            *alignment_mask = _CC_MAX_ADDR;
        return _CC_N(RESET_EBT);
    }
#else
    _cc_debug_assert(req_base <= req_top && "Cannot invert base and top");
#endif
    /*
     * With compressed capabilities we may need to increase the range of
     * memory addresses to be wider than requested so it is
     * representable.
     */
    _cc_length_t req_length65 = req_top - req_base;
    // function setCapBounds(cap, base, top) : (Capability, bits(64), bits(65)) -> (bool, Capability) = {
    //  /* {cap with base=base; length=(bits(64)) length; offset=0} */
    //  let base65 = 0b0 @ base;
    //  let length = top - base65;
    //  /* Find an exponent that will put the most significant bit of length
    //     second from the top as assumed during decoding. We ignore the bottom
    //     MW - 1 bits because those are handled by the ie = 0 format. */
    //  let e = 52 - CountLeadingZeros(length[64..13]);
    uint8_t E = (uint8_t)_cc_N(get_exponent)(req_length65);
    const uint64_t req_length64 = (uint64_t)req_length65;
    // Use internal exponent if e is non-zero or if e is zero but
    // but the implied bit of length is not zero (denormal vs. normal case)
    //  let ie = (e != 0) | length[12];
    //
    const bool InternalExponent = E != 0 || _cc_N(getbits)(req_length64, _CC_BOT_INTERNAL_EXP_WIDTH + 1, 1);
    if (!InternalExponent) {
        //  /* The non-ie e == 0 case is easy. It is exact so just extract relevant bits. */
        //  Bbits = truncate(base, 14);
        //  Tbits = truncate(top, 14);
        //  lostSignificantTop  : bool = false;
        //  lostSignificantBase : bool = false;
        //  incE : bool = false;

        /*
         * L8 is bit 8 of the bounds length.
         *
         * For RV32 and exponent E == 0, both B and T are 10 bits wide.
         * A capability stores B[9:0] and T[7:0]. For the bounds length l
         * (the req_length64 variable), we know T = B + l. If l[9] was 1
         * we'd not have exponent 0, so l[9] must be 0.
         *
         * T[9:8] are not stored. Apart from B[9:0] and T[7:0], what else is
         * required to recover T[9:8] from a stored capability?
         *
         *   B   .. .... ....
         * + l   0. #### ####
         *       ------------
         * = T   xx .... ....
         *
         * (. == known bit (stored), # == known bit (not stored),
         * x == unknown bit).
         *
         * T[9:8] = B[9:8] + l[8] + carry bit
         * The carry bit is 1 if B[7:0] > T [7:0]
         *
         * Long story short: If we also store l[8] in the capability, we
         * can recover T[9:8].
         */
        uint8_t l8 = _cc_N(getbits)(req_length64, 8, 1);

        uint32_t ebt_bits = _CC_ENCODE_EBT_FIELD(0, INTERNAL_EXPONENT) | _CC_ENCODE_EBT_FIELD(1, EF) |
                            _CC_ENCODE_EBT_FIELD(l8, L8) |
                            _CC_ENCODE_EBT_FIELD(req_top, EXP_ZERO_TOP) |
                            _CC_ENCODE_EBT_FIELD(req_base, EXP_ZERO_BOTTOM);
#ifdef CC_IS_MORELLO
        // Due to morello conditionally inverting bits, we need to invert the bits that would be an internal exponent
        // here
        ebt_bits ^= _CC_ENCODE_EBT_FIELD(~0, EXPONENT_HIGH_PART) | _CC_ENCODE_EBT_FIELD(~0, EXPONENT_LOW_PART);
#endif
        if (alignment_mask)
            *alignment_mask = _CC_MAX_ADDR; // no adjustment to base required
        *exact = true;
        return ebt_bits; /* Exactly representable */
    }
    // Handle IE case:
    //  if ie then {
    //    /* the internal exponent case is trickier */
    //
    //    /* Extract B and T bits (we lose 3 bits of each to store the exponent) */
    //    B_ie = truncate(base >> (e + 3), 11);
    //    T_ie = truncate(top >> (e + 3), 11);
    //
    _cc_addr_t bot_ie = _cc_N(truncate64)(req_base >> (E + _CC_EXP_LOW_WIDTH), _CC_BOT_INTERNAL_EXP_WIDTH);
    if (alignment_mask) {
        *alignment_mask = UINT64_MAX << (E + _CC_EXP_LOW_WIDTH);
    }
    _cc_addr_t top_ie = _cc_N(truncate64)((_cc_addr_t)(req_top >> (E + _CC_EXP_LOW_WIDTH)), _CC_BOT_INTERNAL_EXP_WIDTH);
    //    /* Find out whether we have lost significant bits of base and top using a
    //       mask of bits that we will lose (including 3 extra for exp). */
    //    maskLo : bits(65) = zero_extend(replicate_bits(0b1, e + 3));
    //    z65    : bits(65) = zeros();
    //    lostSignificantBase = (base65 & maskLo) != z65;
    //    lostSignificantTop = (top & maskLo) != z65;
    // TODO: stop using _cc_length_t and just handle bit65 set specially?
    const _cc_length_t maskLo = (((_cc_length_t)1u) << (E + _CC_EXP_LOW_WIDTH)) - 1;
    const _cc_length_t zero65 = 0;
    bool lostSignificantBase = (req_base & maskLo) != zero65;
    bool lostSignificantTop = (req_top & maskLo) != zero65;
    //    if lostSignificantTop then {
    //      /* we must increment T to make sure it is still above top even with lost bits.
    //         It might wrap around but if that makes B<T then decoding will compensate. */
    //      T_ie = T_ie + 1;
    //    };
    if (lostSignificantTop) {
        top_ie = _cc_N(truncate64)(top_ie + 1, _CC_BOT_INTERNAL_EXP_WIDTH);
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
    const _cc_addr_t len_ie = _cc_N(truncate64)(top_ie - bot_ie, _CC_BOT_INTERNAL_EXP_WIDTH);
    bool incE = false;
    if (_cc_N(getbits)(len_ie, _CC_BOT_INTERNAL_EXP_WIDTH - 1, 1)) {
        incE = true;
        lostSignificantBase = lostSignificantBase || _cc_N(getbits)(bot_ie, 0, 1);
        lostSignificantTop = lostSignificantTop || _cc_N(getbits)(top_ie, 0, 1);
        bot_ie = _cc_N(truncate64)(req_base >> (E + _CC_EXP_LOW_WIDTH + 1), _CC_BOT_INTERNAL_EXP_WIDTH);
        // If we had to adjust bot_ie (shift by one more) also update alignment_mask
        if (alignment_mask) {
            *alignment_mask = UINT64_MAX << (E + _CC_EXP_LOW_WIDTH + 1);
        }
        const bool incT = lostSignificantTop;
        top_ie = _cc_N(truncate64)((_cc_addr_t)(req_top >> (E + _CC_EXP_LOW_WIDTH + 1)), _CC_BOT_INTERNAL_EXP_WIDTH);
        if (incT) {
            top_ie = _cc_N(truncate64)(top_ie + 1, _CC_BOT_INTERNAL_EXP_WIDTH);
        }
    }
    //
    //    Bbits = B_ie @ 0b000;
    //    Tbits = T_ie @ 0b000;
    const _cc_addr_t Bbits = bot_ie << _CC_N(FIELD_EXPONENT_LOW_PART_SIZE);
    const _cc_addr_t Tbits = top_ie << _CC_N(FIELD_EXPONENT_LOW_PART_SIZE);
    const uint8_t newE = E + (incE ? 1 : 0);
#if _CC_N(FIELD_EF_USED) == 1
    /* _cc_N(get_exponent) above returned a valid exponent. If we decided to
     * increment it, we should have checked that it's still valid. */
    _cc_debug_assert(newE <= _CC_N(MAX_EXPONENT));
    const uint8_t newE_coded = _CC_N(MAX_EXPONENT) - newE;
#else
    const uint8_t newE_coded = newE;
#endif
    //  };
    //  let exact = not(lostSignificantBase | lostSignificantTop);
    *exact = !lostSignificantBase && !lostSignificantTop;
    // Split E between L8, T and B
    const uint8_t l8 = newE_coded >> (_CC_N(FIELD_EXPONENT_LOW_PART_SIZE) + _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE));
    const _cc_addr_t expHighBits =
        _cc_N(getbits)(newE_coded >> _CC_N(FIELD_EXPONENT_LOW_PART_SIZE), 0, _CC_N(FIELD_EXPONENT_HIGH_PART_SIZE));
    const _cc_addr_t expLowBits = _cc_N(getbits)(newE_coded, 0, _CC_N(FIELD_EXPONENT_LOW_PART_SIZE));
    const _cc_addr_t Te = Tbits | expHighBits;
    const _cc_addr_t Be = Bbits | expLowBits;
    return _CC_ENCODE_EBT_FIELD(1, INTERNAL_EXPONENT) | _CC_ENCODE_EBT_FIELD(0, EF) |
           _CC_ENCODE_EBT_FIELD(l8, L8) |
           _CC_ENCODE_EBT_FIELD(Te, TOP_ENCODED) | _CC_ENCODE_EBT_FIELD(Be, BOTTOM_ENCODED);
}

static inline bool _cc_N(precise_is_representable_new_addr)(const _cc_cap_t* oldcap, _cc_addr_t new_cursor) {
    // If the decoded bounds are the same with an updated cursor then the capability is representable.
    _cc_cap_t newcap = *oldcap;
    newcap._cr_cursor = new_cursor;
    _cc_bounds_bits old_bounds_bits = _cc_N(extract_bounds_bits)(_cc_N(compress_raw)(oldcap));
    newcap.cr_bounds_valid = _cc_N(compute_base_top)(old_bounds_bits, new_cursor, &newcap.cr_base, &newcap._cr_top);
    return newcap.cr_base == oldcap->cr_base && newcap._cr_top == oldcap->_cr_top && newcap.cr_bounds_valid &&
           oldcap->cr_bounds_valid;
}

static inline bool _cc_N(cap_bounds_uses_value_for_exp)(uint8_t exponent) {
    return exponent < (sizeof(_cc_addr_t) * 8) - _CC_N(FIELD_BOTTOM_ENCODED_SIZE);
}

/// Returns whether the capability bounds depend on any of the cursor bits or if they can be fully derived from E/B/T.
static inline bool _cc_N(cap_bounds_uses_value)(const _cc_cap_t* cap) {
    // This should only be used on decompressed caps, as it relies on the exp field
    _cc_debug_assert(_cc_N(pesbt_is_correct)(cap));
    return _cc_N(cap_bounds_uses_value_for_exp)(cap->cr_exp);
}

static inline bool _cc_N(cap_sign_change)(_cc_addr_t addr1, _cc_addr_t addr2) {
#ifdef CC_IS_MORELLO
    return ((addr1 ^ addr2) & (1ULL << (63 - MORELLO_FLAG_BITS)));
#else
    (void)addr1;
    (void)addr2;
    return false;
#endif
}

static inline bool _cc_N(cap_sign_change_causes_unrepresentability)(const _cc_cap_t* cap, _cc_addr_t addr1,
                                                                    _cc_addr_t addr2) {
    return _cc_N(cap_sign_change)(addr1, addr2) && _cc_N(cap_bounds_uses_value)(cap);
}

static inline bool _cc_N(is_representable_with_addr)(const _cc_cap_t* cap, _cc_addr_t new_addr,
                                                     bool precise_representable_check) {
#ifdef CC_IS_MORELLO
    // If the top bit is changed on morello this can change bounds
    if (__builtin_expect(_cc_N(cap_sign_change_causes_unrepresentability)(cap, new_addr, cap->_cr_cursor), false)) {
        return false;
    }
    // If the exponent is out-of-range (i.e. E>50 && E!=63), all capability address changes will detag.
    // This can happen for underivable capabilities that were created with the settag instruction.
    if (__builtin_expect(!cap->cr_bounds_valid, false)) {
        return false;
    }
#endif
    _cc_addr_t extended_cursor = _cc_N(cap_bounds_address)(new_addr);
    // In-bounds capabilities are always representable.
    if (__builtin_expect(extended_cursor >= cap->cr_base && extended_cursor < cap->_cr_top, true)) {
        return true;
    }
    if (precise_representable_check) {
        return _cc_N(precise_is_representable_new_addr)(cap, new_addr);
    } else {
        return _cc_N(fast_is_representable_new_addr)(cap, new_addr);
    }
}

/// Updates the address of a capability using semantics that match the hardware/format (i.e. using a fast approximate
/// representability check rather than a precise one).
static inline void _cc_N(set_addr)(_cc_cap_t* cap, _cc_addr_t new_addr) {
#if _CC_N(FIELD_EF_USED) == 1
    // The fast representability check is not usable for the risc-v formats.
    bool precise_repr_check = true;
#else
    bool precise_repr_check = false;
#endif
    if (cap->cr_tag && _cc_N(is_cap_sealed)(cap)) {
        cap->cr_tag = false;
    }
    if (!_cc_N(is_representable_with_addr)(cap, new_addr, precise_repr_check)) {
        // Detag and recompute the new bounds if the capability became unrepresentable.
        cap->cr_tag = false;
        _cc_N(decompress_raw)(cap->cr_pesbt, new_addr, false, cap);
    } else {
        cap->_cr_cursor = new_addr;
    }
}

static bool _cc_N(fast_is_representable_new_addr)(const _cc_cap_t* cap, _cc_addr_t new_addr) {
    if (cap->_cr_top == _CC_MAX_TOP && cap->cr_base == 0) {
        return true; // 1 << 65 is always representable
    }

    _cc_bounds_bits bounds = _cc_N(extract_bounds_bits)(cap->cr_pesbt);
    // For Morello this computation uses the sig-extended bounds value.
    _cc_addr_t inc = _cc_N(cap_bounds_address)(new_addr - cap->_cr_cursor);
    _cc_addr_t cursor = _cc_N(cap_bounds_address)(cap->_cr_cursor);

    // i_top uses an arithmetic shift, i_mid and a_mid use logic shifts.
    size_t i_top_shift = bounds.E + _CC_MANTISSA_WIDTH;
    if (i_top_shift >= _CC_ADDR_WIDTH) {
        i_top_shift = _CC_ADDR_WIDTH - 1; // Avoid UBSan errors for shifts > signed bitwidth.
    }
    size_t e_shift = bounds.E >= _CC_ADDR_WIDTH ? _CC_ADDR_WIDTH - 1 : bounds.E;
    _cc_saddr_t i_top = (_cc_saddr_t)inc >> i_top_shift;
    _cc_addr_t i_mid = _cc_N(truncate_addr)((_cc_addr_t)inc >> e_shift, _CC_MANTISSA_WIDTH);
    _cc_addr_t a_mid = _cc_N(truncate_addr)((_cc_addr_t)cursor >> e_shift, _CC_MANTISSA_WIDTH);
    _cc_addr_t B3 = (_cc_addr_t)_cc_truncateLSB(_CC_MANTISSA_WIDTH)(bounds.B, 3);
    _cc_addr_t R3 = _cc_N(truncate_addr)(B3 - 1, 3);
    _cc_addr_t R = _cc_N(truncate_addr)(R3 << (_CC_MANTISSA_WIDTH - 3), _CC_MANTISSA_WIDTH);
    _cc_addr_t diff = _cc_N(truncate_addr)(R - a_mid, _CC_MANTISSA_WIDTH);
    _cc_addr_t diff1 = _cc_N(truncate_addr)(diff - 1, _CC_MANTISSA_WIDTH);
    // i_top determines: (1) whether the increment is inRange i.e. less than the size of the representable region
    // (2**(E+MW)) (2) whether it is positive or negative. To satisfy (1) all top bits must be the same so we are
    // interested in the cases where i_top is 0 or -1.
    bool inLimits;
    if (i_top == 0) {
        inLimits = i_mid < diff1;
    } else if (i_top == (_cc_saddr_t)-1) {
        inLimits = i_mid >= diff && R != a_mid;
    } else {
        inLimits = false;
    }
    return inLimits || bounds.E >= _CC_MAX_EXPONENT - 2;
}

/* @return whether the operation was able to set precise bounds precise or not */
static inline bool _cc_N(setbounds_impl)(_cc_cap_t* cap, _cc_length_t req_len, _cc_addr_t* alignment_mask) {
    uint64_t req_base = cap->_cr_cursor;
    if (_cc_N(is_cap_sealed)(cap)) {
        cap->cr_tag = 0; // Detag sealed inputs to maintain invariants
    }
#ifdef CC_IS_MORELLO
    if (!cap->cr_bounds_valid) {
        cap->cr_tag = 0;
    }
    bool from_large = !_cc_N(cap_bounds_uses_value)(cap);
    if (!from_large) {
        // Mask and sign-extend if any of the address bits are used for bounds.
        // Note: Not doing the masking for large (E>50) capabilities means that some bounds with flag bits set that
        // could be representable end up being detagged, but we have to match the taped-out chip here.
        req_base = _cc_N(cap_bounds_address)(req_base);
    }
#endif
    _cc_length_t req_top = (_cc_length_t)req_base + req_len;
    _cc_debug_assert(req_base <= req_top && "Cannot invert base and top");
    // Clear the tag if the requested base or top are outside the bounds of the input capability.
    if (req_base < cap->cr_base || req_top > cap->_cr_top) {
        cap->cr_tag = 0;
    }
#if _CC_N(FIELD_L8_USED) == 1
    _CC_STATIC_ASSERT(_CC_EXP_LOW_WIDTH == 2, "expected 2 bits to be used by");
    _CC_STATIC_ASSERT(_CC_EXP_HIGH_WIDTH == 2, "expected 2 bits to be used by");
#else
    _CC_STATIC_ASSERT(_CC_EXP_LOW_WIDTH == 3, "expected 3 bits to be used by");
    _CC_STATIC_ASSERT(_CC_EXP_HIGH_WIDTH == 3, "expected 3 bits to be used by");
#endif
    /*
     * With compressed capabilities we may need to increase the range of
     * memory addresses to be wider than requested so it is
     * representable.
     */
    bool exact = false;
    uint32_t new_ebt = _cc_N(compute_ebt)(req_base, req_top, alignment_mask, &exact);
    _cc_addr_t new_base;
    _cc_length_t new_top;
    bool new_bounds_valid = _cc_N(compute_base_top)(_cc_N(extract_bounds_bits)(_CC_ENCODE_FIELD(new_ebt, EBT)),
                                                    cap->_cr_cursor, &new_base, &new_top);
    if (exact) {
#ifndef CC_IS_MORELLO
        // Morello considers a setbounds that takes a capability from "large" (non-sign extended bounds) to "small"
        // to still be exact, even if this results in a change in requested bounds. The exact assert would be tedious
        // to express so I have turned it off for morello.
        _cc_debug_assert(new_base == req_base && "Should be exact");
        _cc_debug_assert(new_top == req_top && "Should be exact");
#endif
    } else {
        _cc_debug_assert((new_base != req_base || new_top != req_top) &&
                         "Was inexact, but neither base nor top different?");
    }
#ifdef CC_IS_MORELLO
    bool to_small = _cc_N(cap_bounds_uses_value_for_exp)(_cc_N(extract_bounds_bits)(new_ebt).E);
    // On morello we may end up with a length that could have been exact, but has changed the flag bits.
    if ((from_large && to_small) && _cc_N(cap_bounds_address)(cap->_cr_cursor) != cap->_cr_cursor) {
        cap->cr_tag = 0;
    }
#endif
    if (cap->cr_tag) {
        // For invalid inputs, new_top could have been larger than max_top and if it is sufficiently larger, it
        // will be truncated to zero, so we can only assert that we get top > base for tagged, valid inputs.
        // See https://github.com/CTSRD-CHERI/sail-cheri-riscv/pull/36 for a decoding change that guarantees
        // this invariant for any input.
        _cc_debug_assert(new_top >= new_base);
        _cc_debug_assert(_cc_N(get_reserved)(cap) == 0 && "Unknown reserved bits set in tagged capability");
        _cc_debug_assert(new_base >= cap->cr_base && "Cannot reduce base on tagged capabilities");
        _cc_debug_assert(new_top <= cap->_cr_top && "Cannot increase top on tagged capabilities");
    }
    cap->cr_base = new_base;
    cap->_cr_top = new_top;
    _cc_N(update_ebt)(cap, new_ebt);
    cap->cr_bounds_valid = new_bounds_valid;
    return exact;
}

/* @return whether the operation was able to set precise bounds precise or not */
static inline bool _cc_N(setbounds)(_cc_cap_t* cap, _cc_length_t req_len) {
    __attribute__((unused)) _cc_addr_t old_base = cap->cr_base;
    __attribute__((unused)) _cc_length_t old_top = cap->_cr_top;
    __attribute__((unused)) _cc_addr_t req_base =
        _cc_N(cap_bounds_uses_value)(cap) ? _cc_N(cap_bounds_address)(cap->_cr_cursor) : cap->_cr_cursor;
    __attribute__((unused)) _cc_length_t req_top = req_len + req_base;
    bool exact = _cc_N(setbounds_impl)(cap, req_len, NULL);
    if (cap->cr_tag) {
        // Assertions to check that we didn't break any invariants.
        _cc_debug_assert(!_cc_N(is_cap_sealed)(cap) && "result cannot be sealed and tagged");
        _cc_debug_assert(((cap->_cr_top - cap->cr_base) >> _CC_ADDR_WIDTH) <= 1 &&
                         "length must be smaller than 1 << 65");
        _cc_debug_assert(cap->cr_base >= old_base && "cannot remain tagged if base was decreased");
        _cc_debug_assert(cap->_cr_top <= old_top && "cannot remain tagged if top was increased");
        // For compatibility with hardware, we keep the tag valid if the input was an underivable tagged capability,
        // but in all other cases it should be impossible to end up with a new top greater than MAX_TOP.
        _cc_debug_assert((cap->_cr_top <= _CC_MAX_TOP || old_top > _CC_MAX_TOP) &&
                         "cannot remain tagged if new top greater 1 << 65");
    }
    // For Morello, we have to compare the sign-extended base and top as the exact check does not cover sign change.
    if (exact) {
        _cc_debug_assert(_cc_N(cap_bounds_address)(cap->cr_base) == _cc_N(cap_bounds_address)(req_base) &&
                         "base changed but still reported exact");
        _cc_debug_assert(_cc_N(cap_bounds_address)(cap->_cr_top) == _cc_N(cap_bounds_address)(req_top) &&
                         "top changed but still reported exact");
    } else {
        _cc_debug_assert((cap->_cr_top != req_top || cap->cr_base != req_base) &&
                         "result is exact but reported inexact");
    }
    return exact;
}

/** Like setbounds, but also asserts that the operation is strictly monotonic. */
static inline bool _cc_N(checked_setbounds)(_cc_cap_t* cap, _cc_length_t req_len) {
    __attribute__((unused)) _cc_addr_t req_base =
        _cc_N(cap_bounds_uses_value)(cap) ? _cc_N(cap_bounds_address)(cap->_cr_cursor) : cap->_cr_cursor;
    __attribute__((unused)) _cc_length_t req_top = req_len + req_base;
    if (cap->cr_tag) {
        // Assertions to detect API misuse - those checks should have been performed before calling setbounds.
        _cc_api_requirement(!_cc_N(is_cap_sealed)(cap), "cannot be used on tagged sealed capabilities");
        _cc_api_requirement(req_base >= cap->cr_base, "cannot decrease base on tagged capabilities");
        _cc_api_requirement(req_top <= cap->_cr_top, "cannot increase top on tagged capabilities");
        _cc_api_requirement(req_len < _CC_MAX_TOP, "requested length must be smaller than max length");
        _cc_api_requirement(req_top < _CC_MAX_TOP, "new top must be smaller than max length");
        _cc_api_requirement(cap->_cr_top <= _CC_MAX_TOP, "input capability top must be less than max top");
    }
    return _cc_N(setbounds)(cap, req_len);
}

// For risc-v cheri formats, the value of M depends on Zcherihybrid support.
// CL field and SL, EL perms depend on lvbits (number of Zcherilevels or 0 if unsupported)
static inline _cc_cap_t _cc_N(make_max_perms_cap_m_lv)(_cc_addr_t base, _cc_addr_t cursor, _cc_length_t top, bool m, uint8_t lvbits) {
    _cc_cap_t creg;
    memset(&creg, 0, sizeof(creg));
    assert(base <= top && "Invalid arguments");
    creg.cr_base = base;
    creg._cr_cursor = cursor;
    creg.cr_bounds_valid = true;
    creg._cr_top = top;
    /* There's no need to exclude unused fields here. */
    creg.cr_pesbt = _CC_ENCODE_FIELD(_CC_N(UPERMS_ALL), UPERMS) | _CC_ENCODE_FIELD(_CC_N(PERMS_ALL), HWPERMS) |
                    _CC_ENCODE_FIELD(_CC_N(FIELD_SDP_MAX_VALUE), SDP) |
                    _CC_ENCODE_FIELD(0, CT) |
                    _CC_ENCODE_FIELD(_CC_N(OTYPE_UNSEALED), OTYPE);
    creg.cr_tag = true;
    creg.cr_exp = _CC_N(RESET_EXP);
    bool exact_input = false;
    _cc_N(update_ebt)(&creg, _cc_N(compute_ebt)(creg.cr_base, creg._cr_top, NULL, &exact_input));
    assert(exact_input && "Invalid arguments");
    assert(_cc_N(is_representable_cap_exact)(&creg));
    creg.cr_m = m ? 1 : 0;
    assert(lvbits <= 1 && "We only support local-global levels.");
    creg.cr_lvbits = lvbits;
    /* We need the most global level here, the level can only be decreased. */
    creg.cr_pesbt |= _CC_ENCODE_FIELD(lvbits, CL);
    creg.cr_arch_perm = CAP_AP_C | CAP_AP_W | CAP_AP_R | CAP_AP_X | CAP_AP_ASR | CAP_AP_LM;
    if (lvbits > 0) {
        creg.cr_arch_perm |= CAP_AP_EL | CAP_AP_SL;
    }
    _cc_N(m_ap_compress)(&creg);
    return creg;
}

static inline _cc_cap_t _cc_N(make_max_perms_cap)(_cc_addr_t base, _cc_addr_t cursor, _cc_length_t top) {
    return _cc_N(make_max_perms_cap_m_lv)(base, cursor, top, false, 0);
}

/* @return the mask that needs to be applied to base in order to get a precisely representable capability */
static inline _cc_addr_t _cc_N(get_alignment_mask)(_cc_addr_t req_length) {
    if (req_length == 0) {
        // With a length of zero we know it is precise so we can just return an
        // all ones mask.
        // This avoids undefined behaviour when counting most significant bit later.
        return _CC_MAX_ADDR;
    }
    // To compute the mask we set bounds on a maximum permissions capability and
    // return the mask that was used to adjust the length
    _cc_cap_t tmpcap = _cc_N(make_max_perms_cap(0, 0, _CC_MAX_TOP));
    _cc_addr_t mask = 0;
    _cc_N(setbounds_impl)(&tmpcap, req_length, &mask);
    return mask;
}

static inline _cc_cap_t _cc_N(make_null_derived_cap)(_cc_addr_t addr) {
    _cc_cap_t creg;
    memset(&creg, 0, sizeof(creg));
    creg._cr_cursor = addr;
    creg._cr_top = _CC_N(MAX_TOP);
    creg.cr_pesbt = _CC_N(NULL_PESBT);
    creg.cr_bounds_valid = 1;
    creg.cr_exp = _CC_N(NULL_EXP);
    _cc_debug_assert(_cc_N(is_representable_cap_exact)(&creg));
    return creg;
}

static inline _cc_addr_t _cc_N(get_required_alignment)(_cc_addr_t req_length) {
    // To get the required alignment from the CRAM mask we can just invert
    // the bits and add one to get a power-of-two
    return ~_cc_N(get_alignment_mask)(req_length) + 1;
}

static inline _cc_addr_t _cc_N(get_representable_length)(_cc_addr_t req_length) {
    _cc_addr_t mask = _cc_N(get_alignment_mask)(req_length);
    return (req_length + ~mask) & mask;
}

/// Provide a C++ class with the same function names
/// to simplify writing code that handles both 128 and 64-bit capabilities
#ifdef __cplusplus
inline bool _cc_N(cap)::operator==(const _cc_N(cap) & other) const { return _cc_N(raw_equal)(this, &other); }

class _CC_CONCAT(CompressedCap, CC_FORMAT_LOWER) {
public:
    using length_t = _cc_length_t;
    using offset_t = _cc_offset_t;
    using addr_t = _cc_addr_t;
    using cap_t = _cc_cap_t;
    using bounds_bits = _cc_bounds_bits;

    static inline addr_t compress_raw(const cap_t& csp) { return _cc_N(compress_raw)(&csp); }
    static inline cap_t decompress_raw(addr_t pesbt, addr_t cursor, bool tag) {
        cap_t result;
        _cc_N(decompress_raw)(pesbt, cursor, tag, &result);
        return result;
    }
    static inline addr_t compress_mem(const cap_t& csp) { return _cc_N(compress_mem)(&csp); }
    static inline cap_t decompress_mem(addr_t pesbt, addr_t cursor, bool tag) {
        cap_t result;
        _cc_N(decompress_mem)(pesbt, cursor, tag, &result);
        return result;
    }
    static inline bounds_bits extract_bounds_bits(addr_t pesbt) { return _cc_N(extract_bounds_bits)(pesbt); }
    static inline bool setbounds(cap_t* cap, length_t req_len) { return _cc_N(setbounds)(cap, req_len); }
    static inline bool is_representable_cap_exact(const cap_t& cap) { return _cc_N(is_representable_cap_exact)(&cap); }
    static inline cap_t make_max_perms_cap(addr_t base, addr_t cursor, length_t top) {
        return _cc_N(make_max_perms_cap)(base, cursor, top);
    }
    static inline cap_t make_null_derived_cap(addr_t addr) { return _cc_N(make_null_derived_cap)(addr); }
    static inline addr_t representable_length(addr_t len) { return _cc_N(get_representable_length)(len); }
    static inline addr_t representable_mask(addr_t len) { return _cc_N(get_alignment_mask)(len); }
    static inline bool fast_is_representable_new_addr(const cap_t& cap, addr_t new_addr) {
        return _cc_N(fast_is_representable_new_addr)(&cap, new_addr);
    }
    static inline bool precise_is_representable_new_addr(const cap_t& cap, addr_t new_addr) {
        return _cc_N(precise_is_representable_new_addr)(&cap, new_addr);
    }
};
#define CompressedCapCC _CC_CONCAT(CompressedCap, CC_FORMAT_LOWER)
#endif
