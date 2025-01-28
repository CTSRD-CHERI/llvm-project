/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 Codasip
 *
 * based on cheri_compressed_cap_128.h, which is
 * Copyright (c) 2018-2020 Alex Richardson
 */

#define CC_FORMAT_LOWER 128r
#define CC_FORMAT_UPPER 128R

#define CC128R_CAP_SIZE    16
#define CC128R_CAP_BITS   128
#define CC128R_ADDR_WIDTH  64
#define CC128R_LEN_WIDTH   65

/* Max exponent is the largest exponent _required_, not that can be encoded. */
#define CC128R_MANTISSA_WIDTH 14
#define CC128R_MAX_EXPONENT 52
#define CC128R_CURSOR_MASK 0xFFFFFFFFFFFFFFFF

#define CC128R_MAX_ADDRESS_PLUS_ONE ((cc128r_length_t)1u << CC128R_ADDR_WIDTH)
#define CC128R_NULL_TOP    CC128R_MAX_ADDRESS_PLUS_ONE
#define CC128R_NULL_LENGTH CC128R_MAX_ADDRESS_PLUS_ONE
#define CC128R_MAX_LENGTH  CC128R_MAX_ADDRESS_PLUS_ONE
#define CC128R_MAX_TOP     CC128R_MAX_ADDRESS_PLUS_ONE
#define CC128R_MAX_ADDR    UINT64_MAX

