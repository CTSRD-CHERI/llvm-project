#define TEST_CC_FORMAT_LOWER 128m
#define TEST_CC_FORMAT_UPPER 128M
#define TEST_CC_IS_MORELLO

#include "setbounds_test_common.cpp"

TEST_CASE("Morello setbounds with high bits", "[bounds]") {
    // Check that Morello flags are not changed when we set the base to exclude them.
    const TestAPICC::cap_t cap = TestAPICC::make_max_perms_cap(0, UINT64_C(0xFF00000000000944), _CC_N(MAX_LENGTH));
    REQUIRE(cap.address() == 0xFF00000000000944);
    bool was_exact = false;
    auto result = do_csetbounds<TestAPICC>(cap, &was_exact, 16);
    CHECK(result.address() == 0xFF00000000000944);
    CHECK(result.length() == 16);
    CHECK(result.base() == 0x944); // Base should not include the high 8 bits
    CHECK(result.top() == 0x954);  // Same for top
    CHECK(was_exact);
}

TEST_CASE("Fuzzer generated behaviour difference 1", "[fuzz]") {
    // Check that Morello flags are not set if setbounds overflows (found by fuzzer).
    TestAPICC::cap_t cap = TestAPICC::make_max_perms_cap(0x000001fffffebef8, 0xff0001ffffff0011, 0x0000001fffffec0f8);
    cap.cr_tag = false;
    uint64_t req_len = UINT64_C(0xffffff7272727272);
    bool was_exact = false;
    CHECK(cap.cr_exp == 0);
    auto result = do_csetbounds<TestAPICC>(cap, &was_exact, req_len);
    CHECK(result.address() == 0xff0001ffffff0011);
    CHECK(result.base() == 0x0000000000000000);
    CHECK(result.top() == (CC128M_MAX_ADDRESS_PLUS_ONE | 0x0020000000000000));
    CHECK(result.length() == (CC128M_MAX_ADDRESS_PLUS_ONE | 0x0020000000000000));
    CHECK(result.cr_exp == 50);
    CHECK(!was_exact);
}

TEST_CASE("Fuzzer incorrect exact result 1", "[fuzz]") {
    TestAPICC::cap_t cap =
        TestAPICC::decompress_raw(/*pesbt=*/0x27ff00007ff6fe00, /*cursor=*/0xfe00000000000000, /*tag=*/true);
    uint64_t req_len = 0x2300000000000000;
    CHECK(cap.base() == 0xfe00000000000000);
    CHECK(cap.top() == (_CC_MAX_TOP | 0x3ff0000000000000));
    bool was_exact = false;
    auto result = do_csetbounds<TestAPICC>(cap, &was_exact, req_len);
    // Top is > MAX_TOP so tag should be cleared
    CHECK(was_exact);
    CHECK(!result.cr_tag);
    CHECK(result.base() == 0xfe00000000000000);
    CHECK(result.top() == (CC128M_MAX_ADDRESS_PLUS_ONE | 0x2100000000000000));
    CHECK(result.top() - req_len == result.base());
}

TEST_CASE("Fuzzer rounded top greater existing top", "[fuzz]") {
    // We should detag if the rounded new top is greater than the existing top (found by fuzzer).
    // NB: if we add addr+req_len we get 0x3F0037222B0000F6, but due to this changing the "sign" of the capability
    // address the rounded bounds end up being > 1<<64.
    // This previously asserted because we were asserting `cap->_cr_top <= old_top` before the sign change detagged.
    TestAPICC::cap_t cap = TestAPICC::make_max_perms_cap(0, 0x3effffff00000000, _CC_MAX_TOP);
    uint64_t req_len = UINT64_C(0x37232b0000f6);
    bool was_exact = false;
    auto result = do_csetbounds<TestAPICC>(cap, &was_exact, req_len);
    CHECK(!result.cr_tag);
    CHECK(result.address() == 0x3effffff00000000);
    CHECK(result.base() == 0xfffffffc00000000);
    CHECK(result.top() == (CC128M_MAX_ADDRESS_PLUS_ONE | 0x0000372400000000));
    CHECK(result.cr_exp == 31);
    CHECK(!was_exact);
}
