/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Alex Richardson
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
#include "contrib/sail.h"
#include "contrib/sail_failure.h"
#include "sail_wrapper.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsometimes-uninitialized"
#ifdef HAVE_UNUSED_BUT_SET_VARIABLE
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#ifdef HAVE_UNUSED_BUT_SET_VARIABLE
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#endif
#include SAIL_COMPRESSION_GENERATED_C_FILE
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#define CC_FORMAT_LOWER SAIL_WRAPPER_CC_FORMAT_LOWER
#define CC_FORMAT_UPPER SAIL_WRAPPER_CC_FORMAT_UPPER

__attribute__((constructor, used)) static void sail_startup(void) { model_init(); }
__attribute__((destructor, used)) static void sail_cleanup(void) { model_fini(); }

int process_arguments(int argc, char** argv) { abort(); }

void setup_rts(void) {
    // Avoid double initialization of sail.c data
    static int done_setup = false;
    if (!done_setup) {
        setup_library();
        done_setup = true;
    }
}

void cleanup_rts(void) {
    // Avoid double free of sail.c data
    static int done_cleanup = false;
    if (!done_cleanup) {
        cleanup_library();
        done_cleanup = true;
    }
}

static inline uint64_t extract_bits(lbits op, uint64_t start, uint64_t len) {
    sail_int start_sail;
    CREATE_OF(sail_int, mach_int)(&start_sail, start);
    sail_int len_sail;
    CREATE_OF(sail_int, mach_int)(&len_sail, len);
    lbits slice_bits;
    CREATE(lbits)(&slice_bits);
    slice(&slice_bits, op, start_sail, len_sail);
    KILL(sail_int)(&start_sail);
    KILL(sail_int)(&len_sail);
    uint64_t result = CONVERT_OF(fbits, lbits)(slice_bits, true);
    KILL(lbits)(&slice_bits);
    return result;
}

#ifdef SAIL_WRAPPER_CC_IS_MORELLO

static inline uint64_t extract_low_bits(lbits bits) {
    fbits bits_low = CONVERT_OF(fbits, lbits)(bits, true);
    return (uint64_t)bits_low;
}

#define CHECK_FIELD_LENGTHS

static inline void check_length(lbits bits, uint64_t length) {
#ifdef CHECK_FIELD_LENGTHS
    sail_int len;
    CREATE(sail_int)(&len);
    length_lbits(&len, bits);
    assert(CONVERT_OF(mach_int, sail_int)(len) == length);
    KILL(sail_int)(&len);
#endif
}

// The Morello cap is just all the bits slammed together with no extra decode
static lbits to_sail_cap(uint64_t mem_pesbt, uint64_t mem_cursor, bool tag) {
    lbits cap_without_tag;
    pesbt_and_addr_to_sail_cap_bits(&cap_without_tag, mem_pesbt, mem_cursor);
    lbits capbits;
    CREATE(lbits)(&capbits);
    lbits captag;
    CREATE_OF(lbits, fbits)(&captag, tag ? 1 : 0, 1, true);
    append(&capbits, captag, cap_without_tag);
    KILL(lbits)(&captag);
    KILL(lbits)(&cap_without_tag);

    check_length(capbits, 129);
    return capbits;
}

static lbits cap_t_to_sail_cap(const _cc_cap_t* c) {
    return to_sail_cap(c->cr_pesbt ^ _CC_N(NULL_XOR_MASK), c->_cr_cursor, c->cr_tag);
}

/* Exported API */
static _cc_cap_t from_sail_cap(const lbits* sail_cap) {
    _cc_cap_t result;
    memset(&result, 0, sizeof(result));

    // Now extract some fields
    // Bounds (base and top) and exponent
    struct sail_bounds_tuple bounds;
    _CC_CONCAT(create_, sail_bounds_tuple)(&bounds);
    sailgen_CapGetBounds(&bounds, *sail_cap);

    check_length(bounds.ztup0, 65);
    check_length(bounds.ztup1, 65);

    result.cr_base = extract_bits(bounds.ztup0, 0, 64);
    uint64_t top_hi = extract_bits(bounds.ztup1, 64, 1);
    uint64_t top_lo = extract_low_bits(bounds.ztup1);
    result._cr_top = (((_cc_length_t)top_hi) << 64) | (_cc_length_t)top_lo;
    _CC_CONCAT(kill_, sail_bounds_tuple)(&bounds);

    result.cr_bounds_valid = bounds.ztup2;
    result.cr_exp = sailgen_CapGetExponent(*sail_cap);

    // Address including flag bits
    result._cr_cursor = sailgen_CapGetValue(*sail_cap);
    _cc_N(update_perms)(&result, sailgen_CapGetPermissions(*sail_cap));
    _cc_N(update_otype)(&result, sailgen_CapGetObjectType(*sail_cap));
    result.cr_tag = sailgen_CapGetTag(*sail_cap);

    // Fix these extra fields not really present in sail
    _cc_N(update_reserved)(&result, 0);
    _cc_N(update_flags)(&result, 0);
    _cc_N(update_uperms)(&result, 0);
    // Morello sail does not include the XOR, so we have to apply it here to match the C compression library.
    result.cr_pesbt = extract_bits(*sail_cap, 64, 64) ^ _CC_N(NULL_XOR_MASK);
    return result;
}

