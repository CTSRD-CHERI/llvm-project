#define TEST_CC_FORMAT_LOWER 128
#define TEST_CC_FORMAT_UPPER 128

#define HAVE_SAIL_SETBOUNDS_TEST_CASES
#include "contrib/setbounds_inputs_128.cpp"

#include "setbounds_test_common.cpp"

TEST_CASE("regression from cheritest", "[bounds]") {
    // The length of 0x0000000000ffffff should not be representable since it uses more than 14/23 bits
    // Therefore it should be rounded up but we were was not doing it correctly.
    // sail log:
    // 1 <-  t:1 s:0 perms:0x00078FFF type:0xFFFFFFFFFFFFFFFF offset:0xFFFFFFFFFF000000 base:0x0000000000000000
    // length:0x10000000000000000
    // PC: 0x900000004000114C op:0x48010848 (csetbounds)
    // 1 <-  t:1 s:0 perms:0x00078FFF type:0xFFFFFFFFFFFFFFFF offset:0x0000000000000000 base:0xFFFFFFFFFF000000
    // length:0x00000000001000000
    // qemu log:
    // 0x9000000040001148:  csetoffset	c1,c1,t0
    //    Write C01|v:1 s:0 p:00078fff b:0000000000000000 l:ffffffffffffffff
    //             |o:ffffffffff000000 t:3ffff
    // 0x900000004000114c:  csetbounds	c1,c1,at
    //    Write C01|v:1 s:0 p:00078fff b:ffffffffff000000 l:0000000000ffffff
    //             |o:0000000000000000 t:3ffff

    cc128_cap_t initial = TestAPICC::make_max_perms_cap(0, 0xFFFFFFFFFF000000, _CC_MAX_TOP);
    cc128_cap_t with_bounds = check_bounds_exact<TestAPI128>(initial, 0xffffff, false);
    CHECK(with_bounds.cr_base == 0xFFFFFFFFFF000000);
    CHECK(with_bounds.offset() == 0x0000000000000000);
    CHECK(with_bounds.length() == 0x00000000001000000);
}

TEST_CASE("Old format setbounds regression with new format", "[old]") {
    // 0x9000000040000fdc:  cincoffsetimm	c3,c1,7
    //    Write C03|v:1 s:0 p:00078fff b:0000000000000000 l:ffffffffffffffff
    //             |o:0000000000000007 t:ffffff
    // 0x9000000040000fe0:  lui	a0,0x1000
    //    Write a0 = 0000000010000000
    // 0x9000000040000fe4:  csetbounds	c3,c3,a0
    //  -> crash
    auto cap = TestAPICC::make_max_perms_cap(0, 7, _CC_MAX_TOP);
    uint64_t requested_length = 0x0000000010000000;
    cc128_cap_t with_bounds = check_bounds_exact<TestAPI128>(cap, requested_length, false);
    CHECK(with_bounds.cr_base == 0x0000000000000000);
    CHECK(with_bounds.offset() == 0x0000000000000007);
    uint64_t expected_length = 0x00000000010080000;
    CHECK(with_bounds.length() == expected_length);
}

TEST_CASE("Cheritest regression case", "[regression]") {
    // dli	$t0, 0x160600000
    // cgetdefault $c1
    // csetoffset $c1, $c1, $t0
    // dli	$t1, 0x300000
    // csetbounds $c1, $c1, $t1
    auto cap = TestAPICC::make_max_perms_cap(0, 0x160600000, _CC_MAX_TOP);
    cc128_cap_t with_bounds = check_bounds_exact<TestAPI128>(cap, 0x300000, true);
    CHECK(with_bounds.cr_base == 0x160600000);
    CHECK(with_bounds.offset() == 0);
    CHECK(with_bounds.length() == 0x300000);
    CHECK(with_bounds.address() == cap.address());
}

