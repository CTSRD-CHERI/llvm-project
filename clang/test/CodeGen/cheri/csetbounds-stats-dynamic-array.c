// REQUIRES: mips-registered-target, asserts

// RUN: rm -fv %t.csv
// RUN: %cheri128_purecap_cc1 %s -mllvm -cheri-cap-table-abi=pcrel -cheri-bounds=conservative -debug-info-kind=standalone \
// RUN:   -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t.csv -S -o /dev/null -O2
// RUN: FileCheck -input-file %t.csv %s -check-prefixes CSV


// CSV: alignment_bits,size,kind,source_loc,compiler_pass,details

// CSV-NEXT: 11,<unknown>,s,"{{.+}}/csetbounds-stats-dynamic-array.c:17","CHERI sandbox ABI setup","set bounds on local variable buf"
// CSV-NEXT: 11,<unknown>,s,"{{.+}}/csetbounds-stats-dynamic-array.c:23","CHERI sandbox ABI setup","set bounds on anonymous AllocaInst of type i8 addrspace(200)*"

extern int do_stuff(char* buf, int n);

int test(int n) {
  char buf[n];
  return do_stuff(buf, n);
  // CSV-NEXT:   11,<unknown>,s,"{{.+}}/csetbounds-stats-dynamic-array.c:[[@LINE-2]]","ExpandDYNAMIC_STACKALLOC",""
}

int test2(int n) {
  char* buf = __builtin_alloca(n);
  return do_stuff(buf, n);
  // CSV-NEXT: 11,<unknown>,s,"{{.+}}/csetbounds-stats-dynamic-array.c:[[@LINE-2]]","ExpandDYNAMIC_STACKALLOC",""
}

// CSV-EMPTY:
