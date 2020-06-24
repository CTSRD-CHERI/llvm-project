// REQUIRES: mips-registered-target

// RUN: rm -fv %t-debug.csv %t-nodebug.csv
// RUN: %cheri_purecap_cc1 %s -cheri-bounds=conservative \
// RUN:    -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t-nodebug.csv -S -o /dev/null
// RUN: cat %t-nodebug.csv
// RUN: FileCheck -input-file %t-nodebug.csv %s -check-prefixes CSV,CSV-NODEBUG
// RUN: %cheri_purecap_cc1 %s -cheri-bounds=conservative -debug-info-kind=standalone \
// RUN:   -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t-debug.csv -S -o /dev/null
// RUN: cat %t-debug.csv
// RUN: FileCheck -input-file %t-debug.csv %s -check-prefixes CSV,CSV-DEBUG

// CSV: alignment_bits,size,kind,source_loc,compiler_pass,details

extern int use_pointer(void *);

int use_stack_var() {
  int x = 1;
  return use_pointer(&x);
  // CSV-NODEBUG-NEXT: 2,4,s,"<somewhere in use_stack_var>","CHERI bound stack allocations","set bounds on AllocaInst x"
  // CSV-DEBUG-NEXT:   2,4,s,"{{.+}}/csetbounds-stats-purecap-stack.c:[[@LINE-3]]","CHERI bound stack allocations","set bounds on local variable x"
}

// CSV-EMPTY:
