// Previously clang would ignore always_inline at -O0
// REQUIRES: asserts

// CHeck that -O0 inlines only the always_inline function
// RUN: %cheri_cc1 -emit-llvm %s -O0 -o - | FileCheck %s -check-prefixes CHECK,NOOPT,N64,N64-NOOPT
// RUN: %cheri_purecap_cc1 -emit-llvm -mllvm -cheri-cap-table-abi=pcrel %s -O0 -o - | FileCheck %s -check-prefixes CHECK,NOOPT,PURECAP,PURECAP-CAPTABLE-NOOPT,PURECAP-CAPTABLE
// RUN: %cheri_purecap_cc1 -emit-llvm -mllvm -cheri-cap-table-abi=legacy %s -O0 -o - | FileCheck %s -check-prefixes CHECK,NOOPT,PURECAP,PURECAP-LEGACY-NOOPT,PURECAP-LEGACY

// at -O1/O2 the maybe_inline function should be inlined
// RUN: %cheri_cc1 -emit-llvm %s -O1 -o - | FileCheck %s -check-prefixes CHECK,OPT,N64
// RUN: %cheri_cc1 -emit-llvm %s -O2 -o - | FileCheck %s -check-prefixes CHECK,OPT,N64
// RUN: %cheri_purecap_cc1 -emit-llvm -mllvm -cheri-cap-table-abi=pcrel %s -O2 -o - | FileCheck %s -check-prefixes CHECK,OPT,PURECAP,PURECAP-CAPTABLE
// RUN: %cheri_purecap_cc1 -emit-llvm -mllvm -cheri-cap-table-abi=legacy %s -O2 -o - | FileCheck %s -check-prefixes CHECK,OPT,PURECAP,PURECAP-LEGACY
// RUN: %cheri_purecap_cc1 -emit-llvm -mllvm -cheri-cap-table-abi=pcrel %s -O1 -o - | FileCheck %s -check-prefixes CHECK,OPT,PURECAP,PURECAP-CAPTABLE
// At -O1 the CHERI legacy calls are not inlined:
// RUsN: %cheri_purecap_cc1 -emit-llvm -mllvm -cheri-cap-table-abi=legacy %s -O1 -o - | FileCheck %s -check-prefixes CHECK,OPT,PURECAP,PURECAP-LEGACY

static __attribute__((always_inline)) int always_inlined(void) {
  return 5;
}

int call_always_inline(void) {
  return always_inlined();
}

// PURECAP: target datalayout =
// all globals+functions+allocas should be in AS200
// PURECAP-SAME: A200-P200
// PURECAP-LEGACY-NOT: -G200
// PURECAP-CAPTABLE-SAME: -G200

// CHECK-NOT: always_inlined
// This function must be inlined even at -O0
// CHECK-LABEL: define i32 @call_always_inline()
// CHECK-NEXT: entry:
// CHECK-NEXT:   ret i32 5
// CHECK-NEXT: }
// CHECK-NOT: always_inlined

int __attribute__((noinline)) not_inlined(int x) {
  return x + 6;
}

int call_not_inlined(void) {
  return not_inlined(0);
}

// This function must be inlined even at -O0
// CHECK-LABEL: define i32 @call_not_inlined()
// CHECK-NEXT: entry:
// OPT-NEXT:                    [[CALL:%.+]] = {{(tail )?}}call i32 @not_inlined(i32 signext 0)
// N64-NOOPT-NEXT:              [[CALL:%.+]] = {{(tail )?}}call i32 @not_inlined(i32 signext 0)
// PURECAP-LEGACY-NOOPT-NEXT:   [[CALL:%.+]] = {{(tail )?}}call i32 @not_inlined(i32 signext 0)
// PURECAP-CAPTABLE-NOOPT-NEXT: [[CALL:%.+]] = {{(tail )?}}call i32 @not_inlined(i32 signext 0)
// CHECK-NEXT:    ret i32 [[CALL]]
// CHECK-NEXT: }

static int maybe_inlined(int x) {
  return x + 7;
}

int call_maybe_inlined(void) {
  return maybe_inlined(0);
}

// OPT-NOT: maybe_inlined
// This function must be inlined even at -O0
// CHECK-LABEL: define i32 @call_maybe_inlined()
// CHECK-NEXT: entry:
// OPT-NEXT:     ret i32 7
// N64-NOOPT-NEXT:              [[CALL:%.+]] = call i32 @maybe_inlined(i32 signext 0)
// PURECAP-LEGACY-NOOPT-NEXT:   [[CALL:%.+]] = call i32 @maybe_inlined(i32 signext 0)
// PURECAP-CAPTABLE-NOOPT-NEXT: [[CALL:%.+]] = call i32 @maybe_inlined(i32 signext 0)
// NOOPT-NEXT:   ret i32 [[CALL]]
// CHECK-NEXT: }
// OPT-NOT: maybe_inlined
