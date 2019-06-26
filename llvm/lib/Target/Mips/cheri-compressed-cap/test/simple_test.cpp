#include "../cheri_compressed_cap.h"
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "test_util.h"

TEST_CASE("Compressed NULL cap encodes to zeroes", "[nullcap]") {
    {
        cap_register_t null_cap;
        memset(&null_cap, 0, sizeof(null_cap));
        null_cap.cr_otype = CC128_OTYPE_UNSEALED;
        null_cap._cr_length = CC128_NULL_LENGTH;
        auto pesbt = compress_128cap(&null_cap);
        auto pesbt_without_xor = compress_128cap_without_xor(&null_cap);
        fprintf(stderr, "NULL ENCODED: 0x%llx\n", (long long)pesbt_without_xor);
        CHECK(pesbt_without_xor == CC128_NULL_XOR_MASK);
        check(pesbt, (uint64_t)0, "compressing NULL should result in zero pesbt");
        cap_register_t decompressed;
        memset(&decompressed, 'b', sizeof(decompressed));
        decompress_128cap(pesbt, 0, &decompressed);
        CHECK_FIELD(decompressed, base, 0);
        CHECK_FIELD(decompressed, offset, 0);
        CHECK_FIELD(decompressed, uperms, 0);
        CHECK_FIELD(decompressed, perms, 0);
        CHECK_FIELD(decompressed, pesbt_xored_for_mem, 0); // loaded pesbt xored with mask
        CHECK_FIELD_RAW(decompressed._cr_length, CC128_NULL_LENGTH);
        CHECK_FIELD(decompressed, otype, CC128_OTYPE_UNSEALED);
    }
    {
        // Same for CHERI256
        cap_register_t null_cap;
        memset(&null_cap, 0, sizeof(null_cap));
        null_cap.cr_otype = CC256_OTYPE_UNSEALED;
        null_cap._cr_length = CC256_NULL_LENGTH;
        inmemory_chericap256 buffer;
        compress_256cap(&buffer, &null_cap);
        check(buffer.u64s[0], (uint64_t)0, "compressing NULL should result in zero[0]");
        check(buffer.u64s[1], (uint64_t)0, "compressing NULL should result in zero[1]");
        check(buffer.u64s[2], (uint64_t)0, "compressing NULL should result in zero[2]");
        check(buffer.u64s[3], (uint64_t)0, "compressing NULL should result in zero[3]");

        // compressing this result should give 0 again
        cap_register_t decompressed;
        memset(&decompressed, 'b', sizeof(decompressed));
        decompress_256cap(buffer, &decompressed, false);
        CHECK_FIELD(decompressed, base, 0);
        CHECK_FIELD(decompressed, offset, 0);
        CHECK_FIELD(decompressed, uperms, 0);
        CHECK_FIELD(decompressed, perms, 0);
        CHECK_FIELD_RAW(decompressed._cr_length, CC256_NULL_LENGTH);
        CHECK_FIELD(decompressed, otype, CC256_OTYPE_UNSEALED);
    }
}

TEST_CASE("Zeroes decode to NULL cap", "[nullcap]") {
    cap_register_t result;
    memset(&result, 'a', sizeof(result));
    decompress_128cap(0, 0, &result);
    fprintf(stderr, "Decompressed 128-bit NULL cap:\n");
    dump_cap_fields(result);
    fprintf(stderr, "\n");
    CHECK_FIELD(result, base, 0);
    CHECK_FIELD(result, offset, 0);
    CHECK_FIELD(result, uperms, 0);
    CHECK_FIELD(result, perms, 0);
    CHECK_FIELD(result, pesbt_xored_for_mem, 0); // loaded pesbt xored with mask
    CHECK_FIELD_RAW(result._cr_length, CC128_NULL_LENGTH);
    CHECK_FIELD(result, otype, CC128_OTYPE_UNSEALED);

    // Same for CHERI256
    inmemory_chericap256 buffer;
    memset(&buffer.bytes, 0, sizeof(buffer));
    memset(&result, 'a', sizeof(result));
    decompress_256cap(buffer, &result, false);
    fprintf(stderr, "Decompressed 256-bit NULL cap:\n");
    dump_cap_fields(result);
    fprintf(stderr, "\n");
    CHECK_FIELD(result, base, 0);
    CHECK_FIELD(result, offset, 0);
    CHECK_FIELD(result, uperms, 0);
    CHECK_FIELD(result, perms, 0);
    CHECK_FIELD_RAW(result._cr_length, CC256_NULL_LENGTH);
    CHECK_FIELD(result, otype, CC256_OTYPE_UNSEALED);
}