_cc_cap_t _cc_sail_decode_mem(uint64_t mem_pesbt, uint64_t mem_cursor, bool tag) {
    lbits capbits = to_sail_cap(mem_pesbt, mem_cursor, tag);
    _cc_cap_t result = from_sail_cap(&capbits);
    KILL(lbits)(&capbits);
    return result;
}

_cc_cap_t _cc_sail_decode_raw(uint64_t mem_pesbt, uint64_t mem_cursor, bool tag) {
    // Morello RAW has no mask. If this has been masked, undo it.
    return _cc_sail_decode_mem(mem_pesbt ^ _CC_N(NULL_XOR_MASK), mem_cursor, tag);
}

uint64_t sail_compress_common_mem(const _cc_cap_t* csp) {
    lbits capbits = cap_t_to_sail_cap(csp);
    // The sail representation uses the memory format internally.
    uint64_t mem_pesbt = extract_bits(capbits, 64, 64);
    KILL(lbits)(&capbits);
    return mem_pesbt;
}

uint64_t sail_compress_common_raw(const _cc_cap_t* csp) {
    // Morello sail does not include the XOR, so we have to apply it here to match the C compression library.
    return sail_compress_common_mem(csp) ^ _CC_N(NULL_XOR_MASK);
}

#else

static inline void sail_dump_cap(const char* msg, struct zCapability cap) {
    sail_string zcap_str;
    CREATE(sail_string)(&zcap_str);
    sailgen_capToString(&zcap_str, cap);
    fprintf(stderr, "%s: %s\n", msg, zcap_str);
    KILL(sail_string)(&zcap_str);
}

static inline void set_top_base_from_sail(const struct zCapability* sail, _cc_cap_t* c);
static inline uint64_t extract_sail_cap_bits(sail_cap_bits* bits, uint64_t start, uint64_t len);

static _cc_addr_t _compress_sailcap_raw(struct zCapability sailcap) {
    sail_cap_bits sailbits;
    CREATE(sail_cap_bits)(&sailbits);
#ifdef SAIL_COMPRESSION_INDIRECT_BITS
    sailgen_capToBits(&sailbits, sailcap);
#else
    sailbits = sailgen_capToBits(sailcap);
#endif
    _cc_addr_t result = extract_sail_cap_bits(&sailbits, _CC_ADDR_WIDTH, _CC_ADDR_WIDTH);
    KILL(sail_cap_bits)(&sailbits);
    return result;
}

static _cc_cap_t sail_cap_to_cap_t(const struct zCapability* sail) {
    _cc_cap_t c;
    memset(&c, 0, sizeof(c));
    c._cr_cursor = sail->zaddress;
    set_top_base_from_sail(sail, &c);
    _cc_N(update_perms)(&c, sailgen_getCapHardPerms(*sail));
    _cc_N(update_uperms)(&c, sail->zuperms);
    _cc_N(update_otype)(&c, sail->zotype);
    _cc_N(update_flags)(&c, sailgen_getCapFlags(*sail));
    _cc_N(update_reserved)(&c, sail->zreserved);
    c.cr_tag = sail->ztag;
    c.cr_exp = sail->zE;
    c.cr_bounds_valid = true; // This field is only ever false for Morello.
    // extract cc128 EBT field:
    // TODO: avoid roundtrip via sailgen_capToBits?
    uint64_t sail_pesbt = _compress_sailcap_raw(*sail);
    c.cr_pesbt = sail_pesbt;
    return c;
}

_cc_cap_t _cc_sail_decode_mem(_cc_addr_t mem_pesbt, _cc_addr_t mem_cursor, bool tag) {
    sail_cap_bits sail_all_bits;
    pesbt_and_addr_to_sail_cap_bits(&sail_all_bits, mem_pesbt, mem_cursor);
    struct zCapability sail_result = sailgen_memBitsToCapability(tag, sail_all_bits);
    KILL(sail_cap_bits)(&sail_all_bits);
    // sail_dump_cap("sail_result", sail_result);
    return sail_cap_to_cap_t(&sail_result);
}

static struct zCapability _sail_decode_common_raw_impl(_cc_addr_t pesbt, _cc_addr_t cursor, bool tag) {
    sail_cap_bits sail_all_bits;
    pesbt_and_addr_to_sail_cap_bits(&sail_all_bits, pesbt, cursor);
    struct zCapability sail_result = sailgen_capBitsToCapability(tag, sail_all_bits);
    KILL(sail_cap_bits)(&sail_all_bits);
    return sail_result;
}

