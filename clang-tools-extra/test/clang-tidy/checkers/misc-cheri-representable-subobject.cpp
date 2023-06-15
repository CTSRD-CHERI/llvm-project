// RUN: %check_clang_tidy -check-suffix=RV64 %s misc-cheri-representable-subobject %t -- -- -target riscv64-unknown-freebsd -march=rv64imafdcxcheri -mabi=l64pc128d -cheri-bounds=subobject-safe
// RUN: %check_clang_tidy -check-suffix=RV32 %s misc-cheri-representable-subobject %t -- -- -target riscv32-unknown-freebsd -march=rv32imafdcxcheri -mabi=il32pc64 -cheri-bounds=subobject-safe

struct test_large_subobject {
  int skew_offset;
  char large_buffer[((1 << 13) - 7)];
};
// CHECK-MESSAGES-RV64: :[[@LINE-2]]:8: warning: Field 'large_buffer' ('char[8185]') at 4 in 'test_large_subobject' with size 8185 may not be representable: offset will be imprecisely aligned to 0 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV64: :[[@LINE-3]]:8: warning: Field 'large_buffer' ('char[8185]') at 4 in 'test_large_subobject' with size 8185 may not be representable: top will be imprecisely aligned to 8192 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV32: :[[@LINE-4]]:8: warning: Field 'large_buffer' ('char[8185]') at 4 in 'test_large_subobject' with size 8185 may not be representable: offset will be imprecisely aligned to 0 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV32: :[[@LINE-5]]:8: warning: Field 'large_buffer' ('char[8185]') at 4 in 'test_large_subobject' with size 8185 may not be representable: top will be imprecisely aligned to 8192 [misc-cheri-representable-subobject]

struct test_small_subobject {
  int int_value;
  long long_value;
  char small_buffer[256];
  void *pointer_value;
};
// CHECK-MESSAGES-RV32: :[[@LINE-3]]:8: warning: Field 'small_buffer' ('char[256]') at 8 in 'test_small_subobject' with size 256 may not be representable: offset will be imprecisely aligned to 0 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV32: :[[@LINE-4]]:8: warning: Field 'small_buffer' ('char[256]') at 8 in 'test_small_subobject' with size 256 may not be representable: top will be imprecisely aligned to 288 [misc-cheri-representable-subobject]

struct test_mixed {
  char pre1[33024];
  char buf[2941200];
};
// CHECK-MESSAGES-RV64: :[[@LINE-2]]:8: warning: Field 'buf' ('char[2941200]') at 33024 in 'test_mixed' with size 2941200 may not be representable: offset will be imprecisely aligned to 32768 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV64: :[[@LINE-3]]:8: warning: Field 'buf' ('char[2941200]') at 33024 in 'test_mixed' with size 2941200 may not be representable: top will be imprecisely aligned to 2977792 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV32: :[[@LINE-5]]:8: warning: Field 'pre1' ('char[33024]') at 0 in 'test_mixed' with size 33024 may not be representable: top will be imprecisely aligned to 36864 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV32: :[[@LINE-5]]:8: warning: Field 'buf' ('char[2941200]') at 33024 in 'test_mixed' with size 2941200 may not be representable: offset will be imprecisely aligned to 0 [misc-cheri-representable-subobject]
// CHECK-MESSAGES-RV32: :[[@LINE-6]]:8: warning: Field 'buf' ('char[2941200]') at 33024 in 'test_mixed' with size 2941200 may not be representable: top will be imprecisely aligned to 3145728 [misc-cheri-representable-subobject]

struct test_flexible {
  int int_value;
  char flexbuf[];
};
