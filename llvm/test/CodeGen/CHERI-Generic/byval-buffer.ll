; REQUIRES: mips-registered-target, riscv-registered-target
; After merging to the LLVM11 branch point certain by-value arguments were no longer being copied.
; Changing the IR to use addrspace(0) and compiling for non-CHERI adds memcpy() calls, so we should do the same for purecap.
; This (incorrect) behaviour was added in a297402637b73b9d783a229e9b41c57ea7b92e2e, but we apparently never hit
; it because the frontend would almost always emit a local alloca already.

; First check that non-CHERI targets add a memcpy
; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | sed 's/p200i8/p0i8/g' > %t-nocheri.ll
; RUN: llc -mtriple riscv64-unknown-freebsd -o - -relocation-model=static < %t-nocheri.ll | FileCheck %s --check-prefixes CHECK,RV64,RV64-STATIC
; RUN: llc -mtriple riscv64-unknown-freebsd -o - -relocation-model=pic < %t-nocheri.ll | FileCheck %s --check-prefixes CHECK,RV64,RV64-PIC
; RUN: llc -mtriple mips64-unknown-freebsd -o - -relocation-model=pic  < %t-nocheri.ll | FileCheck %s --check-prefixes CHECK,MIPS,MIPS-PIC
; Next check purecap targets:
; RUN: %riscv64_cheri_purecap_llc -o - %s | FileCheck %s --check-prefixes CHECK,PURECAP-RV64
; RUN: %cheri_purecap_llc -o - %s | FileCheck %s --check-prefixes CHECK,PURECAP-MIPS

; IR originally generated from %cheri_purecap_cc1 -mrelocation-model pic -pic-level 1 -pic-is-pie -O -S %s -o - -emit-llvm
; Interestingly, the purecap RISCV IR contains a temporary alloca and MIPS also does without the pic flags (CodeGenFunction::EmitCall()).
; struct foo {
;   char data[1024];
; };
;
; extern void assert_eq(long l1, long l2);
; extern void *memset(void *, int, unsigned long);
; struct foo global_foo;
; __attribute__((noinline)) void foo_byval(struct foo f) {
;   memset(f.data, 42, sizeof(f.data));
; }
;
; void clang_purecap_byval_args(void) {
;   /* Check that structs passed by value are actually copied */
;   memset(global_foo.data, 0, sizeof(global_foo.data));
;   assert_eq(global_foo.data[0], 0);
;   foo_byval(global_foo);
;   assert_eq(global_foo.data[0], 0);
; }

%struct.foo = type { [1024 x i8] }

@global_foo = dso_local local_unnamed_addr addrspace(200) global %struct.foo zeroinitializer, align 8

declare dso_local void @foo_byval(%struct.foo addrspace(200)* nocapture byval(%struct.foo) align 8 %f) local_unnamed_addr addrspace(200) noinline nounwind writeonly
; define dso_local void @foo_byval(%struct.foo addrspace(200)* nocapture byval(%struct.foo) align 8 %f) local_unnamed_addr addrspace(200) noinline nounwind writeonly {
; entry:
;   %arraydecay = getelementptr inbounds %struct.foo, %struct.foo addrspace(200)* %f, i64 0, i32 0, i64 0
;   call void @llvm.memset.p200i8.i64(i8 addrspace(200)* nonnull align 8 dereferenceable(1024) %arraydecay, i8 42, i64 1024, i1 false)
;   ret void
; }

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8, i64, i1 immarg) addrspace(200) argmemonly nounwind willreturn writeonly

