// REQUIRES: riscv-registered-target
// Some projects (e.g. Linux kernel) like to use global register variables, check
// that we can handle the naive port from "tp" -> "ctp"
// See e.g. https://elixir.bootlin.com/linux/v5.8.10/source/arch/riscv/include/asm/current.h
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm %s -o - | FileCheck %s
// RUN: %riscv64_cheri_cc1 -emit-llvm %s -o -  | FileCheck %s
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm %s -o -
// RUN: %riscv64_cheri_purecap_cc1 -S %s -o - | FileCheck %s  --check-prefixes ASM,PURECAP-ASM
// RUN: %riscv64_cheri_cc1 -S %s -o - | FileCheck %s --check-prefixes ASM,HYBRID-ASM

register long int_tp __asm__("tp");
// CHECK-LABEL: define {{[^@]+}}@get_tp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i64 @llvm.read_register.i64(metadata !0)
// CHECK-NEXT:    ret i64 [[TMP0]]
// ASM-LABEL:        get_tp:
// ASM:              mv a0, tp
// PURECAP-ASM-NEXT: cret
// HYBRID-ASM-NEXT:  ret
long get_tp(void) {
  return int_tp;
}

register __uintcap_t cap_tp __asm__("ctp");
// CHECK-LABEL: define {{[^@]+}}@get_ctp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i8 addrspace(200)* @llvm.read_register.p200i8(metadata !1)
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
// ASM-LABEL:        get_ctp:
// ASM:              cmove ca0, ctp
// PURECAP-ASM-NEXT: cret
// HYBRID-ASM-NEXT:  ret
__uintcap_t get_ctp(void) {
  return cap_tp;
}

// Check that we handle casts to other pointer types correctly
struct StackPtr;
register struct StackPtr *__capability cap_sp __asm__("csp");
// CHECK-LABEL: define {{[^@]+}}@get_csp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i8 addrspace(200)* @llvm.read_register.p200i8(metadata !2)
// CHECK-NEXT:    [[TMP1:%.*]] = bitcast i8 addrspace(200)* [[TMP0]] to %struct.StackPtr addrspace(200)*
// CHECK-NEXT:    ret %struct.StackPtr addrspace(200)* [[TMP1]]
// ASM-LABEL:        get_csp:
// ASM:              cmove ca0, csp
// PURECAP-ASM-NEXT: cret
// HYBRID-ASM-NEXT:  ret
struct StackPtr *__capability get_csp(void) {
  return cap_sp;
}

// Also check conversions to integral types
register __uintcap_t cap_gp __asm__("cgp");
// CHECK-LABEL: define {{[^@]+}}@get_gp_addr()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i8 addrspace(200)* @llvm.read_register.p200i8(metadata !3)
// CHECK-NEXT:    [[TMP1:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* [[TMP0]])
// CHECK-NEXT:    ret i64 [[TMP1]]
// ASM-LABEL:        get_gp_addr:
// ASM:              cmove ca0, cgp
// ASM-NEXT:         cgetaddr a0, ca0
// PURECAP-ASM-NEXT: cret
// HYBRID-ASM-NEXT:  ret
long get_gp_addr(void) {
  return cap_gp;
}

// CHECK: !llvm.named.register.tp = !{!0}
// CHECK: !llvm.named.register.ctp = !{!1}
// CHECK: !llvm.named.register.csp = !{!2}
// CHECK: !llvm.named.register.cgp = !{!3}
// CHECK: !0 = !{!"tp"}
// CHECK: !1 = !{!"ctp"}
// CHECK: !2 = !{!"csp"}
// CHECK: !3 = !{!"cgp"}