_cc_cap_t _cc_sail_decode_raw(_cc_addr_t pesbt, _cc_addr_t cursor, bool tag) {
    struct zCapability sail_result = _sail_decode_common_raw_impl(pesbt, cursor, tag);
    return sail_cap_to_cap_t(&sail_result);
}

static _cc_bounds_bits sail_extract_bounds_bits_common(_cc_addr_t pesbt) {
    _cc_bounds_bits result;
    struct zCapability sail_result = _sail_decode_common_raw_impl(pesbt, 0, false);
    result.E = sail_result.zE;
    result.B = sail_result.zB;
    result.T = sail_result.zT;
    result.IE = sail_result.zinternal_E;
    return result;
}

static struct zCapability cap_t_to_sail_cap(const _cc_cap_t* c) {
    struct zCapability result;
    result = znull_cap;
    result = sailgen_setCapPerms(result, _cc_N(get_perms)(c));
    result.ztag = c->cr_tag;
    result.zreserved = _cc_N(get_reserved)(c);
    result.zaddress = c->_cr_cursor;
    result.zuperms = _cc_N(get_uperms)(c);
    result.zotype = _cc_N(get_otype)(c);
    result.zflag_cap_mode = _cc_N(get_flags)(c);

    // Extract E,B,T,IE from the cr_pesbt field:
    _cc_addr_t fake_pesbt = c->cr_pesbt;
    _cc_bounds_bits c_bounds = _cc_N(extract_bounds_bits)(fake_pesbt);
    result.zinternal_E = c_bounds.IE;
    result.zE = c_bounds.E;
    result.zT = c_bounds.T;
    result.zB = c_bounds.B;
    return result;
}

static _cc_addr_t sail_compress_common_raw(const _cc_cap_t* csp) {
    return _compress_sailcap_raw(cap_t_to_sail_cap(csp));
}

static _cc_addr_t sail_compress_common_mem(const _cc_cap_t* csp) {
    struct zCapability sailcap = cap_t_to_sail_cap(csp);
    sail_cap_bits sailbits;
    CREATE(sail_cap_bits)(&sailbits);
#ifdef SAIL_COMPRESSION_INDIRECT_BITS
    sailgen_capToMemBits(&sailbits, sailcap);
#else
    sailbits = sailgen_capToMemBits(sailcap);
#endif
    _cc_addr_t result = extract_sail_cap_bits(&sailbits, _CC_ADDR_WIDTH, _CC_ADDR_WIDTH);
    KILL(sail_cap_bits)(&sailbits);
    return result;
}

bool _cc_sail(fast_is_representable)(const _cc_cap_t* cap, _cc_addr_t new_addr) {
    struct zCapability sailcap = cap_t_to_sail_cap(cap);
    uint64_t increment = new_addr - cap->_cr_cursor;
    return sailgen_fastRepCheck(sailcap, increment);
}

bool _cc_sail(precise_is_representable)(const _cc_cap_t* cap, _cc_addr_t new_addr) {
    struct zCapability sailcap = cap_t_to_sail_cap(cap);
    return sailgen_setCapAddr(sailcap, new_addr).ztup0;
}

_cc_addr_t _CC_CONCAT(sail_null_pesbt_, SAIL_WRAPPER_CC_FORMAT_LOWER)(void) {
    // NULL CAP BITS:
    _cc_addr_t null_pesbt = _compress_sailcap_raw(znull_cap);
    sail_dump_cap("null cap", znull_cap);
    fprintf(stderr, "Sail%d null pesbt 0x%jx\n", SAIL_WRAPPER_CC_FORMAT_LOWER, (uintmax_t)null_pesbt);
    return null_pesbt;
}

bool _CC_CONCAT(sail_setbounds_, SAIL_WRAPPER_CC_FORMAT_LOWER)(_cc_cap_t* cap, _cc_length_t req_len) {
    struct zCapability sailcap = cap_t_to_sail_cap(cap);
    sail_cap_bits sail_req_len;
    CREATE(sail_cap_bits)(&sail_req_len);
    cc_length_t_to_sail_cap_bits(&sail_req_len, req_len);
    __typeof__(sailgen_doCSetBounds(sailcap, sail_req_len)) result = sailgen_doCSetBounds(sailcap, sail_req_len);
    KILL(sail_cap_bits)(&sail_req_len);
    bool exact = result.ztup0;
    *cap = sail_cap_to_cap_t(&result.ztup1);
    return exact;
}

_cc_cap_t _CC_CONCAT(sail_reset_capability_, SAIL_WRAPPER_CC_FORMAT_LOWER)(void) {
    return sail_cap_to_cap_t(&zdefault_cap);
}

#endif // SAIL_WRAPPER_CC_IS_MORELLO
