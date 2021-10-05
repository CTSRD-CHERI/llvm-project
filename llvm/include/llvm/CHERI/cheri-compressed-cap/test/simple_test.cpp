#include "../cheri_compressed_cap.h"
#include <unistd.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "sail_wrapper.h"
#include "test_util.h"

TEST_CASE("Compressed NULL cap encodes to zeroes", "[nullcap]") {
    {
        cc128_cap_t null_cap;
        memset(&null_cap, 0, sizeof(null_cap));
        cc128_update_otype(&null_cap, CC128_OTYPE_UNSEALED);
        null_cap._cr_top = CC128_NULL_TOP;
        null_cap.cr_pesbt = CC128_NULL_PESBT;
        bool ebt_exact = true;
        uint32_t computed_ebt = cc128_compute_ebt(null_cap.cr_base, null_cap._cr_top, NULL, &ebt_exact);
        REQUIRE((computed_ebt & CC128_FIELD_EBT_MASK64) == (null_cap.cr_pesbt & CC128_FIELD_EBT_MASK64));
        REQUIRE(ebt_exact);
        auto pesbt = cc128_compress_mem(&null_cap);
        auto pesbt_from_sail = sail_compress_128_mem(&null_cap);
        CHECK(pesbt == pesbt_from_sail);
        auto pesbt_without_xor = cc128_compress_raw(&null_cap);
        auto pesbt_from_sail_without_xor = sail_compress_128_raw(&null_cap);
        CHECK(pesbt_without_xor == pesbt_from_sail_without_xor);
        fprintf(stderr, "NULL ENCODED: 0x%llx\n", (long long)pesbt_without_xor);
        CHECK(pesbt_without_xor == CC128_NULL_XOR_MASK);
        check(pesbt, (uint64_t)0, "compressing NULL should result in zero pesbt");
        cc128_cap_t decompressed;
        memset(&decompressed, 'b', sizeof(decompressed));
        cc128_decompress_mem(pesbt, 0, false, &decompressed);
        CHECK_FIELD(decompressed, base, 0);
        CHECK(decompressed.address() == 0);
        CHECK_FIELD(decompressed, software_permissions, 0);
        CHECK_FIELD(decompressed, permissions, 0);
        CHECK((decompressed.cr_pesbt & CC128_FIELD_EBT_MASK64) == (CC128_NULL_PESBT & CC128_FIELD_EBT_MASK64));
        CHECK(cc128_get_reserved(&decompressed) == 0); // reserved bits
        CHECK(decompressed.length() == CC128_NULL_LENGTH);
        CHECK(decompressed.top() == CC128_NULL_TOP);
        CHECK_FIELD(decompressed, type, CC128_OTYPE_UNSEALED);
    }
    {
        // Same for CHERI256
        cc256_cap_t null_cap;
        memset(&null_cap, 0, sizeof(null_cap));
        null_cap.cr_otype = CC256_OTYPE_UNSEALED;
        null_cap.cr_length = CC256_NULL_LENGTH;
        inmemory_chericap256 buffer;
        compress_256cap(&buffer, &null_cap);
        check(buffer.u64s[0], (uint64_t)0, "compressing NULL should result in zero[0]");
        check(buffer.u64s[1], (uint64_t)0, "compressing NULL should result in zero[1]");
        check(buffer.u64s[2], (uint64_t)0, "compressing NULL should result in zero[2]");
        check(buffer.u64s[3], (uint64_t)0, "compressing NULL should result in zero[3]");

        // compressing this result should give 0 again
        cc256_cap_t decompressed;
        memset(&decompressed, 'b', sizeof(decompressed));
        decompress_256cap(buffer, &decompressed, false);
        CHECK_FIELD(decompressed, base, 0);
        CHECK_FIELD(decompressed, offset, 0);
        CHECK_FIELD(decompressed, software_permissions, 0);
        CHECK_FIELD(decompressed, permissions, 0);
        CHECK_FIELD_RAW(decompressed.length(), CC256_NULL_LENGTH);
        CHECK_FIELD(decompressed, type, CC256_OTYPE_UNSEALED);
    }
}

