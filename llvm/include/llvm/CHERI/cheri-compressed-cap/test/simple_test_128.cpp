#define TEST_CC_FORMAT_LOWER 128
#define TEST_CC_FORMAT_UPPER 128

#include "simple_test_common.cpp"

TEST_CASE("QEMU representability regression tests", "[representable]") {
    // regression test from QEMU broken cincoffset
    CHECK(check_repr(false, 0x00000000000b7fcc, 0x00000000000000e1, 52));
    check_representable(0x00000000000b7fcc, 0x00000000000000e1, 52, true, "regression");
    // $c17: v:1 s:0 p:0007817d b:00000000401cf020 l:0000000000001800 o:0 t:-1 + 0x1800
    check_representable(0x00000000401cf020, 0x0000000000001800, 0x1800, true, "regression");
    CHECK(check_repr(false, 0x00000000401cf020, 0x0000000000001800, 0x1800));
    // $c18: v:1 s:0 p:0007817d b:00000000401ffff8 l:0000000000000008 o:0 t:-1 + 0x8
    check_representable(0x00000000401ffff8, 0x0000000000000008, 0x8, true, "regression");
    CHECK(check_repr(false, 0x00000000401ffff8, 0x0000000000000008, 0x8));
}

TEST_CASE("Reprentability with TOP>MAX_TOP", "[representable]") {
    auto cap = TestAPICC::make_max_perms_cap(0xffff002d01ffc000, 0xffff002d02013ff6, 0xffff002d027fc000);
    CHECK(cap.cr_pesbt == 0xffff1ffffffe7ffb);
    CHECK(!TestAPICC::sail_fast_is_representable(cap, 0));
    CHECK(!TestAPICC::sail_precise_is_representable(cap, 0));
    CHECK(!TestAPICC::fast_is_representable_new_addr(cap, 0));
    // The following line used to assert with cdp->_cr_top <= ((cc128_length_t)1u << 64)
    CHECK(!TestAPICC::precise_is_representable_new_addr(cap, 0));
    // Decode a new capability with the same pesbt value and check that the bounds differ (and are > MAX_TOP)
    const _cc_cap_t cap2 = TestAPICC::decompress_raw(cap.cr_pesbt, 0, false);
    CHECK(cap2.base() != cap.base());
    CHECK(cap2.top() != cap.top());
    CHECK(cap2.top() > _CC_MAX_TOP);
    CHECK(cap2.base() == 0xffffffffffffc000);
    CHECK(cap2.top() == (_CC_MAX_TOP | 0x7fc000));
}
