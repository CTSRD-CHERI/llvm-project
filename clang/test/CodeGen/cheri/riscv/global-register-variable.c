// Please also update llvm/test/CodeGen/RISCV/cheri/read_write_register.ll when changing this test
// RUN_TO_UPDATE_IR_TO_ASM_TEST: %riscv64_cheri_cc1 -emit-llvm %s -O2 -o %S/../../../../../llvm/test/CodeGen/RISCV/cheri/read_write_register.ll.template
// Some projects (e.g. Linux kernel) like to use global register variables, check
// that we can handle the naive port from "tp" -> "ctp"
// See e.g. https://elixir.bootlin.com/linux/v5.8.10/source/arch/riscv/include/asm/current.h
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only %s -o - -verify -DCHECK_BAD
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm %s -o - | FileCheck %s
// RUN: %riscv64_cheri_cc1 -emit-llvm %s -o -  | FileCheck %s

register long int_tp __asm__("tp");
// CHECK-LABEL: define {{[^@]+}}@get_tp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i64 @llvm.read_register.i64(metadata !0)
// CHECK-NEXT:    ret i64 [[TMP0]]
long get_tp(void) {
  return int_tp;
}

// CHECK-LABEL: define {{[^@]+}}@set_tp
// CHECK-SAME: (i64 noundef [[VALUE:%[a-z0-9]+]])
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[VALUE_ADDR:%.*]] = alloca i64, align 8
// CHECK-NEXT:    store i64 [[VALUE]], i64{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 8
// CHECK-NEXT:    [[TMP0:%.*]] = load i64, i64{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 8
// CHECK-NEXT:    call void @llvm.write_register.i64(metadata !0, i64 [[TMP0]])
// CHECK-NEXT:    ret void
//
void set_tp(long value) {
  int_tp = value;
}

register __uintcap_t cap_tp __asm__("ctp");
// CHECK-LABEL: define {{[^@]+}}@get_ctp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i8 addrspace(200)* @llvm.read_register.p200i8(metadata !1)
// CHECK-NEXT:    ret i8 addrspace(200)* [[TMP0]]
__uintcap_t get_ctp(void) {
  return cap_tp;
}
// CHECK-LABEL: define {{[^@]+}}@set_ctp
// CHECK-SAME: (i8 addrspace(200)* noundef [[VALUE:%[a-z0-9]+]])
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[VALUE_ADDR:%.*]] = alloca i8 addrspace(200)*, align 16
// CHECK-NEXT:    store i8 addrspace(200)* [[VALUE]], i8 addrspace(200)*{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 16
// CHECK-NEXT:    [[TMP0:%.*]] = load i8 addrspace(200)*, i8 addrspace(200)*{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 16
// CHECK-NEXT:    call void @llvm.write_register.p200i8(metadata !1, i8 addrspace(200)* [[TMP0]])
// CHECK-NEXT:    ret void
//
// FIXME: This looks wrong, it does not actually set ctp
void set_ctp(__uintcap_t value) {
  cap_tp = value;
}

// Check that we handle casts to other pointer types correctly
struct StackPtr;
register struct StackPtr *__capability cap_sp __asm__("csp");
// CHECK-LABEL: define {{[^@]+}}@get_csp()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i8 addrspace(200)* @llvm.read_register.p200i8(metadata !2)
// CHECK-NEXT:    [[TMP1:%.*]] = bitcast i8 addrspace(200)* [[TMP0]] to %struct.StackPtr addrspace(200)*
// CHECK-NEXT:    ret %struct.StackPtr addrspace(200)* [[TMP1]]
struct StackPtr *__capability get_csp(void) {
  return cap_sp;
}
// CHECK-LABEL: define {{[^@]+}}@set_csp
// CHECK-SAME: (%struct.StackPtr addrspace(200)* noundef [[VALUE:%[a-z0-9]+]])
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[VALUE_ADDR:%.*]] = alloca [[STRUCT_STACKPTR:%.*]] addrspace(200)*, align 16
// CHECK-NEXT:    store [[STRUCT_STACKPTR]] addrspace(200)* [[VALUE]], [[STRUCT_STACKPTR]] addrspace(200)*{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 16
// CHECK-NEXT:    [[TMP0:%.*]] = load [[STRUCT_STACKPTR]] addrspace(200)*, [[STRUCT_STACKPTR]] addrspace(200)*{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 16
// CHECK-NEXT:    [[TMP1:%.*]] = bitcast [[STRUCT_STACKPTR]] addrspace(200)* [[TMP0]] to i8 addrspace(200)*
// CHECK-NEXT:    call void @llvm.write_register.p200i8(metadata !2, i8 addrspace(200)* [[TMP1]])
// CHECK-NEXT:    ret void
void set_csp(struct StackPtr *__capability value) {
  cap_sp = value;
}

// Also check conversions to integral types
register __uintcap_t cap_gp __asm__("cgp");
// CHECK-LABEL: define {{[^@]+}}@get_gp_addr()
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i8 addrspace(200)* @llvm.read_register.p200i8(metadata !3)
// CHECK-NEXT:    [[TMP1:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* [[TMP0]])
// CHECK-NEXT:    ret i64 [[TMP1]]
long get_gp_addr(void) {
  return cap_gp;
}

// CHECK-LABEL: define {{[^@]+}}@set_gp_addr
// CHECK-SAME: (i64 noundef [[VALUE:%[a-z0-9]+]])
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[VALUE_ADDR:%.*]] = alloca i64, align 8
// CHECK-NEXT:    store i64 [[VALUE]], i64{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 8
// CHECK-NEXT:    [[TMP0:%.*]] = load i64, i64{{( addrspace\(200\))?}}* [[VALUE_ADDR]], align 8
// CHECK-NEXT:    [[TMP1:%.*]] = getelementptr i8, i8 addrspace(200)* null, i64 [[TMP0]]
// CHECK-NEXT:    call void @llvm.write_register.p200i8(metadata !3, i8 addrspace(200)* [[TMP1]])
// CHECK-NEXT:    ret void
void set_gp_addr(long value) {
  cap_gp = value;
}

#ifdef CHECK_BAD
register int cnull_bad1 __asm__("cnull");
int get_cnull_bad1(void) { return cnull_bad1; }
register long cnull_bad2 __asm__("cnull");
long get_cnull_bad2(void) { return cnull_bad2; }
register float cnull_bad3 __asm__("cnull"); // expected-error{{bad type for named register variable}}
float get_cnull_bad3(void) { return cnull_bad3; }
// FIXME: Should warn about this and suggest using ctp/emit an error
register __uintcap_t bad_tp __asm__("tp");
__uintcap_t get_tp_bad(void) { return bad_tp; }
#endif
// CHECK: !llvm.named.register.tp = !{!0}
// CHECK: !llvm.named.register.ctp = !{!1}
// CHECK: !llvm.named.register.csp = !{!2}
// CHECK: !llvm.named.register.cgp = !{!3}
// CHECK: !0 = !{!"tp"}
// CHECK: !1 = !{!"ctp"}
// CHECK: !2 = !{!"csp"}
// CHECK: !3 = !{!"cgp"}