TEST_CASE("Zeroes decode to NULL cap", "[nullcap]") {
    cc128_cap_t result128;
    memset(&result128, 'a', sizeof(result128));
    CompressedCap128::decompress_mem(0, 0, false, &result128);
    fprintf(stderr, "Decompressed 128-bit NULL cap:\n");
    dump_cap_fields(result128);
    fprintf(stderr, "\n");
    CHECK_FIELD(result128, base, 0);
    CHECK_FIELD(result128, offset, 0);
    CHECK_FIELD(result128, software_permissions, 0);
    CHECK_FIELD(result128, permissions, 0);
    CHECK_FIELD(result128, flags, 0);
    CHECK_FIELD(result128, reserved_bits, 0);
    CHECK((result128.cr_pesbt & CC128_FIELD_EBT_MASK64) == (CC128_NULL_PESBT & CC128_FIELD_EBT_MASK64));
    CHECK_FIELD_RAW(result128.length(), CC128_NULL_LENGTH);
    CHECK_FIELD(result128, type, CC128_OTYPE_UNSEALED);

    cc64_cap_t result64;
    memset(&result64, 'a', sizeof(result64));
    CompressedCap64::decompress_mem(0, 0, false, &result64);
    fprintf(stderr, "Decompressed 128-bit NULL cap:\n");
    dump_cap_fields(result64);
    fprintf(stderr, "\n");
    CHECK_FIELD(result64, base, 0);
    CHECK_FIELD(result64, offset, 0);
    CHECK_FIELD(result64, software_permissions, 0);
    CHECK_FIELD(result64, permissions, 0);
    CHECK_FIELD(result64, flags, 0);
    CHECK_FIELD(result64, reserved_bits, 0);
    CHECK((result64.cr_pesbt & CC64_FIELD_EBT_MASK64) == (CC64_NULL_PESBT & CC64_FIELD_EBT_MASK64));
    CHECK_FIELD_RAW(result64.length(), CC64_NULL_LENGTH);
    CHECK_FIELD(result64, type, CC64_OTYPE_UNSEALED);

    // Same for CHERI256
    inmemory_chericap256 buffer;
    cc256_cap_t result;
    memset(&buffer.bytes, 0, sizeof(buffer));
    memset(&result, 'a', sizeof(result));
    decompress_256cap(buffer, &result, false);
    fprintf(stderr, "Decompressed 256-bit NULL cap:\n");
    dump_cap_fields(result);
    fprintf(stderr, "\n");
    CHECK_FIELD(result, base, 0);
    CHECK_FIELD(result, offset, 0);
    CHECK_FIELD(result, software_permissions, 0);
    CHECK_FIELD(result, permissions, 0);
    CHECK_FIELD(result64, flags, 0);
    CHECK_FIELD(result64, reserved_bits, 0);
    CHECK_FIELD_RAW(result.length(), CC256_NULL_LENGTH);
    CHECK_FIELD(result, type, CC256_OTYPE_UNSEALED);
}

static void check_representable(uint64_t base, cc128_length_t length, uint64_t offset, bool should_work,
                                const std::string& ctx) {
    // INFO("Checking representability for " << ctx);
    // INFO("Base = " << base);
    // INFO("Length = " << length);
    // INFO("Expected to work = " << should_work);
    CAPTURE(base, length, should_work, ctx);
    cc128_cap_t cap;
    memset(&cap, 0, sizeof(cap));
    cap.cr_base = base;
    cap._cr_cursor = base + offset;
    cap._cr_top = base + length;
    cap.cr_tag = true;
    bool exact_input = false;
    cap.cr_pesbt = cc128_compute_ebt(cap.cr_base, cap._cr_top, NULL, &exact_input);
    cc128_update_otype(&cap, CC128_OTYPE_UNSEALED);
    REQUIRE(exact_input == should_work);
    uint64_t compressed = cc128_compress_mem(&cap);
    cc128_cap_t decompressed;
    memset(&decompressed, 0, sizeof(decompressed));

    cc128_decompress_mem(compressed, cap.cr_base + cap.offset(), cap.cr_tag, &decompressed);
    CAPTURE(cap);
    CAPTURE(decompressed);
    bool unsealed_roundtrip = cap.cr_base == decompressed.cr_base && cap.length() == decompressed.length() &&
                              cap.offset() == decompressed.offset();
    bool unsealed_representable = cc128_is_representable_cap_exact(&cap);
    CHECK(unsealed_representable == should_work);
    CHECK(unsealed_roundtrip == unsealed_representable);
    // TODO: CHECK(fast_representable == unsealed_representable);
    auto cap_sealed = cap;
    cc128_update_otype(&cap_sealed, 22);
    bool sealed_representable = cc128_is_representable_cap_exact(&cap_sealed);
    cc128_decompress_mem(compressed, cap.cr_base + cap.offset(), cap.cr_tag, &decompressed);
    bool sealed_roundtrip = cap.cr_base == decompressed.cr_base && cap.length() == decompressed.length() &&
                            cap.offset() == decompressed.offset();
    CHECK(sealed_representable == should_work);
    CHECK(sealed_roundtrip == unsealed_representable);
    // fprintf(stderr, "Base 0x%" PRIx64 " Len 0x%" PRIx64 "%016" PRIx64 ": roundtrip: sealed=%d, unsealed=%d -- Fast:
    // sealed=%d, unsealed=%d\n",
    //        base, (uint64_t)(length >> 64), (uint64_t)length, sealed_roundtrip, unsealed_roundtrip,
    //        sealed_representable, unsealed_representable);
}

static inline bool check_repr(bool sealed, uint64_t base, uint64_t length, uint64_t offset) {
    (void)sealed;
    auto cap = make_max_perms_cap(base, base + offset, base + length);
    return cc128_is_representable_cap_exact(&cap);
}

TEST_CASE("Check max size cap represetable", "[representable]") {
    // 0000000d b:0000000000000000 l:ffffffffffffffff |o:0000000000000000 t:ffffff
    check_representable(0, CC128_MAX_ADDRESS_PLUS_ONE, 0, true, "1 << 64 length");
    check_representable(0, 0, 0, true, "zero length");
    check_representable(0, UINT64_MAX, 0, false, "UINT64_MAX length");

    check_representable(0xffffffffff000000, 0x00000000000ffffff, 0, false, "length with too many bits");

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

TEST_CASE("Check NULL mask matches sail", "[sail]") {
    CHECK(sail_null_pesbt_128() == CC128_NULL_PESBT);
    CHECK(sail_null_pesbt_128() == CC128_NULL_XOR_MASK);
}

TEST_CASE("Check 64-bit NULL mask matches sail", "[sail]") {
    CHECK(sail_null_pesbt_64() == CC64_NULL_PESBT);
    // CHECK(sail_null_pesbt_64() == CC64_NULL_XOR_MASK);
}