#ifdef CC128_OLD_FORMAT
#define OLD_XOR_MASK 0x200001000005

static uint64_t convert_old_to_new_pesbt(uint64_t old_pesbt_for_mem) {
    // Now that exponent = 0 is encoded we need to increment it here
    uint64_t decoded_pesbt = old_pesbt_for_mem ^ OLD_XOR_MASK;
    bool internal = CC128_EXTRACT_FIELD(decoded_pesbt, INTERNAL_EXPONENT);
    if (!internal)
        return decoded_pesbt;
    auto expLow = CC128_EXTRACT_FIELD(decoded_pesbt, EXPONENT_LOW_PART);
    auto expHigh = CC128_EXTRACT_FIELD(decoded_pesbt, EXPONENT_HIGH_PART);
    auto lh = CC128_EXTRACT_FIELD(decoded_pesbt, LH);
    if (expLow == 7) {
        if (expHigh == 7) {
            lh = 1;
        } else {
            expHigh++;
        }
    } else {
        expLow++;
    }
    auto clearMask = ~(CC128_ENCODE_FIELD(UINT64_MAX, EXPONENT_LOW_PART) |
                       CC128_ENCODE_FIELD(UINT64_MAX, EXPONENT_HIGH_PART) |
                       CC128_ENCODE_FIELD(UINT64_MAX, LH));
    uint64_t result = decoded_pesbt & clearMask;
    result = result | CC128_ENCODE_FIELD(expLow, EXPONENT_LOW_PART) |
                       CC128_ENCODE_FIELD(expHigh, EXPONENT_HIGH_PART) |
                       CC128_ENCODE_FIELD(lh, LH);
    return result;
}

TEST_CASE("Old format test 1", "[old]") {
    auto result = decompress_representable(convert_old_to_new_pesbt(0xffae000000000000), 0x9000000040001650);
    CHECK_FIELD(result, base, 0);
    CHECK_FIELD(result, offset, 0x9000000040001650);
    CHECK_FIELD(result, uperms, 0xf);
    CHECK_FIELD(result, perms, 0xfae);
    CHECK_FIELD_RAW(result._cr_length, (unsigned __int128)CAP_MAX_ADDRESS_PLUS_ONE);
    CHECK_FIELD(result, otype, CC128_OTYPE_UNSEALED);
}

TEST_CASE("Old format test 2", "[old]") {
    auto result = decompress_representable(convert_old_to_new_pesbt(0xfffa200b1f001633), 0x9000000040001636);
    CHECK_FIELD(result, base, 0x9000000040001636);
    CHECK_FIELD(result, offset, 0);
    CHECK_FIELD(result, uperms, 0xf);
    CHECK_FIELD(result, perms, 0xffa);
    CHECK_FIELD_RAW(result._cr_length, (unsigned __int128)6);
    CHECK_FIELD(result, otype, CC128_OTYPE_UNSEALED);
}

TEST_CASE("Old format test 3", "[old]") {
    auto result = decompress_representable(convert_old_to_new_pesbt(0xfffa201a19003035), 0x9000000040003296);
    CHECK_FIELD(result, base, 0x9000000040003030);
    CHECK_FIELD(result, offset, 0x266);
    CHECK_FIELD(result, uperms, 0xf);
    CHECK_FIELD(result, perms, 0xffa);
    CHECK_FIELD_RAW(result._cr_length, (unsigned __int128)0x400);
    CHECK_FIELD(result, otype, CC128_OTYPE_UNSEALED);
}

// 4 and 5 are from test_cp2_cincoffset_rep_underflow.log
TEST_CASE("Old format test 4", "[old]") {
    // 0x9000000040000f6c:  cincoffset c2,c1,t4
    //    Write C02|v:1 s:0 p:00007fff b:0000000000000000 l:0000000130000000
    //             |o:0000000000001010 t:0
    // 0x9000000040000f70:  csc        c2,a1,0(c18)
    //    Cap Memory Write [9000000040001030] = v:1 PESBT:fffe030000000004 Cursor:0000000000001010
    auto result = decompress_representable(convert_old_to_new_pesbt(0xfffe030000000004), 0x1010);
    CHECK_FIELD(result, base, 0);
    CHECK_FIELD(result, offset, 0x1010);
    CHECK_FIELD(result, uperms, 0xf);
    CHECK_FIELD(result, perms, 0xffe);
    CHECK_FIELD_RAW(result._cr_length, (unsigned __int128)0x130000000);
    CHECK_FIELD(result, otype, CC128_OTYPE_UNSEALED);
}

