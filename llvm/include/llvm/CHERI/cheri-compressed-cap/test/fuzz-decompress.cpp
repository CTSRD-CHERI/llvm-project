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
#include <cinttypes>
#include <cstdio>

static void dump_cap_fields(const cc128_cap_t& result) {
    fprintf(stderr, "Permissions: 0x%" PRIx32 "\n", result.permissions()); // TODO: decode perms
    fprintf(stderr, "User Perms:  0x%" PRIx32 "\n", result.software_permissions());
    fprintf(stderr, "Base:        0x%016" PRIx64 "\n", result.base());
    fprintf(stderr, "Offset:      0x%016" PRIx64 "\n", (uint64_t)result.offset());
    fprintf(stderr, "Cursor:      0x%016" PRIx64 "\n", result.address());
    fprintf(stderr, "Length:      0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(result.length() >> 64),
        (uint64_t)result.length() , result.length()  > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
    cc128_length_t top_full = result.top();
    fprintf(stderr, "Top:         0x%" PRIx64 "%016" PRIx64 " %s\n", (uint64_t)(top_full >> 64), (uint64_t)top_full,
        top_full > UINT64_MAX ? " (greater than UINT64_MAX)" : "");
    fprintf(stderr, "Sealed:      %d\n", (int)result.is_sealed());
    fprintf(stderr, "OType:       0x%" PRIx32 "\n", result.type());
    fprintf(stderr, "\n");
}

static inline void check_crrl_and_cram(uint64_t value) {
#ifndef CC_IS_MORELLO
    uint64_t sail_crrl = sail_representable_length_128(value);
    uint64_t clib_crrl = cc128_get_representable_length(value);
    if (sail_crrl != clib_crrl) {
        fprintf(stderr, "CRRL(0x%" PRIx64 ") mismatch: sail=0x%" PRIx64 ", C lib=0x%" PRIx64 "\n", value, sail_crrl,
                clib_crrl);
        abort();
    }
    uint64_t sail_cram = sail_representable_mask_128(value);
    uint64_t clib_cram = cc128_get_alignment_mask(value);
    if (sail_cram != clib_cram) {
        fprintf(stderr, "CRAM(0x%" PRIx64 ") mismatch: sail=0x%" PRIx64 ", C lib=0x%" PRIx64 "\n", value, sail_cram,
                clib_cram);
        abort();
    }
#endif
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size != 16) {
        return 0; // Need 128 bits of data
    }
    FuzzedDataProvider fuzzData(data, size);
    uint64_t pesbt = fuzzData.ConsumeIntegral<uint64_t>();
    uint64_t cursor = fuzzData.ConsumeIntegral<uint64_t>();
    cc128_cap_t result;
    cc128_cap_t sail_result;
    memset(&result, 0, sizeof(result));
    memset(&sail_result, 0, sizeof(sail_result));
    cc128_decompress_mem(pesbt, cursor, false, &result);
    sail_decode_128_mem(pesbt, cursor, false, &sail_result);
    if (memcmp(&result, &sail_result, sizeof(result)) != 0) {
        fprintf(stderr, "DECODE FROM MEM FAILED\n");
        fprintf(stderr, "PESBT  0x%016" PRIx64 "\n", pesbt);
        fprintf(stderr, "Cursor 0x%016" PRIx64 "\n", cursor);
        dump_cap_fields(result);
        dump_cap_fields(sail_result);
        abort();
    }

    check_crrl_and_cram(pesbt);
    check_crrl_and_cram(cursor);

    memset(&result, 0, sizeof(result));
    memset(&sail_result, 0, sizeof(sail_result));
    cc128_decompress_raw(pesbt, cursor, false, &result);
    sail_decode_128_raw(pesbt, cursor, false, &sail_result);
    if (memcmp(&result, &sail_result, sizeof(result)) != 0) {
        fprintf(stderr, "DECODE ALREADY XORED FAILED\n");
        fprintf(stderr, "PESBT  0x%016" PRIx64 "\n", pesbt);
        fprintf(stderr, "Cursor 0x%016" PRIx64 "\n", cursor);
        dump_cap_fields(result);
        dump_cap_fields(sail_result);
        abort();
    }

    return 0;  // Non-zero return values are reserved for future use.
}
