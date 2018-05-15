; REQUIRES: asserts
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=legacy -o - -thread-model=posix -mattr=-noabicalls -O2 -verify-machineinstrs %s | FileCheck %s

; llvm.cheri.stack.cap.get should only be defined in the purecap ABI (since c11 is not used as the stack pointer in hybrid mode)
; RUN: not %cheri_llc -o - -relocation-model=pic -thread-model=posix -mattr=-noabicalls \
; RUN:     -target-abi n64 -O2 -verify-machineinstrs %s 2>&1 | FileCheck %s -check-prefix BAD_INTRIN
; BAD_INTRIN: LLVM ERROR: Cannot select: intrinsic %llvm.cheri.stack.cap.get

@reserved_reg_target = external global i8 addrspace(200)*, align 32

; Function Attrs: noinline nounwind optnone
define void @test_fault_read_epcc() {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.epcc.get()
  store i8 addrspace(200)* %0, i8 addrspace(200)** @reserved_reg_target, align 32
  ; CHECK: creadhwr	[[EPCC:\$c[0-9]+]], $chwr_epcc
  ; CHECK: cjr	$c17
  ; CHECK: csc	[[EPCC]], $1, 0($c0)
  ret void
}
declare i8 addrspace(200)* @llvm.mips.epcc.get()

define i8 addrspace(200)* @getstack() nounwind {
entry:
  ; CHECK-LABEL: getstack:
  ; CHECK: 	cincoffset	$c3, $c11, $zero
  ; CHECK:	cjr	$c17
  ; CHECK:	csc	$c3, $1, 0($c0)
  %0 = call i8 addrspace(200)* @llvm.cheri.stack.cap.get()
  store i8 addrspace(200)* %0, i8 addrspace(200)** @reserved_reg_target, align 32
  ret i8 addrspace(200)* %0
}
declare i8 addrspace(200)* @llvm.cheri.stack.cap.get()


;define i32 @x86_intrin() nounwind {
;entry:
;  %0 = call i32 @llvm.x86.flags.read.u32()
;  ret i32 %0
;}
;declare i32 @llvm.x86.flags.read.u32()

