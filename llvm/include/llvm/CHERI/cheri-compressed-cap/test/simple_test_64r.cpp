
#define TEST_CC_FORMAT_LOWER 64r
#define TEST_CC_FORMAT_UPPER 64R

#include "test_common.cpp"
#include "cap_m_ap.h"

TEST_CASE("update ct", "[ct]") {
    _cc_cap_t cap;

    _cc_N(update_ct)(&cap, 1);
    CHECK_FIELD(cap, is_sealed, 1);
}

/*
 * AP compression
 */

/* --- lvbits = 0, valid permission set --- */

TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_R, CAP_AP_Q0 | 1)
TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_W, CAP_AP_Q0 | 4)
TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_R | CAP_AP_W, CAP_AP_Q0 | 5)

TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR,
        CAP_AP_Q1 | 0)
TEST_CASE_M_AP_COMP(LVB_0,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR,
        CAP_AP_Q1 | 1)
TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM,
        CAP_AP_Q1 | 2)
TEST_CASE_M_AP_COMP(LVB_0,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM,
        CAP_AP_Q1 | 3)
TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM,
        CAP_AP_Q1 | 4)
TEST_CASE_M_AP_COMP(LVB_0,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM,
        CAP_AP_Q1 | 5)
TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W,
        CAP_AP_Q1 | 6)
TEST_CASE_M_AP_COMP(LVB_0,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W,
        CAP_AP_Q1 | 7)

TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_R | CAP_AP_C, CAP_AP_Q2 | 3)

TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_LM | CAP_AP_C | CAP_AP_R, CAP_AP_Q3 | 3)
TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_LM | CAP_AP_C | CAP_AP_R | CAP_AP_W, CAP_AP_Q3 | 7)

/* --- lvbits = 1, valid permission set --- */

TEST_CASE_M_AP_COMP(LVB_1, 0, CAP_AP_R, CAP_AP_Q0 | 1)
TEST_CASE_M_AP_COMP(LVB_1, 0, CAP_AP_W, CAP_AP_Q0 | 4)
TEST_CASE_M_AP_COMP(LVB_1, 0, CAP_AP_R | CAP_AP_W, CAP_AP_Q0 | 5)

TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL,
        CAP_AP_Q1 | 0)
TEST_CASE_M_AP_COMP(LVB_1,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL,
        CAP_AP_Q1 | 1)
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL,
        CAP_AP_Q1 | 2)
TEST_CASE_M_AP_COMP(LVB_1,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL,
        CAP_AP_Q1 | 3)
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL,
        CAP_AP_Q1 | 4)
TEST_CASE_M_AP_COMP(LVB_1,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL,
        CAP_AP_Q1 | 5)
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W,
        CAP_AP_Q1 | 6)
TEST_CASE_M_AP_COMP(LVB_1,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W,
        CAP_AP_Q1 | 7)

TEST_CASE_M_AP_COMP(LVB_1, 0, CAP_AP_R | CAP_AP_C, CAP_AP_Q2 | 3)
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_SL,
        CAP_AP_Q2 | 6)
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM,
        CAP_AP_Q2 | 7)

TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL,
        CAP_AP_Q3 | 3)
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL,
        CAP_AP_Q3 | 6)
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL,
        CAP_AP_Q3 | 7)

/* --- lvbits = 0, invalid permission set --- */

/* Only C is not a valid set of permissions. */
TEST_CASE_M_AP_COMP(LVB_0, 0, CAP_AP_C, 0);

/* M must not be set in Q2 */
TEST_CASE_M_AP_COMP(LVB_0, 1, CAP_AP_R | CAP_AP_C, 0);

/* EL is not used for lvbits == 0 */
TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL,
        0);

/* SL is not used for lvbits == 0 */
TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL,
        0);

/* there is no valid combination with ASR and without X */
TEST_CASE_M_AP_COMP(LVB_0,
        0, CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR,
        0);

/* --- lvbits = 1, invalid permission set --- */

/* SL makes no sense without W */
TEST_CASE_M_AP_COMP(LVB_1,
        0, CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL,
        0);

/*
 * AP decompression
 */

/* --- lvbits = 0, valid encoding --- */

TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 1, 0, CAP_AP_R)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 4, 0, CAP_AP_W)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 5, 0, CAP_AP_R | CAP_AP_W)

TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 1,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 2,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM)
TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 3,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM)
TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 4,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM)
TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 5,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM)
TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 6,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W)
TEST_CASE_M_AP_DECOMP(LVB_0,
        CAP_AP_Q1 | 7,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W)

TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 3,
        0, CAP_AP_R | CAP_AP_C)

TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 3, 0, CAP_AP_LM | CAP_AP_C | CAP_AP_R)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 7, 0, CAP_AP_LM | CAP_AP_C | CAP_AP_R | CAP_AP_W)

/* --- lvbits = 1, valid encoding --- */

TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 1, 0, CAP_AP_R)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 4, 0, CAP_AP_W)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 5, 0, CAP_AP_R | CAP_AP_W)

TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 1,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 2,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 3,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 4,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 5,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_EL | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 6,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W)
TEST_CASE_M_AP_DECOMP(LVB_1,
        CAP_AP_Q1 | 7,
        1, CAP_AP_X | CAP_AP_R | CAP_AP_W)

TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 3, 0, CAP_AP_R | CAP_AP_C)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 6,
        0, CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 7,
        0, CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM)

TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 3, 0, CAP_AP_LM | CAP_AP_C | CAP_AP_R | CAP_AP_EL)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 6, 0, CAP_AP_LM | CAP_AP_C | CAP_AP_R | CAP_AP_W | CAP_AP_EL | CAP_AP_SL)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 7, 0, CAP_AP_LM | CAP_AP_C | CAP_AP_R | CAP_AP_W | CAP_AP_EL)

/* --- lvbits = 0, invalid encoding --- */

/* invalid in Q0 -> no permissions */
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 0, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 2, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 3, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 6, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q0 | 7, 0, 0)

/* invalid in Q2 -> all permissions */
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 1,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 2,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 4,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 5,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 6,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q2 | 7,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)

/* invalid in Q3 -> all permissions */
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 0,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 1,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 2,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 4,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 5,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)
TEST_CASE_M_AP_DECOMP(LVB_0, CAP_AP_Q3 | 6,
        0, CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR)

/* --- lvbits = 1, invalid encoding --- */

/* invalid in Q0 -> no permissions */
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 0, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 2, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 3, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 6, 0, 0)
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q0 | 7, 0, 0)

/* invalid in Q2 -> all permissions */
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 0,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 1,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 2,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 4,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q2 | 5,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))

/* invalid in Q3 -> all permissions */
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 0,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 1,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 2,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 4,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))
TEST_CASE_M_AP_DECOMP(LVB_1, CAP_AP_Q3 | 5,
        0, (CAP_AP_X | CAP_AP_R | CAP_AP_W | CAP_AP_C | CAP_AP_LM | CAP_AP_ASR | CAP_AP_EL | CAP_AP_SL))


TEST_CASE("bounds encoding, internal exponent, L8 = 1", "[bounds]") {
    /* params are base, cursor, top */
    _cc_cap_t cap = CompressedCap64r::make_max_perms_cap(0x0, 0x1000, 0x8000);

    /*
     * 11 SDP
     * 01000 AP quadrant 1, execute + ASR
     * 0000 reserved
     * 0 S
     * 0 EF
     * 1 L8
     * 000000 T[7:2]
     * 00 TE
     * 00000000 B[9:2]
     * 01 BE
     *
     * internal exponent, E = 24 - 0b10001 = 7
     * LCout = 0, LMSB = 1
     * c_t = 0, c_b = 0
     */
    CHECK(cap.cr_pesbt == 0xD0040001);
}

TEST_CASE("bounds encoding, exponent > 0, T8==0", "[bounds]") {
    _cc_cap_t cap = CompressedCap64r::make_max_perms_cap(0x4C000000,
            0x90000000, 0xAC000000);

    /*
     * 11 SDP
     * 01000 AP quadrant 1, execute + ASR
     * 0000 reserved
     * 0 S
     * 0 EF
     * 0 L8
     * 101100 T[7:2]
     * 00 TE
     * 01001100 B[9:2]
     * 10 BE
     *
     * internal exponent, E = 24 - 0b00010 = 22
     * LCout = 0, LMSB = 1
     * c_t = 0, c_b = 0
     */
    CHECK(cap.cr_pesbt == 0xD002c132);
}

TEST_CASE("bounds encoding, exponent > 0, T8==0, c_b==-1", "[bounds]") {
    _cc_cap_t cap = CompressedCap64r::make_max_perms_cap(0x9F000000,
            0xA0000000, 0xAC000000);

    /*
     * 11 SDP
     * 01000 AP quadrant 1, execute + ASR
     * 0000 reserved
     * 0 S
     * 0 EF
     * 0 L8
     * 100000 T[7:2]
     * 01 TE
     * 11111000 B[9:2]
     * 01 BE
     *
     * internal exponent, E = 24 - 0b00101 = 19
     * LCout = 1, LMSB = 1
     * A < R is true, B < R is false, T < R is true
     * c_t = 0, c_b = -1
     */
     CHECK(cap.cr_pesbt == 0xD00207e1);
}