/* Special otypes are allocated downwards from -1 */
#define CC128R_SPECIAL_OTYPE_VAL(subtract) \
    (CC128R_MAX_REPRESENTABLE_OTYPE - subtract##u)
#define CC128R_SPECIAL_OTYPE_VAL_SIGNED(subtract) (((int64_t)-1) - subtract##u)

/* Use __uint128 to represent 65 bit length */
__extension__ typedef unsigned __int128 cc128r_length_t;
__extension__ typedef signed __int128 cc128r_offset_t;
typedef uint64_t cc128r_addr_t;
typedef int64_t cc128r_saddr_t;
#include "cheri_compressed_cap_macros.h"

/* ignore ISO C restricts enumerator values to range of 'int' */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
enum {
    _CC_FIELD(RESERVED2, 127, 121),
    _CC_FIELD(SDP, 120, 117),
    _CC_FIELD(M, 116, 116),
    _CC_FIELD(AP, 115, 108),
    _CC_FIELD(CL, 107, 107),
    _CC_FIELD(RESERVED, 106, 92),
    _CC_FIELD(CT, 91, 91),
    _CC_FIELD(EBT, 90, 64),

    _CC_FIELD(EF, 90, 90),
    _CC_FIELD(TOP_ENCODED, 89, 78),
    _CC_FIELD(BOTTOM_ENCODED, 77, 64),

    // Top/bottom offsets depending on INTERNAL_EXPONENT flag:
    // Without internal exponent:
    _CC_FIELD(EXP_ZERO_TOP, 89, 78),
    _CC_FIELD(EXP_ZERO_BOTTOM, 77, 64),
    // With internal exponent:
    _CC_FIELD(EXP_NONZERO_TOP, 89, 81),
    _CC_FIELD(EXPONENT_HIGH_PART, 80, 78),
    _CC_FIELD(EXP_NONZERO_BOTTOM, 77, 67),
    _CC_FIELD(EXPONENT_LOW_PART, 66, 64),

    /* The following fields are unused for the 128r format. */
    _CC_FIELD(INTERNAL_EXPONENT, 81, 82),
    _CC_FIELD(FLAGS, 81, 82),
    _CC_FIELD(OTYPE, 81, 82),
    _CC_FIELD(UPERMS, 81, 82),
    _CC_FIELD(HWPERMS, 81, 82),
    _CC_FIELD(L8, 81, 82),
};
#pragma GCC diagnostic pop

#define CC128R_FIELD_M_USED 1
#define CC128R_FIELD_FLAGS_USED 0
#define CC128R_FIELD_OTYPE_USED 0
#define CC128R_FIELD_HWPERMS_USED 0
#define CC128R_FIELD_UPERMS_USED 0
#define CC128R_FIELD_EF_USED 1
#define CC128R_FIELD_L8_USED 0
#define CC128R_FIELD_CL_USED 1

#define CC128R_OTYPE_BITS CC128R_FIELD_OTYPE_SIZE
#define CC128R_BOT_WIDTH CC128R_FIELD_EXP_ZERO_BOTTOM_SIZE
#define CC128R_BOT_INTERNAL_EXP_WIDTH CC128R_FIELD_EXP_NONZERO_BOTTOM_SIZE
#define CC128R_EXP_LOW_WIDTH CC128R_FIELD_EXPONENT_LOW_PART_SIZE

#define CC128R_PERM_GLOBAL (1 << 0)
#define CC128R_PERM_EXECUTE (1 << 1)
#define CC128R_PERM_LOAD (1 << 2)
#define CC128R_PERM_STORE (1 << 3)
#define CC128R_PERM_LOAD_CAP (1 << 4)
#define CC128R_PERM_STORE_CAP (1 << 5)
#define CC128R_PERM_STORE_LOCAL (1 << 6)
#define CC128R_PERM_SEAL (1 << 7)
#define CC128R_PERM_CINVOKE (1 << 8)
#define CC128R_PERM_UNSEAL (1 << 9)
#define CC128R_PERM_ACCESS_SYS_REGS (1 << 10)
#define CC128R_PERM_SETCID (1 << 11)

#define CC128R_PERMS_ALL       (0)
#define CC128R_UPERMS_ALL      (0)
#define CC128R_UPERMS_SHFT     (0)
#define CC128R_UPERMS_MEM_SHFT (0)
#define CC128R_MAX_UPERM       (0)

/* Unused for 128r, but referenced by cheri_compressed_cap_common.h, so they
   have to be defined. */
enum _CC_N(OTypes) {
    CC128R_FIRST_NONRESERVED_OTYPE = 0,
    /*
     * When otype is used for a format, we can calculate the maximum
     * representable otype from the size of the otype field. We use a dummy
     * constant here to satisfy the min < max assert in
     * cheri_compressed_cap_common.h.
     */
    CC128R_MAX_REPRESENTABLE_OTYPE = 16,
    _CC_SPECIAL_OTYPE(OTYPE_UNSEALED, 0),
    _CC_SPECIAL_OTYPE(OTYPE_SENTRY, 1),
    _CC_SPECIAL_OTYPE(OTYPE_INDIRECT_PAIR, 2),
    _CC_SPECIAL_OTYPE(OTYPE_INDIRECT_SENTRY, 3),
    _CC_SPECIAL_OTYPE(OTYPE_RESERVED_LAST, 3),
    /*
     * We allocate otypes subtracting from the maximum value, so the smallest is
     * actually the one with the largest _CC_SPECIAL_OTYPE() argument.
     * With 4 reserved otypes, this is currently -4, i.e. 0x3fffc.
     */
    _CC_N(MIN_RESERVED_OTYPE) = _CC_N(OTYPE_RESERVED_LAST),
    _CC_N(MAX_RESERVED_OTYPE) = _CC_N(OTYPE_UNSEALED),
};

#define CC128R_LS_SPECIAL_OTYPES(ITEM, ...) \
    ITEM(OTYPE_UNSEALED, __VA_ARGS__) \
    ITEM(OTYPE_SENTRY, __VA_ARGS__) \
    ITEM(OTYPE_INDIRECT_PAIR, __VA_ARGS__) \
    ITEM(OTYPE_INDIRECT_SENTRY, __VA_ARGS__)

_CC_STATIC_ASSERT_SAME(CC128R_MANTISSA_WIDTH, CC128R_FIELD_EXP_ZERO_BOTTOM_SIZE);

#define CC128R_M_AP_FCTS M_AP_FCTS_IDENT

#include "cheri_compressed_cap_common.h"

/* Sanity-check mask is the expected NULL encoding */
_CC_STATIC_ASSERT_SAME(CC128R_NULL_XOR_MASK, UINT64_C(0x0));

#define CC128R_FIELD(name, last, start)      _CC_FIELD(name, last, start)
#define CC128R_ENCODE_FIELD(value, name)     _CC_ENCODE_FIELD(value, name)
#define CC128R_EXTRACT_FIELD(value, name)    _CC_EXTRACT_FIELD(value, name)
#define CC128R_ENCODE_EBT_FIELD(value, name) _CC_ENCODE_EBT_FIELD(value, name)

#undef CC_FORMAT_LOWER
#undef CC_FORMAT_UPPER
