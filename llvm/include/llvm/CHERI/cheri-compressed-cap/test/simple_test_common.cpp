#include "test_common.cpp"

// Due to magic constant XOR aversion (i.e. fields are either entirely
// inverted or not at all, rather than select bits within them like in
// normal CHERI Concentrate), NULL is special in Morello, with bounds
// using the maximum encodable exponent, not the minimum necessary one
// like is used for the reset capability.
#ifndef TEST_CC_IS_MORELLO
TEST_CASE("Compressed NULL cap has canonical bounds", "[nullcap]") {
    _cc_cap_t null_cap;
    memset(&null_cap, 0, sizeof(null_cap));
    _cc_N(update_otype)(&null_cap, _CC_N(OTYPE_UNSEALED));
    null_cap._cr_top = _CC_N(NULL_TOP);
    null_cap.cr_pesbt = _CC_N(NULL_PESBT);
    bool ebt_exact = true;
    uint32_t computed_ebt = _cc_N(compute_ebt)(null_cap.cr_base, null_cap._cr_top, NULL, &ebt_exact);
    REQUIRE((computed_ebt & _CC_N(FIELD_EBT_MASK64)) == (null_cap.cr_pesbt & _CC_N(FIELD_EBT_MASK64)));
    REQUIRE(ebt_exact);
}
#endif

// TODO: Implement non-stub sail_compress_common_mem/raw for Morello
TEST_CASE("Compressed NULL cap encodes to zeroes", "[nullcap]") {
    _cc_cap_t null_cap = TestAPICC::make_null_derived_cap(0);
    auto pesbt = _cc_N(compress_mem)(&null_cap);
    auto pesbt_from_sail = _cc_sail_compress_mem(&null_cap);
    CHECK(pesbt == pesbt_from_sail);
    auto pesbt_without_xor = _cc_N(compress_raw)(&null_cap);
    auto pesbt_from_sail_without_xor = _cc_sail_compress_raw(&null_cap);
    CHECK(pesbt_without_xor == pesbt_from_sail_without_xor);
    fprintf(stderr, "NULL ENCODED: 0x%llx\n", (long long)pesbt_without_xor);
    CHECK(pesbt_without_xor == _CC_N(NULL_XOR_MASK));
    check(pesbt, (_cc_addr_t)0, "compressing NULL should result in zero pesbt");
    _cc_cap_t decompressed;
    memset(&decompressed, 'b', sizeof(decompressed));
    _cc_N(decompress_mem)(pesbt, 0, false, &decompressed);
    CHECK_FIELD(decompressed, base, 0);
    CHECK(decompressed.address() == 0);
    CHECK_FIELD(decompressed, software_permissions, 0);
    CHECK_FIELD(decompressed, permissions, 0);
    CHECK((decompressed.cr_pesbt & _CC_N(FIELD_EBT_MASK64)) == (_CC_N(NULL_PESBT) & _CC_N(FIELD_EBT_MASK64)));
    CHECK(_cc_N(get_reserved)(&decompressed) == 0); // reserved bits
    CHECK(decompressed.length() == _CC_N(NULL_LENGTH));
    CHECK(decompressed.top() == _CC_N(NULL_TOP));
    CHECK_FIELD(decompressed, type, _CC_N(OTYPE_UNSEALED));
}

TEST_CASE("Zeroes decode to NULL cap", "[nullcap]") {
    _cc_cap_t result = CompressedCapCC::decompress_mem(0, 0, false);
    fprintf(stderr, "Decompressed NULL cap:\n");
    dump_cap_fields(stderr, result);
    fprintf(stderr, "\n");
    CHECK_FIELD(result, base, 0);
    CHECK_FIELD(result, offset, 0);
    CHECK_FIELD(result, software_permissions, 0);
    CHECK_FIELD(result, permissions, 0);
    CHECK_FIELD(result, flags, 0);
    CHECK_FIELD(result, reserved_bits, 0);
    CHECK((result.cr_pesbt & _CC_N(FIELD_EBT_MASK64)) == (_CC_N(NULL_PESBT) & _CC_N(FIELD_EBT_MASK64)));
    CHECK_FIELD_RAW(result.length(), _CC_N(NULL_LENGTH));
    CHECK_FIELD(result, type, _CC_N(OTYPE_UNSEALED));
}