TEST_CASE("Fuzzer assertion exact untagged", "[fuzz]") {
    // Regression test for an assertion found by fuzzing setbounds.
    constexpr uint64_t pesbt = UINT64_C(0x00f58dffffffffff);
    constexpr uint64_t base = UINT64_C(0xff80000000000000);
    constexpr uint64_t addr = UINT64_C(0xf806ffffffff3e0a);
    constexpr TestAPICC::length_t top = base;
    TestAPICC::cap_t decoded = TestAPICC::decompress_raw(pesbt, addr, false);
    REQUIRE(decoded.base() == base);
    REQUIRE(decoded.top() == top);
    REQUIRE(decoded.address() == addr);
    {
        TestAPICC::cap_t tmp = decoded;
        _cc_N(set_addr)(&tmp, base);
        TestAPICC::cap_t sail_setbounds_base_top = tmp;
        TestAPICC::cap_t cc_setbounds_base_top = tmp;
        TestAPICC::sail_setbounds(&sail_setbounds_base_top, decoded.length());
        TestAPICC::setbounds(&cc_setbounds_base_top, decoded.length());
        CHECK(sail_setbounds_base_top == cc_setbounds_base_top);
    }
    {
        auto sail_setbounds_addr_top = decoded;
        auto cc_setbounds_addr_top = decoded;
        TestAPICC::sail_setbounds(&sail_setbounds_addr_top, decoded.top() - decoded.address());
        TestAPICC::setbounds(&cc_setbounds_addr_top, decoded.top() - decoded.address());
        CHECK(cc_setbounds_addr_top == cc_setbounds_addr_top);
    }

    do_csetbounds<TestAPICC>(decoded, nullptr, decoded.length());
    decoded._cr_cursor = base;
    do_csetbounds<TestAPICC>(decoded, nullptr, decoded.length());
}

TEST_CASE("TestRIG assertion untagged max cap", "[fuzz]") {
    // Regression test for an assertion found by fuzzing QEMU vs sail using testrig.
    // The length is too large, but this should be fine for untagged values.
    const TestAPICC::cap_t cap = TestAPICC::make_null_derived_cap(0xfffffffffa119000);
    constexpr uint64_t req_len = UINT64_C(0xfffffffffa119000);
    CHECK(cap.address() == req_len);

    auto sail_setbounds_result = cap;
    auto cc_setbounds_result = cap;
    TestAPICC::sail_setbounds(&sail_setbounds_result, req_len);
    TestAPICC::setbounds(&cc_setbounds_result, req_len);
    CHECK(sail_setbounds_result == cc_setbounds_result);
    // The requested top of 0x1fffffffff4232000 will be rounded to 0x20000000000000000 which is not
    // representable with the 65-bit top and is therefore truncated to zero.
    CHECK(sail_setbounds_result.top() == 0);
    do_csetbounds<TestAPICC>(cap, nullptr, req_len);
    auto cap2 = cap;
    cap2._cr_cursor = cap.base();
    do_csetbounds<TestAPICC>(cap2, nullptr, req_len);
}

TEST_CASE("TestRIG setbounds result mismatch", "[fuzz]") {
    // [0] [M]: 0x0000000080000000 (0x93420093) addi ra, tp, 2356
    // x1 <-  t:0 s:0 perms:0x00000 type:0xFFFFFFFFFFFFFFFF offset:0xFFFFFFFFFFFFF934 base:0x0000000000000000
    //        top:0b10000000000000000000000000000000000000000000000000000000000000000 length:0x10000000000000000
    // [1] [M]: 0x0000000080000004 (0xCB10A85B) csetboundsimm ca6, cra, 3249
    // x16 <-  t:0 s:0 perms:0x00000 type:0xFFFFFFFFFFFFFFFF offset:0x0000000000000000 base:0xFFFFFFFFFFFFF934
    //         top:0b10000000000000000000000000000000000000000000000000000010111100101 length:0x00000000000000CB1
    const TestAPICC::cap_t cap = TestAPICC::make_null_derived_cap(UINT64_C(0xFFFFFFFFFFFFF934));
    constexpr uint64_t req_len = 3249;
    REQUIRE(cap.address() == 0xFFFFFFFFFFFFF934);

    auto sail_setbounds_result = cap;
    auto cc_setbounds_result = cap;
    TestAPICC::sail_setbounds(&sail_setbounds_result, req_len);
    TestAPICC::setbounds(&cc_setbounds_result, req_len);
    CHECK(sail_setbounds_result == cc_setbounds_result);
    REQUIRE(cc_setbounds_result.base() == UINT64_C(0xFFFFFFFFFFFFF934));
    REQUIRE(cc_setbounds_result.address() == UINT64_C(0xFFFFFFFFFFFFF934));
    REQUIRE(cc_setbounds_result.top() == CC128_MAX_TOP + 0b10111100101);
    REQUIRE(cc_setbounds_result.length() == req_len);

    check_csetbounds_invariants<TestAPICC>(cap, cc_setbounds_result, true, req_len);
}
