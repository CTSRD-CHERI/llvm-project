/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Lawrence Esswood
 * Copyright (c) 2018 Alex Richardson
 * All rights reserved.
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

#include "../cheri_compressed_cap.h"
#include "FuzzedDataProvider.h"
#include "sail_wrapper.h"
#include "test_util.h"
#include <cinttypes>
#include <cstdio>

#define CC_FORMAT_LOWER TEST_CC_FORMAT_LOWER
#define CC_FORMAT_UPPER TEST_CC_FORMAT_UPPER

#define DO_STRINGIFY2(x) #x
#define DO_STRINGIFY1(x) DO_STRINGIFY2(x)
#define STRINGIFY(x) DO_STRINGIFY1(x)

static void dump_cap_fields(const _cc_cap_t& result) { dump_cap_fields(stderr, result); }

static bool compare_caps(const char* context, const _cc_cap_t& result, const _cc_cap_t& sail_result) {
    if (memcmp(&result, &sail_result, sizeof(result)) == 0)
        return true;
    fprintf(stderr, "%s FAILED\n", context);
    dump_cap_fields(result);
    dump_cap_fields(sail_result);
    return false;
}

// TODO: Implement for Morello
#ifndef TEST_CC_IS_MORELLO
static inline void check_crrl_and_cram(_cc_addr_t value) {
    _cc_addr_t sail_crrl = _cc_sail(representable_length)(value);
    _cc_addr_t clib_crrl = _cc_N(get_representable_length)(value);
    if (sail_crrl != clib_crrl) {
        fprintf(stderr, "CRRL(0x%" PRIx64 ") mismatch: sail=0x%" PRIx64 ", C lib=0x%" PRIx64 "\n", (uint64_t)value,
                (uint64_t)sail_crrl, (uint64_t)clib_crrl);
        abort();
    }
    _cc_addr_t sail_cram = _cc_sail(representable_mask)(value);
    _cc_addr_t clib_cram = _cc_N(get_alignment_mask)(value);
    if (sail_cram != clib_cram) {
        fprintf(stderr, "CRAM(0x%" PRIx64 ") mismatch: sail=0x%" PRIx64 ", C lib=0x%" PRIx64 "\n", (uint64_t)value,
                (uint64_t)sail_cram, (uint64_t)clib_cram);
        abort();
    }
}
#endif

void fuzz_setbounds(const _cc_cap_t& input_cap, _cc_addr_t req_len) {
    _cc_cap_t new_result = input_cap;
    _cc_cap_t new_sail_result = new_result;
    _cc_sail(setbounds)(&new_sail_result, req_len);
    _cc_N(setbounds)(&new_result, req_len);
    if (!compare_caps("SETBOUNDS", new_result, new_sail_result)) {
        fprintf(stderr, "with addr=%" PRIx64 " and req_len=%" PRIx64 "\n", (uint64_t)input_cap.address(),
                (uint64_t)req_len);
        dump_cap_fields(input_cap);
        abort();
    }
}

void fuzz_representable(const _cc_cap_t& input_cap, _cc_addr_t new_addr) {
    bool cc_fast_rep = TestAPICC::fast_is_representable_new_addr(input_cap, new_addr);
    bool sail_fast_rep = TestAPICC::sail_fast_is_representable(input_cap, new_addr);
    if (cc_fast_rep != sail_fast_rep) {
        fprintf(stderr, "Fast rep check differs for sail (%d) vs cclib (%d) for addr %#016" PRIx64 " \nInput was:\n",
                sail_fast_rep, cc_fast_rep, (uint64_t)new_addr);
        dump_cap_fields(input_cap);
        abort();
    }
    bool cc_full_rep = TestAPICC::precise_is_representable_new_addr(input_cap, new_addr);
    bool sail_full_rep = TestAPICC::sail_precise_is_representable(input_cap, new_addr);
    if (cc_full_rep != sail_full_rep) {
        fprintf(stderr, "Precise rep check differs for sail (%d) vs cclib (%d) for addr %#016" PRIx64 " \nInput was:\n",
                sail_full_rep, cc_full_rep, (uint64_t)new_addr);
        dump_cap_fields(input_cap);
        abort();
    }
}

/// Create a tagged version of the input capability (if there are no reserved bits set)
/// NB: This may not be possible to create at runtime (unless you have SetTag), but our library should handle them
/// correctly by detagging.
_cc_cap_t make_tagged_cap(const _cc_cap_t& c) {
    _cc_cap_t result = c;
    result.cr_tag = c.reserved_bits() == 0;
    return result;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 2 * sizeof(_cc_addr_t) || size > 4 * sizeof(_cc_addr_t)) {
        return 0; // Need two to four words of data
    }
    FuzzedDataProvider fuzzData(data, size);
    _cc_addr_t pesbt = fuzzData.ConsumeIntegral<_cc_addr_t>();
    _cc_addr_t cursor = fuzzData.ConsumeIntegral<_cc_addr_t>();
    _cc_addr_t new_len = fuzzData.ConsumeIntegral<_cc_addr_t>();
    _cc_addr_t random_base = fuzzData.ConsumeIntegral<_cc_addr_t>();

    _cc_cap_t result;
    memset(&result, 0, sizeof(result));
    _cc_N(decompress_mem)(pesbt, cursor, false, &result);
    _cc_cap_t sail_result = TestAPICC::sail_decode_mem(pesbt, cursor, false);
    if (!compare_caps("DECODE FROM MEM", result, sail_result)) {
        abort();
    }
    const _cc_cap_t tagged_result = make_tagged_cap(result);

#ifndef TEST_CC_IS_MORELLO
    check_crrl_and_cram(pesbt);
    check_crrl_and_cram(cursor);
#endif

    memset(&result, 0, sizeof(result));
    _cc_N(decompress_raw)(pesbt, cursor, false, &result);
    sail_result = _cc_sail_decode_raw(pesbt, cursor, false);
    if (!compare_caps("DECODE ALREADY XORED", result, sail_result)) {
        abort();
    }

    // Compare the fast and full representability check to sail.
    fuzz_representable(result, random_base);
    fuzz_representable(tagged_result, random_base);

    // Try running setbounds and compare to sail.
    fuzz_setbounds(result, new_len);
    fuzz_setbounds(tagged_result, new_len);
    {
        _cc_cap_t tmp = result;
        _cc_N(set_addr)(&tmp, result.base());
        fuzz_setbounds(tmp, new_len);
        _cc_cap_t tmp_tagged = make_tagged_cap(tmp);
        fuzz_setbounds(tmp_tagged, new_len);

        _cc_N(set_addr)(&tmp, random_base);
        fuzz_setbounds(tmp, new_len);
        tmp_tagged = make_tagged_cap(tmp);
        fuzz_setbounds(tmp_tagged, new_len);
    }

    return 0; // Non-zero return values are reserved for future use.
}