TEST_CASE("Old format test 5", "[old]") {
    // 0x9000000040000f88:  cincoffset c2,c1,t4
    //    Write C02|v:1 s:0 p:00007fff b:0000000000000000 l:0000000130000000
    //             |o:0000000000000ff0 t:0
    // 0x9000000040000f8c:  csc        c2,a1,64(c18)
    //    Cap Memory Write [9000000040001070] = v:1 PESBT:fffe030000000004 Cursor:0000000000000ff0
    auto result = decompress_representable(convert_old_to_new_pesbt(0xfffe030000000004), 0xff0);
    CHECK_FIELD(result, base, 0);
    CHECK_FIELD(result, offset, 0xff0);
    CHECK_FIELD(result, uperms, 0xf);
    CHECK_FIELD(result, perms, 0xffe);
    CHECK_FIELD_RAW(result._cr_length, (unsigned __int128)0x130000000);
    CHECK_FIELD(result, otype, CC128_OTYPE_UNSEALED);
}

TEST_CASE("Old format setbounds regression", "[old]") {
    // 0x9000000040000fdc:  cincoffsetimm	c3,c1,7
    //    Write C03|v:1 s:0 p:00078fff b:0000000000000000 l:ffffffffffffffff
    //             |o:0000000000000007 t:ffffff
    // 0x9000000040000fe0:  lui	a0,0x1000
    //    Write a0 = 0000000010000000
    // 0x9000000040000fe4:  csetbounds	c3,c3,a0
    //  -> crash
    auto cap = make_max_perms_cap(0, 7, CC128_MAX_LENGTH);
    CAPTURE(cap);
    cap_register_t with_bounds = cap;
    uint64_t requested_length = 0x0000000010000000;
    auto req_top = cap.cr_base + cap.cr_offset + requested_length;
    CAPTURE(req_top);
    bool exact = cc128_setbounds(&with_bounds, cap.cr_offset, req_top);
    CAPTURE(with_bounds);
    CHECK(!exact);
    CHECK(with_bounds.cr_base == 0x0000000000000000);
    CHECK(with_bounds.cr_offset == 0x0000000000000007);
    CHECK(with_bounds._cr_length == 0x00000000010000400);
}

#else
TEST_CASE("New format max length regression", "[new]") {
// sample input from cheritest:
// bad test from cheritest:
//     Write C24|v:1 s:0 p:00007ffd b:900000000000efe0 l:0000000000001000
//             |o:0000000000000fa0 t:3ffff
// -> 0x9000000040000a14:  csc	c24,zero,48(c11)
//    Cap Tag Write [ff70] 0 -> 1
//    Cap Memory Write [900000000000ff70] = v:1 PESBT:fffd000007f9afe4 Cursor:900000000000ff80
// This should have resulted in a different PESBT (with IE bit set)
// And it caused the following wrong capability to be loaded again (length is now 0:)
// ->
//    Cap Memory Read [900000000000ff70] = v:1 PESBT:fffd000007f9afe4 Cursor:900000000000ff80
//    Write C24|v:1 s:0 p:00007ffd b:900000000000efe0 l:0000000000000000
//             |o:0000000000000fa0 t:3ffff

    cap_register_t original;
    memset(&original, 0, sizeof(original));
    original.cr_tag = 1;
    original.cr_base = 0x900000000000efe0;
    original._cr_length = 0x1000;
    original.cr_offset = 0xfa0;
    original.cr_otype = CC128_OTYPE_UNSEALED;
    original.cr_perms = CC128_PERMS_ALL & ~2u;
    original.cr_uperms = CC128_UPERMS_ALL;
    dump_cap_fields(original);
    uint64_t pesbt_for_mem = compress_128cap(&original);
    fprintf(stderr, "Resulting PESBT= 0x%016" PRIx64 "\n", pesbt_for_mem);
    const uint64_t orig_cursor = original.cr_base + original.cr_offset;
    check(UINT64_C(0x900000000000ff80), orig_cursor, "Compressing cursor wrong?");
    cap_register_t decompressed;
    memset(&decompressed, 0, sizeof(decompressed));
    decompress_128cap(pesbt_for_mem, orig_cursor, &decompressed);
    dump_cap_fields(decompressed);
    CHECK_FIELD(decompressed, base, original.cr_base);
    CHECK_FIELD(decompressed, offset, original.cr_offset);
    CHECK_FIELD(decompressed, uperms, original.cr_uperms);
    CHECK_FIELD(decompressed, perms, original.cr_perms);
    CHECK_FIELD_RAW(decompressed._cr_length, original._cr_length);
    CHECK_FIELD(decompressed, otype, CC128_OTYPE_UNSEALED);

    CHECK(UINT64_C(0xfffd000003f9afe4) == pesbt_for_mem);
    CHECK(UINT64_C(0xfffd000003f9afe4) == compress_128cap(&decompressed));

}
#endif

