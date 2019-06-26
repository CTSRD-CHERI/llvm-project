/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
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
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "cheri_compressed_cap.h"

static const char* otype_suffix(uint32_t otype) {
    switch(otype) {
    case CC128_OTYPE_UNSEALED: return " (CC128_OTYPE_UNSEALED)";
    case CC128_OTYPE_SENTRY: return " (CC128_OTYPE_SENTRY)";
    case CC128_OTYPE_RESERVED2: return " (CC128_OTYPE_RESERVED2)";
    case CC128_OTYPE_RESERVED3: return " (CC128_OTYPE_RESERVED3)";
    default: break;
    }
    switch(otype) {
    case CC256_OTYPE_UNSEALED: return " (CC256_OTYPE_UNSEALED)";
    case CC256_OTYPE_SENTRY: return " (CC256_OTYPE_SENTRY)";
    case CC256_OTYPE_RESERVED2: return " (CC256_OTYPE_RESERVED2)";
    case CC256_OTYPE_RESERVED3: return " (CC256_OTYPE_RESERVED3)";
    default: return "";
    }
}

static void dump_cap_fields(const cap_register_t* result) {
    fprintf(stderr, "Permissions: 0x%" PRIx32 "\n", result->cr_perms); // TODO: decode perms
    fprintf(stderr, "User Perms:  0x%" PRIx32 "\n", result->cr_uperms);
    fprintf(stderr, "Base:        0x%016" PRIx64 "\n", result->cr_base);
    fprintf(stderr, "Offset:      0x%016" PRIx64 "\n", result->cr_offset);
    fprintf(stderr, "Length:      0x%" PRIx64 "%016" PRIx64 " %s\n",
            (uint64_t)(result->_cr_length >> 64), (uint64_t)result->_cr_length,
            result->_cr_length > UINT64_MAX ? " (greater than UINT64_MAX)": "");
    unsigned __int128 top_full = result->cr_base + result->_cr_length;
    fprintf(stderr, "Top:         0x%" PRIx64 "%016" PRIx64 " %s\n",
            (uint64_t)(top_full >> 64), (uint64_t)top_full,
            top_full > UINT64_MAX ? " (greater than UINT64_MAX)": "");
    fprintf(stderr, "Sealed:      %d\n", cc128_is_cap_sealed(result) ? 1 : 0);
    fprintf(stderr, "OType:       0x%" PRIx32 "%s\n", result->cr_otype, otype_suffix(result->cr_otype));
    fprintf(stderr, "\n");
}

int main(int argc, char** argv) {
    fprintf(stderr, "CC128_NULL_XOR_MASK=0x%llx\n", (long long)CC128_NULL_XOR_MASK);
    fprintf(stderr, "CC128_NULL_PESBT   =0x%llx\n", (long long)CC128_NULL_PESBT);
    if (argc < 3) {
        fprintf(stderr, "Usage: %s PESBT CURSOR\n", argv[0]);
        return EXIT_FAILURE;
    }
    errno = 0;
    char* end;
    uint64_t pesbt = strtoull(argv[1], &end, 16);
    if (errno != 0 || !end || *end != '\0') {
        err(EX_DATAERR, "pesbt not a valid hex number: %s", argv[1]);
    }
    uint64_t cursor = strtoull(argv[2], &end, 16);
    if (errno != 0 || !end || *end != '\0') {
        err(EX_DATAERR, "cursor not a valid hex number: %s", argv[2]);
    }
    cap_register_t result;
    memset(&result, 0, sizeof(result));
    printf("Decompressing pesbt = %016" PRIx64 ", cursor = %016" PRIx64 "\n", pesbt, cursor);
    decompress_128cap(pesbt, cursor, &result);
    dump_cap_fields(&result);
    return EXIT_SUCCESS;
}