static void check_representable(_cc_addr_t base, _cc_length_t length, _cc_addr_t offset, bool should_work,
                                const std::string& ctx) {
    // INFO("Checking representability for " << ctx);
    // INFO("Base = " << base);
    // INFO("Length = " << length);
    // INFO("Expected to work = " << should_work);
    CAPTURE(base, length, should_work, ctx);
    bool exact_input = false;
    _cc_length_t top = base + length;
    const _cc_addr_t compressed = _cc_N(compute_ebt)(base, top, nullptr, &exact_input);
    REQUIRE(exact_input == should_work);

    _cc_cap_t decompressed;
    memset(&decompressed, 0, sizeof(decompressed));
    _cc_N(decompress_raw)(compressed, base + offset, false, &decompressed);
    CAPTURE(decompressed);
    bool unsealed_roundtrip =
        base == decompressed.cr_base && length == decompressed.length() && offset == decompressed.offset();
    CHECK(unsealed_roundtrip == should_work);
    _cc_cap_t cap_sealed;
    _cc_N(decompress_raw)(compressed, base + offset, true, &cap_sealed);
    _cc_N(update_otype)(&cap_sealed, 5);
    _cc_N(decompress_raw)(cap_sealed.cr_pesbt, base + offset, true, &cap_sealed);
    bool sealed_roundtrip =
        base == decompressed.cr_base && length == decompressed.length() && offset == decompressed.offset();
    CHECK(sealed_roundtrip == unsealed_roundtrip);
}

static inline bool check_repr(bool sealed, _cc_addr_t base, _cc_addr_t length, _cc_addr_t offset) {
    (void)sealed;
    auto cap = TestAPICC::make_max_perms_cap(base, base + offset, base + length);
    return _cc_N(is_representable_cap_exact)(&cap);
}

TEST_CASE("Check max size cap representable", "[representable]") {
    // 0000000d b:0000000000000000 l:ffffffffffffffff |o:0000000000000000 t:ffffff
    check_representable(0, _CC_N(MAX_ADDRESS_PLUS_ONE), 0, true, "MAX_ADDRESS_PLUS_ONE length");
    check_representable(0, 0, 0, true, "zero length");
    check_representable(0, _CC_N(MAX_ADDR), 0, false, "MAX_ADDR length");
    check_representable((_cc_addr_t)0xffffffffff000000, 0x00000000000ffffff, 0, false, "length with too many bits");
}

TEST_CASE("Check omnipotent capability matches sail", "[sail]") {
    // 0000000d b:0000000000000000 l:ffffffffffffffff |o:0000000000000000 t:ffffff
    auto sail_reset_cap = TestAPICC::sail_reset_capability();
    auto cc_lib_reset_cap = TestAPICC::make_max_perms_cap(0, 0, _CC_MAX_TOP);
    CHECK(sail_reset_cap == cc_lib_reset_cap);
    CHECK(_cc_N(raw_equal(&sail_reset_cap, &cc_lib_reset_cap)));
    CHECK(cc_lib_reset_cap.cr_exp == _CC_N(RESET_EXP));
}

TEST_CASE("Check make_max_perms_cap() sets cr_exp correctly", "[sail]") {
    // make_max_perms_cap was not updating the cr_exp field and kept it as the NULL exponent.
    auto cap = TestAPICC::make_max_perms_cap(0, 16, 16);
    CHECK(cap.cr_exp == 0);
}

// TODO: Implement sail_null_pesbt_128
#ifndef TEST_CC_IS_MORELLO
TEST_CASE("Check NULL mask matches sail", "[sail]") {
    CHECK(_cc_sail(null_pesbt)() == _CC_N(NULL_PESBT));
    CHECK(_cc_sail(null_pesbt)() == _CC_N(NULL_XOR_MASK));
}
#endif

static inline TestAPICC::cap_t checkFastRepCheckSucceeds(const _cc_cap_t& cap, _cc_addr_t new_addr,
                                                         bool set_addr_should_retain_tag = true) {
    bool sail_fast_rep = TestAPICC::sail_fast_is_representable(cap, new_addr);
    bool cc_fast_rep = TestAPICC::fast_is_representable_new_addr(cap, new_addr);
    CHECK(sail_fast_rep == cc_fast_rep);
    CHECK(cc_fast_rep);
    // It should also be representable if we do the full check since the bounds interpretation does not change.
    // NB: Unlike precise_is_representable_new_addr, fast_is_representable_new_addr does not look at cr_bounds_valid.
    if (cap.cr_bounds_valid) {
        CHECK(TestAPICC::precise_is_representable_new_addr(cap, new_addr));
    }
    // Check that creating a new capability with same pesbt and new address decodes to the same bounds
    TestAPICC::cap_t new_cap_with_other_cursor = TestAPICC::decompress_raw(cap.cr_pesbt, new_addr, false);
    CHECK(new_cap_with_other_cursor.base() == cap.base());
    CHECK(new_cap_with_other_cursor.top() == cap.top());
    auto updated = cap;
    updated.cr_tag = true;
    // Updating the address should retain the tag and bounds should stay the same.
    // Calls to set_addr checksfor sealed, so unseal to ensure set_addr only checks representability.
    _cc_N(update_otype)(&updated, _CC_N(OTYPE_UNSEALED));
    _cc_N(set_addr)(&updated, new_addr);
    CHECK(updated.cr_tag == set_addr_should_retain_tag);
    if (set_addr_should_retain_tag) {
        CHECK(updated.base() == cap.base());
        CHECK(updated.top() == cap.top());
    }
    return cap;
}

