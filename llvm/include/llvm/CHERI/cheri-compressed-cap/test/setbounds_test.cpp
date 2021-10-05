#include "../cheri_compressed_cap.h"
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "sail_wrapper.h"
#include "test_util.h"

#if 0
struct setbounds_regressions {
    uint64_t base1;
    uint64_t base2;
    uint64_t top2;
    uint64_t top1;
} regression_inputs[] = {
    {0x000000000086D6A0, 0x000000000000004B, 0x000000007CF18F9B, 0x000000000006D6A8},
    {0x00000712B609C5B0, 0x00000000032DC20F, 0x00000008032D1C77, 0x0000000000000007},
    {0x0B87DF010D7254BB, 0x00000800085F0270, 0x000000000900A7CA, 0x00000000000049FE},
    {0x0080018A6ACD2D6C, 0x0000BEDAF8F73C0F, 0x000001991A6FD045, 0x004D37033A19B295},
    {0x0000003FFFF8EDC8, 0x0000000000032796, 0x000000902DCEEE9C, 0x0000000000003D0E},
    {0x000000000006cdf7, 0x0000000000214459, 0x0000000000086940, 0x1fffff5b88378ec7},
    {0x0010D700C6318A88, 0x383264C38950ADB7, 0x00000D5EBA967A84, 0x0000000002FFFFCE},
};
#endif

template <typename Handler>
static inline void check_csetbounds_invariants(const typename Handler::cap_t& initial_cap,
                                               const typename Handler::cap_t& with_bounds, bool was_exact,
                                               uint64_t requested_base, typename Handler::length_t requested_top) {
    CAPTURE(initial_cap, with_bounds, was_exact);
    // Address should be the same!
    REQUIRE(with_bounds.address() == initial_cap.address());
    if (was_exact) {
        REQUIRE(with_bounds.base() == requested_base);
        REQUIRE(with_bounds.top() == requested_top);
    } else {
        REQUIRE(with_bounds.base() <= requested_base); // base must not be greater than what we asked for
        REQUIRE(with_bounds.top() >= requested_top);   // top must not be less than what we asked for
        // At least one must be different otherwise was_exact is not correct
        REQUIRE((with_bounds.top() != requested_top || with_bounds.base() != requested_base));
    }
    REQUIRE(with_bounds.base() >= initial_cap.base());          // monotonicity broken
    REQUIRE(with_bounds.top() <= initial_cap.top());            // monotonicity broken
    REQUIRE(Handler::is_representable_cap_exact(&with_bounds)); // result of csetbounds must be representable
}

template <typename Handler>
static typename Handler::cap_t do_csetbounds(const typename Handler::cap_t& initial_cap,
                                             typename Handler::length_t requested_top, bool* was_exact) {
    typename Handler::cap_t with_bounds = initial_cap;
    typename Handler::addr_t requested_base = initial_cap.address();
    CAPTURE(initial_cap, requested_top, requested_base);
    bool exact = Handler::setbounds(&with_bounds, requested_base, requested_top);
    CAPTURE(with_bounds, exact);

    typename Handler::cap_t sail_with_bounds = initial_cap;
    bool sail_exact = Handler::sail_setbounds(&sail_with_bounds, requested_base, requested_top);
    CAPTURE(sail_with_bounds, sail_exact);

    CHECK(sail_with_bounds == with_bounds);
    CHECK(sail_exact == exact);
    check_csetbounds_invariants<Handler>(initial_cap, with_bounds, exact, requested_base, requested_top);

    // Check that the cr_pesbt is updated correctly and matches sail
    CHECK(with_bounds.cr_pesbt == Handler::compress_raw(&with_bounds));
    CHECK(with_bounds.cr_pesbt == Handler::sail_compress_raw(&with_bounds));
    // Re-create the bounded capability and assert that the current pesbt values matches that one.
    auto new_cap = Handler::make_max_perms_cap(with_bounds.base(), with_bounds.address(), with_bounds.top());
    CHECK(new_cap == with_bounds);
    CHECK(new_cap.cr_pesbt == with_bounds.cr_pesbt);
    CHECK(new_cap.cr_pesbt == Handler::compress_raw(&with_bounds));
    CHECK(new_cap.cr_pesbt == Handler::sail_compress_raw(&with_bounds));

    if (was_exact)
        *was_exact = exact;
    return with_bounds;
}

template <typename Handler>
static cc128_cap_t check_bounds_exact(const typename Handler::cap_t& initial_cap,
                                         typename Handler::length_t requested_length, bool should_be_exact) {
    typename Handler::length_t req_top = initial_cap.address() + (typename Handler::length_t)requested_length;
    bool exact = false;
    cc128_cap_t with_bounds = do_csetbounds<Handler>(initial_cap, req_top, &exact);
    CHECK(exact == should_be_exact);
    return with_bounds;
}

// check_monotonic

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

    cc128_cap_t initial = make_max_perms_cap(0, 0xFFFFFFFFFF000000, CC128_MAX_LENGTH);
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
    auto cap = make_max_perms_cap(0, 7, CC128_MAX_LENGTH);
    uint64_t requested_length = 0x0000000010000000;
    cc128_cap_t with_bounds = check_bounds_exact<TestAPI128>(cap, requested_length, false);
    CHECK(with_bounds.cr_base == 0x0000000000000000);
    CHECK(with_bounds.offset() == 0x0000000000000007);
    // Higher precision in old format -> more exact bounds
    uint64_t expected_length = TESTING_OLD_FORMAT ? 0x0000000010000400 : 0x00000000010080000;
    CHECK(with_bounds.length() == expected_length);
}