; Function Attrs: nounwind uwtable
define dso_local void @clang_purecap_byval_args() local_unnamed_addr addrspace(200) nounwind {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* nonnull align 8 dereferenceable(1024) getelementptr inbounds (%struct.foo, %struct.foo addrspace(200)* @global_foo, i64 0, i32 0, i64 0), i8 0, i64 1024, i1 false)
  %0 = load i8, i8 addrspace(200)* getelementptr inbounds (%struct.foo, %struct.foo addrspace(200)* @global_foo, i64 0, i32 0, i64 0), align 8
  %conv = sext i8 %0 to i64
  call void @assert_eq(i64 signext %conv, i64 signext 0) nounwind
  call void @foo_byval(%struct.foo addrspace(200)* nonnull byval(%struct.foo) align 8 @global_foo)
  %1 = load i8, i8 addrspace(200)* getelementptr inbounds (%struct.foo, %struct.foo addrspace(200)* @global_foo, i64 0, i32 0, i64 0), align 8
  %conv1 = sext i8 %1 to i64
  call void @assert_eq(i64 signext %conv1, i64 signext 0) nounwind
  ret void

  ; CHECK-LABEL: clang_purecap_byval_args:

  ; RV64-STATIC: addi s0, s2, %lo(global_foo)
  ; RV64-PIC: auipc s0, %pcrel_hi(global_foo)
  ; RV64-PIC: addi s0, s0, %pcrel_lo(.LBB
  ; RV64: call memset{{(@plt)?}}
  ; RV64-STATIC: lb a0, %lo(global_foo)(s2)
  ; RV64-PIC: lb a0, 0(s0)
  ; RV64: call assert_eq{{(@plt)?}}
  ; s0 contains &global_foo -> memcpy src
  ; RV64: addi a2, zero, 1024
  ; RV64: mv a1, s0
  ; RV64: call memcpy{{(@plt)?}}
  ; RV64: call foo_byval
  ; RV64-STATIC: lb a0, %lo(global_foo)(s2)
  ; RV64-PIC: lb a0, 0(s0)
  ; RV64: call assert_eq{{(@plt)?}}

  ; PURECAP-RV64: cincoffset csp, csp, -1104
  ; PURECAP-RV64: auipcc c{{[a-z0-9]+}}, %captab_pcrel_hi(global_foo)
  ; PURECAP-RV64-NEXT: clc cs0, %pcrel_lo(.LBB{{.+}})(c{{[a-z0-9]+}})
  ; PURECAP-RV64: auipcc c{{[a-z0-9]+}}, %captab_pcrel_hi(memset)
  ; PURECAP-RV64: clb a0, 0(cs0)
  ; PURECAP-RV64: auipcc c{{[a-z0-9]+}}, %captab_pcrel_hi(assert_eq)
  ; PURECAP-RV64: clc [[ASSERT_EQ_CAP:cs[0-9]]], %pcrel_lo(.LBB{{.+}})(c{{[a-z0-9]+}})
  ; PURECAP-RV64: auipcc [[MEMCPY_CAP:c[a-z0-9]+]], %captab_pcrel_hi(memcpy)
  ; PURECAP-RV64: cincoffset cs1, csp, 16
  ; PURECAP-RV64-NEXT: addi a2, zero, 1024
  ; PURECAP-RV64-NEXT: cmove ca0, cs1
  ; PURECAP-RV64-NEXT: cmove ca1, cs0
  ; PURECAP-RV64-NEXT: cjalr [[MEMCPY_CAP]]
  ; PURECAP-RV64: auipcc c{{[a-z0-9]+}}, %captab_pcrel_hi(foo_byval)
  ; PURECAP-RV64: cmove ca0, cs1
  ; PURECAP-RV64: clb a0, 0(cs0)
  ; PURECAP-RV64: cjalr [[ASSERT_EQ_CAP]]

  ; MIPS-PIC: ld $16, %got_disp(global_foo)($gp)
  ; MIPS-PIC: ld $25, %call16(memset)($gp)
  ; MIPS-PIC: lb $4, 0($16)
  ; MIPS-PIC: ld $25, %call16(assert_eq)($gp)
  ; MIPS-PIC: ld $25, %call16(memcpy)($gp)
  ; Note: MIPS passes the first 64 byval bytes in registers
  ; MIPS-PIC: daddiu $6, $zero, 960
  ; MIPS-PIC: ld $25, %call16(foo_byval)($gp)
  ; MIPS-PIC: lb $4, 0($16)
  ; MIPS-PIC: ld $25, %call16(assert_eq)($gp)

  ; PURECAP-MIPS: daddiu $1, $zero, -1104
  ; PURECAP-MIPS: cincoffset $c11, $c11, $1
  ; PURECAP-MIPS: clcbi $c18, %captab20(global_foo)($c20)
  ; PURECAP-MIPS: clcbi $c12, %capcall20(memset)($c20)
  ; PURECAP-MIPS: clb $4, $zero, 0($c18)
  ; PURECAP-MIPS: clcbi $c12, %capcall20(assert_eq)($c20)
  ; PURECAP-MIPS: clcbi $c12, %capcall20(memcpy)($c20)
  ; PURECAP-MIPS: cmove $c19, $c11
  ; PURECAP-MIPS: cmove $c3, $c19
  ; PURECAP-MIPS: cmove $c4, $c18
  ; PURECAP-MIPS: csetbounds $c1, $c19, 1024
  ; PURECAP-MIPS: clcbi $c12, %capcall20(foo_byval)($c20)
  ; PURECAP-MIPS: candperm $c13, $c1, $1
  ; PURECAP-MIPS: clb $4, $zero, 0($c18)
  ; PURECAP-MIPS: clcbi $c12, %capcall20(assert_eq)($c20)

}

declare void @assert_eq(i64 signext, i64 signext) local_unnamed_addr addrspace(200) nounwind

