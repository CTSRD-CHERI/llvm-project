// REQUIRES: riscv-registered-target
// Some projects (e.g. Linux kernel) like to use global register variables, check
// that we can handle the naive port from "tp" -> "ctp"
// See e.g. https://elixir.bootlin.com/linux/v5.8.10/source/arch/riscv/include/asm/current.h
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm %s -o - | FileCheck %s
// RUN: %riscv64_cheri_cc1 -emit-llvm %s -o -  | FileCheck %s
// CRASHES-RUN-NOT: %riscv64_cheri_purecap_cc1 -S %s -o - | FileCheck %s  --check-prefix PURECAP-ASM
// CRASHES-RUN-NOT: %riscv64_cheri_cc1 -S %s -o - | FileCheck %s --check-prefix HYBRID-ASM

register long int_tp __asm__("tp");
// CHECK-LABEL: define {{[^@]+}}@get_tp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i64 @llvm.read_register.i64(metadata !0)
// CHECK-NEXT:    ret i64 [[TMP0]]
long get_tp(void) {
  return int_tp;
}

register __uintcap_t cap_tp __asm__("ctp");
// CHECK-LABEL: define {{[^@]+}}@get_ctp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i64 @llvm.read_register.i64(metadata !1)
// CHECK-NEXT:    [[TMP1:%.*]] = inttoptr i64 [[TMP0]] to i8 addrspace(200)*
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP1]]
__uintcap_t get_ctp(void) {
  return cap_tp;
}

// CHECK: !llvm.named.register.tp = !{!0}
// CHECK: !llvm.named.register.ctp = !{!1}
// CHECK: !0 = !{!"tp"}
// CHECK: !1 = !{!"ctp"}