TEST_CASE("Cheritest regression case", "[regression]") {
    // dli	$t0, 0x160600000
    // cgetdefault $c1
    // csetoffset $c1, $c1, $t0
    // dli	$t1, 0x300000
    // csetbounds $c1, $c1, $t1
    auto cap = make_max_perms_cap(0, 0x160600000, CC128_MAX_LENGTH);
    cc128_cap_t with_bounds = check_bounds_exact<TestAPI128>(cap, 0x300000, true);
    CHECK(with_bounds.cr_base == 0x160600000);
    CHECK(with_bounds.offset() == 0);
    CHECK(with_bounds.length() == 0x300000);
    CHECK(with_bounds.address() == cap.address());
}

#include "setbounds_inputs.cpp"

static inline cc128_length_t round_up_with_cram_mask(cc128_length_t value, cc128_length_t round_down_mask) {
    // mask has all ones in the top
    REQUIRE(cc128_idx_MSNZ((uint64_t)round_down_mask) == 63);
    // Round up using by adding ~mask to go over boundary and then round down with & mask
    return (value + ~round_down_mask) & (round_down_mask);
}

static inline void check_cram_matches_setbounds(cc128_length_t req_top, const cc128_cap_t& cap_to_bound,
                                                const cc128_cap_t& setbounds_result) {
    // Check that rounding with cram is equivalent to setbounds rounding:
    uint64_t cram_value = cc128_get_alignment_mask((uint64_t)(req_top - cap_to_bound.address()));
    CAPTURE(cram_value);
    CAPTURE(cap_to_bound);
    CAPTURE(setbounds_result);
    CHECK((cap_to_bound.address() & cram_value) == setbounds_result.base());
    CHECK(round_up_with_cram_mask(req_top, cram_value) == setbounds_result.top());
}

template <typename Handler>
void test_zero_length_one_past_end() { // Create an out-of-bounds capability and set length to one
    constexpr unsigned base = 12345;
    constexpr unsigned base_plus_one = base + 1;
    auto one_past_end = Handler::make_max_perms_cap(base, base_plus_one, base_plus_one);
    bool exact;
    auto result = do_csetbounds<Handler>(one_past_end, base_plus_one, &exact);
    CHECK(result.base() == base_plus_one);
    CHECK(result.top() == base_plus_one);
    CHECK(result.address() == base_plus_one);
    CHECK(result.cr_tag == 1);
    CHECK(exact);
    // Check that calling setbounds with the same target top yields the same result
    bool same_again_exact = false;
    auto same_again = do_csetbounds<Handler>(result, base_plus_one, &same_again_exact);
    CHECK(result == same_again);
}

TEST_CASE("setbounds128 with req_top == top", "[bounds 128]") { test_zero_length_one_past_end<TestAPI128>(); }
TEST_CASE("setbounds64 with req_top == top", "[bounds 64]") { test_zero_length_one_past_end<TestAPI64>(); }

TEST_CASE("setbounds test cases from sail", "[bounds]") {
    using Handler = TestAPI128;
    for (size_t index = 0; index < array_lengthof(setbounds_inputs); index++) {
        CAPTURE(index);
        const setbounds_input& input = setbounds_inputs[index];
        auto first_input = make_max_perms_cap(0, input.base1, CC128_MAX_LENGTH);
        REQUIRE(first_input.address() == input.base1);
        REQUIRE(first_input.base() == 0);
        REQUIRE(first_input.top() == CC128_MAX_LENGTH);
        bool first_exact = false;
        const cc128_cap_t first_bounds = do_csetbounds<Handler>(first_input, input.top1, &first_exact);
        CHECK(first_bounds.base() == input.sail_cc_base1);
        CHECK(first_bounds.top() == input.sail_cc_top1);
        // Check CRAP/CRAM:
        check_cram_matches_setbounds(input.top1, first_input, first_bounds);

        // Check that calling setbounds with the same target top yields the same result
        bool first_again_exact = false;
        const cc128_cap_t first_bounds_again = do_csetbounds<Handler>(first_bounds, input.top1, &first_again_exact);
        CHECK(first_again_exact == first_exact);
        CHECK(first_bounds.base() == first_bounds_again.base());
        CHECK(first_bounds.top() == first_bounds_again.top());
        CHECK(first_bounds.address() == first_bounds_again.address());
        // Check CRAM still works:
        check_cram_matches_setbounds(input.top1, first_bounds, first_bounds_again);

        // Check the second csetbounds:
        cc128_cap_t second_input = first_bounds;
        second_input._cr_cursor += input.base2 - second_input.address();
        REQUIRE(second_input.address() == input.base2);
        bool second_exact = false;
        const cc128_cap_t second_bounds = do_csetbounds<Handler>(second_input, input.top2, &second_exact);
        CHECK(second_bounds.base() == input.sail_cc_base2);
        CHECK(second_bounds.top() == input.sail_cc_top2);
        // Check CRAP/CRAM:
        check_cram_matches_setbounds(input.top2, second_input, second_bounds);

        if (second_bounds.cr_base >= second_bounds.top()) {
            INFO("Cannot set bounds on capability that is not in-bounds");
            continue;
        }
        // Check that calling setbounds with the same target top yields the same result
        bool second_again_exact = false;
        const cc128_cap_t second_bounds_again =
            do_csetbounds<Handler>(second_bounds, input.top2, &second_again_exact);
        CHECK(second_again_exact == second_exact);
        CHECK(second_bounds.base() == second_bounds_again.base());
        CHECK(second_bounds.top() == second_bounds_again.top());
        CHECK(second_bounds.address() == second_bounds_again.address());
        check_cram_matches_setbounds(input.top2, second_bounds, second_bounds_again);
    }
}