static inline TestAPICC::cap_t checkFastRepCheckSucceeds(_cc_addr_t pesbt, _cc_addr_t addr, _cc_addr_t expected_base,
                                                         _cc_length_t expected_top, _cc_addr_t new_addr,
                                                         bool set_addr_should_retain_tag = true) {
    TestAPICC::cap_t cap = TestAPICC::decompress_raw(pesbt, addr, false);
    TestAPICC::cap_t sail_cap = TestAPICC::sail_decode_raw(pesbt, addr, false);
    CHECK(cap == sail_cap);
    CHECK(cap.base() == expected_base);
    CHECK(cap.top() == expected_top);
    checkFastRepCheckSucceeds(cap, new_addr, set_addr_should_retain_tag);
    return cap;
}

/// Check that both the fast and the full representability check fails for the given input.
static inline void checkRepCheckFails(_cc_addr_t pesbt, _cc_addr_t addr, _cc_addr_t expected_base,
                                      _cc_length_t expected_top, _cc_addr_t new_addr) {
    TestAPICC::cap_t cap = TestAPICC::decompress_raw(pesbt, addr, false);
    CHECK(cap.base() == expected_base);
    CHECK(cap.top() == expected_top);
    bool sail_fast_rep = TestAPICC::sail_fast_is_representable(cap, new_addr);
    bool cc_fast_rep = TestAPICC::fast_is_representable_new_addr(cap, new_addr);
    CHECK(sail_fast_rep == cc_fast_rep);
    CHECK(!cc_fast_rep);
    // It should also not be representable if we do the full check since the bounds interpretation changes.
    CHECK(!TestAPICC::precise_is_representable_new_addr(cap, new_addr));
    // Check that creating a new capability with same pesbt and new address decodes to different bounds
    TestAPICC::cap_t new_cap_with_other_cursor = TestAPICC::decompress_raw(pesbt, new_addr, false);
    CHECK((new_cap_with_other_cursor.base() != expected_base || new_cap_with_other_cursor.top() != expected_top));
}

TEST_CASE("Omnipotent cap representable", "") {
    TestAPICC::cap_t max_cap = TestAPICC::make_max_perms_cap(0, 0, _CC_MAX_TOP);
    // Check that various address are representable with a full-address-space capability.
    checkFastRepCheckSucceeds(max_cap, 0);
    checkFastRepCheckSucceeds(max_cap, 1);
    checkFastRepCheckSucceeds(max_cap, _CC_MAX_ADDR - 1);
    checkFastRepCheckSucceeds(max_cap, _CC_MAX_ADDR);
    checkFastRepCheckSucceeds(max_cap, _CC_MAX_ADDR >> 1);
    // Alternating ones and zeroes
    checkFastRepCheckSucceeds(max_cap, (_cc_addr_t)UINT64_C(0xaaaaaaaaaaaaaaaa));
    // And a few arbitrary values
    checkFastRepCheckSucceeds(max_cap, 0x3fffca);
    checkFastRepCheckSucceeds(max_cap, 0x12345678);
    checkFastRepCheckSucceeds(max_cap, 1234);
    checkFastRepCheckSucceeds(max_cap, _CC_MAX_ADDR - 1234);
}

TEST_CASE("pesbt_is_correct ignores cr_extra", "") {
    TestAPICC::cap_t null_cap = TestAPICC::make_null_derived_cap(0);
    CHECK(_cc_N(pesbt_is_correct)(&null_cap));
    // Changing cr_extra previously triggered assertions, check that it is ignored.
    null_cap.cr_extra = 10;
    CHECK(_cc_N(pesbt_is_correct)(&null_cap));
}
