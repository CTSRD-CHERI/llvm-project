#include "../cheri_compressed_cap.h"
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "test_util.h"

#include "decode_inputs.cpp"


static_assert(CC128_FIELD_OTYPE_START == 27, "");
static_assert(CC128_FIELD_OTYPE_SIZE == 18, "");
static_assert(CC128_FIELD_OTYPE_LAST == 44, "");

#define CHECK_AND_SAVE_SUCCESS(expr) do { CHECK(expr); if (!(expr)) { success = false; } } while (false)


static bool check_fields_match(const cap_register_t& result, const test_input& ti, const std::string& prefix) {
        uint64_t top64 = (uint64_t)(result.cr_base + result._cr_length);
        unsigned __int128 top_full = result.cr_base + result._cr_length;
        bool top_bit65 = (top_full >> 64) & 1;

        bool success = true;
        CAPTURE(prefix);
        CHECK_AND_SAVE_SUCCESS(ti.address == result.cr_base + result.cr_offset);
        CHECK_AND_SAVE_SUCCESS(ti.bot == result.cr_base);
        CHECK_AND_SAVE_SUCCESS(ti.otype == result.cr_otype);
        CHECK_AND_SAVE_SUCCESS(ti.permissions == result.cr_perms + (result.cr_uperms << 15));
        CHECK_AND_SAVE_SUCCESS(ti.top.u64 == top64);
        CHECK_AND_SAVE_SUCCESS(ti.top.bit65 == top_bit65);
        return success;
}

static bool test_one_entry(const test_input& ti) {
    // decompress_representable(ti.input.pesbt, ti.input.cursor);
    cap_register_t result;
    memset(&result, 0, sizeof(result));
    // The inputs already have the NULL xor mask removed -> decompress_128cap_already_xored()
    decompress_128cap_already_xored(ti.input.pesbt, ti.input.cursor, &result);
    CAPTURE(ti.input.pesbt, ti.input.cursor);

    bool success = check_fields_match(result, ti, "");
    if (!success) {
        fprintf(stderr, "Decompressed raw pesbt = %016" PRIx64 ", cursor = %016" PRIx64 "\n", ti.input.pesbt, ti.input.cursor);
        dump_cap_fields(result);
    }
    // If it was a valid input ensure that recompressing results in the same pesbt field:
    uint64_t reserved_bits = CC128_EXTRACT_FIELD(ti.input.pesbt, RESERVED);
    // CHECK(UINT64_C(0), reserved_bits, "Reserved capability bits were non-zero");
    if (reserved_bits != 0) {
        // Set the PESBT field in the capreg so that the reserved bits can be added back to the capability
        // This is an invalid capability
        result.cr_pesbt_xored_for_mem = ti.input.pesbt ^ CC128_NULL_XOR_MASK;
    }


    // Now try recompressing and compare pesbt (for valid capabilities)
    unsigned __int128 top_full = result.cr_base + result._cr_length;
    // Also don't attempt to recompress massively out-of-bounds caps since that might not work:
    if (top_full >= result.cr_base && top_full <= CAP_MAX_ADDRESS_PLUS_ONE && result.cr_offset <= result._cr_length) {
        uint64_t recompressed_pesbt = compress_128cap_without_xor(&result);
        CAPTURE(recompressed_pesbt);
        if (ti.input.pesbt != recompressed_pesbt) {
            fprintf(stderr, "Note: Recompressing resulted in different pesbt = 0x%016" PRIx64
                            ", original = 0x%016" PRIx64 ", xor = 0x%016" PRIx64 "\n",
                            recompressed_pesbt, ti.input.pesbt, recompressed_pesbt ^ ti.input.pesbt);
            // This is not an error since there are multiple ways of encoding the same bot/top values with different exponents
        }
        // But even if it didn't compress the same at least all fields decompressed from the new pesbt must be the same:
        cap_register_t result_recompressed;
        memset(&result_recompressed, 0, sizeof(result_recompressed));
        // The inputs already have the NULL xor mask removed -> decompress_128cap_already_xored()
        decompress_128cap_already_xored(recompressed_pesbt, ti.input.cursor, &result_recompressed);
        success = success && check_fields_match(result_recompressed, ti, "Recompressed pesbt: ");
        if (!success) {
            fprintf(stderr, "\nOriginal decoded:\n");
            dump_cap_fields(result);
            fprintf(stderr, "\nRecompressed decoded:\n");
            dump_cap_fields(result_recompressed);
        }
        // It should now be normalized -> recompressing again should result in the same pesbt:
        if (reserved_bits != 0) {
            // Set the PESBT field in the capreg so that the reserved bits can be added back to the capability
            result_recompressed.cr_pesbt_xored_for_mem = recompressed_pesbt ^ CC128_NULL_XOR_MASK;
        }
        uint64_t recompressed_pesbt_after_normalize = compress_128cap_without_xor(&result_recompressed);
        CHECK_AND_SAVE_SUCCESS(recompressed_pesbt == recompressed_pesbt_after_normalize);
    }
    return success;
}

// bad test from cheritest:
//     Write C24|v:1 s:0 p:00007ffd b:900000000000efe0 l:0000000000001000
//             |o:0000000000000fa0 t:3ffff
// -> 0x9000000040000a14:  csc	c24,zero,48(c11)
//    Cap Tag Write [ff70] 0 -> 1
//    Cap Memory Write [900000000000ff70] = v:1 PESBT:fffd000007f9afe4 Cursor:900000000000ff80
// ->
//    Cap Memory Read [900000000000ff70] = v:1 PESBT:fffd000007f9afe4 Cursor:900000000000ff80
//    Write C24|v:1 s:0 p:00007ffd b:900000000000efe0 l:0000000000000000
//             |o:0000000000000fa0 t:3ffff

TEST_CASE("Check sail-generated inputs decode correctly", "[decode]") {
    test_one_entry(inputs[6]);
    int failure_count = 0;
    for (size_t i = 0; i < array_lengthof(inputs); i++) {
        if (!test_one_entry(inputs[i])) {
            fprintf(stderr, "Failed at index %zd\n", i);
            failure_count++;
            // break;
        }
    }
    REQUIRE(failure_count == 0);
}