static void check_representable(uint64_t base, unsigned __int128 length, uint64_t offset, bool should_work, const std::string& ctx) {
    // INFO("Checking representability for " << ctx);
    // INFO("Base = " << base);
    // INFO("Length = " << length);
    // INFO("Expected to work = " << should_work);
    CAPTURE(base, length, should_work, ctx);
    cap_register_t cap;
    memset(&cap, 0, sizeof(cap));
    cap.cr_base = base;
    cap.cr_offset = offset;
    cap._cr_length = length;
    cap.cr_tag = true;
    cap.cr_otype = CC128_OTYPE_UNSEALED;
    uint64_t compressed = compress_128cap(&cap);
    cap_register_t decompressed;
    memset(&decompressed, 0, sizeof(decompressed));

    decompress_128cap(compressed, cap.cr_base + cap.cr_offset, &decompressed);
    CAPTURE(cap);
    CAPTURE(decompressed);
    bool unsealed_roundtrip = cap.cr_base == decompressed.cr_base && cap._cr_length == decompressed._cr_length && cap.cr_offset == decompressed.cr_offset;
    bool unsealed_representable = cc128_is_representable(false, base, length, 0, cap.cr_offset);
    CHECK(unsealed_representable == should_work);
    CHECK(unsealed_roundtrip == unsealed_representable);
    // TODO: CHECK(fast_representable == unsealed_representable);

    bool sealed_representable = cc128_is_representable(true, base, length, 0, cap.cr_offset);
    decompress_128cap(compressed, cap.cr_base + cap.cr_offset, &decompressed);
    bool sealed_roundtrip = cap.cr_base == decompressed.cr_base && cap._cr_length == decompressed._cr_length && cap.cr_offset == decompressed.cr_offset;
    CHECK(sealed_representable == should_work);
    CHECK(sealed_roundtrip == unsealed_representable);
    // fprintf(stderr, "Base 0x%" PRIx64 " Len 0x%" PRIx64 "%016" PRIx64 ": roundtrip: sealed=%d, unsealed=%d -- Fast: sealed=%d, unsealed=%d\n",
    //        base, (uint64_t)(length >> 64), (uint64_t)length, sealed_roundtrip, unsealed_roundtrip, sealed_representable, unsealed_representable);
}

TEST_CASE("Check max size cap represetable", "[representable]") {
    // 0000000d b:0000000000000000 l:ffffffffffffffff |o:0000000000000000 t:ffffff
    check_representable(0, CAP_MAX_ADDRESS_PLUS_ONE, 0, true, "1 << 64 length");
    check_representable(0, 0, 0, true, "zero length");
    check_representable(0, UINT64_MAX, 0, false, "UINT64_MAX length");

    check_representable(0xffffffffff000000, 0x00000000000ffffff, 0, false, "length with too many bits");


    // regression test from QEMU broken cincoffset
    CHECK(cc128_is_representable(false, 0x00000000000b7fcc, 0x00000000000000e1, 0, 52));
    check_representable(0x00000000000b7fcc, 0x00000000000000e1, 52, true, "regression");
    // $c17: v:1 s:0 p:0007817d b:00000000401cf020 l:0000000000001800 o:0 t:-1 + 0x1800
    check_representable(0x00000000401cf020, 0x0000000000001800, 0x1800, true, "regression");
    CHECK(cc128_is_representable(false, 0x00000000401cf020, 0x0000000000001800, 0, 0x1800));
    // $c18: v:1 s:0 p:0007817d b:00000000401ffff8 l:0000000000000008 o:0 t:-1 + 0x8
    check_representable(0x00000000401ffff8, 0x0000000000000008, 0x8, true, "regression");
    CHECK(cc128_is_representable(false, 0x00000000401ffff8, 0x0000000000000008, 0, 0x8));
}
