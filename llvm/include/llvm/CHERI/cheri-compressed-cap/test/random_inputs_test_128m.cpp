#define TEST_CC_FORMAT_LOWER 128m
#define TEST_CC_FORMAT_UPPER 128M
#define TEST_CC_IS_MORELLO

#include "random_inputs_test_common.cpp"

TEST_CASE("Assertion failure during set_addr found by fuzzer", "[fuzz]") {
    uint64_t pesbt = 0xa98aa9a9ff7ea9a9;
    uint64_t cursor = 0xada9a9a9a9e9a9f4;
    TestAPICC::cap_t cap = TestAPICC::decompress_raw(pesbt, cursor, false);
    // NB: flag bits for base and top are not a sign-extensio of the 56-bit address.
    uint64_t expected_base = 0x5350000000000000;
    TestAPICC::length_t expected_top = _CC_MAX_TOP | 0xfef0000000000000;
    CHECK(cap.address() == cursor);
    CHECK(cap.base() == expected_base);
    CHECK(cap.top() == expected_top);
    CHECK(cap.cr_bounds_valid);
    CHECK(cap.cr_exp == 49);
    // The following check previously triggered an assertion failure because we
    // were incorrectly masking off the high bits of new_addr.
    CHECK(cc128m_is_representable_with_addr(&cap, expected_base, /*precise_representable_check=*/true));
    cc128m_set_addr(&cap, cap.base());
    CHECK(cap.base() == expected_base);
    CHECK(cap.top() == expected_top);
    CHECK(cc128m_is_representable_with_addr(&cap, expected_base, /*precise_representable_check=*/true));
}
