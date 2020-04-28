// REQUIRES: mips-registered-target

// RUN: rm -fv %t.csv
// RUN: %cheri_purecap_cc1 %s -mllvm -cheri-cap-table-abi=pcrel -cheri-bounds=conservative -debug-info-kind=standalone \
// RUN:   -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t.csv -S -o /dev/null
// RUN: FileCheck -input-file %t.csv %s -check-prefixes CSV

// CSV: alignment_bits,size,kind,source_loc,compiler_pass,details

extern int foo(int, ...);

int test(void) {
  return foo(3, 1, 2, 3, 4ULL); // promoted to u64 in variadic call -> 4 * 8 bytes
  // CSV-NEXT:   0,32,s,"{{.+}}/csetbounds-stats-variadic-call.c:[[@LINE-1]]","MIPS variadic call lowering","setting varargs bounds for call to foo"
}

// CSV-EMPTY:
