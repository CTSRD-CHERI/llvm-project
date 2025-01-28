
#define TEST_CC_FORMAT_LOWER 128r
#define TEST_CC_FORMAT_UPPER 128R

#include "test_common.cpp"
#include "cap_m_ap.h"

TEST_CASE("update ct", "[ct]") {
    _cc_cap_t cap;

    _cc_N(update_ct)(&cap, 1);
    CHECK_FIELD(cap, is_sealed, 1);
}

/*
 * For 64-bit risc-v cheri, the conversion between M + AP bitfield and its
 * encoding is a trivial 1:1 mapping. Some spot checks will do, there's
 * no point in checking all combinations.
 */

/* M, AP compression */

TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_R, CAP_AP_R)

TEST_CASE_M_AP_COMP(LVB_0, 
        1, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_ASR),
        1 << _CC_N(FIELD_AP_SIZE) | (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_ASR))

/* unused SL and EL permissions must be removed */
TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_SL | CAP_AP_EL,
        CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM)

/* M, AP decompression */

TEST_CASE_M_AP_DECOMP(LVB_0,
        1 << _CC_N(FIELD_AP_SIZE) | (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_ASR),
        1, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_ASR))

TEST_CASE_M_AP_DECOMP(LVB_0,
        (CAP_AP_X | CAP_AP_R | CAP_AP_W),
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W));

TEST_CASE("bounds encoding exponent 0", "[bounds]") {
    /* params are base, cursor, top */
    _cc_cap_t cap = CompressedCap128r::make_max_perms_cap(0x0, 0x10, 0x20);

    /*
     * SDP = 0b1111
     * M = 0b0
     * EF == 1 -> exponent 0
     * { T, TE } == 0b0000_0010_0000
     * { B, BE } == 0
     * all of LCout, LMSB, c_t, c_b are 0
     *
     * top == 0x20, base == 0x0
     */
    CHECK(cap.cr_pesbt == 0x1e3f00004080000);
}

TEST_CASE("bounds encoding exponent > 0", "[bounds]") {
    _cc_cap_t cap = CompressedCap128r::make_max_perms_cap(0x8000,
            0x41DF, 0xA6400);

    /*
     * SDP = 0b1111
     * M = 0b0
     * EF == 0 -> internal exponent
     * E = 52 - 45 = 7
     *
     * T[11:3] == 0 1001 1001
     * TE == 101
     * B[13:3] == 000 0010 0000
     * BE == 101
     *
     * LCout = 0, LMSB = 1
     * c_t = 0, c_b = 0
     */
    CHECK(cap.cr_pesbt == 0x1e3f00001334105);
}
