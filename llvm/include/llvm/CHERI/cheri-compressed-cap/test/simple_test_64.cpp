#define TEST_CC_FORMAT_LOWER 64
#define TEST_CC_FORMAT_UPPER 64

#include "simple_test_common.cpp"

TEST_CASE("Fast representability mismatch 1", "[fuzz]") {
    checkFastRepCheckSucceeds(/*pesbt=*/0, /*addr=*/8, /*expected_base=*/0, /*expected_top=*/0, /*new_addr=*/0);
}

TEST_CASE("Fast representability mismatch 2", "[fuzz]") {
    checkFastRepCheckSucceeds(/*pesbt=*/0x0000000a002020, /*addr=*/0xa, /*expected_base=*/0x20, /*expected_top=*/0x20,
                              /*new_addr=*/0);
}
