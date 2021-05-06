; This used to create a broken function.
; FIXME: the getoffset+add sequence should be folded to an increment
; REQUIRES: mips-registered-target
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -S -instcombine -O3 %s -o - | FileCheck %s
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -S -O3 %s | llc @PURECAP_HARDFLOAT_ARGS@ -O3 -o - | FileCheck %s --check-prefix ASM
target datalayout = "@PURECAP_DATALAYOUT@"

@d = common addrspace(200) global iCAPRANGE 0, align 4
@e = common addrspace(200) global i8 addrspace(200)* null, align 32

; C Source code:
;int d;
;void* e;
;void g(int x, int y) {
;  e = (__uintcap_t)&d + x + y;
;}

define void @g(iCAPRANGE %x, iCAPRANGE %y) addrspace(200) nounwind {
entry:
  %x.addr = alloca iCAPRANGE, align 4, addrspace(200)
  %y.addr = alloca iCAPRANGE, align 4, addrspace(200)
  store iCAPRANGE %x, iCAPRANGE addrspace(200)* %x.addr, align 4
  store iCAPRANGE %y, iCAPRANGE addrspace(200)* %y.addr, align 4
  %tmp1 = load iCAPRANGE, iCAPRANGE addrspace(200)* %x.addr, align 4
  %tmp2 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* null, iCAPRANGE %tmp1)
  %tmp3 = call iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)* bitcast (iCAPRANGE addrspace(200)* @d to i8 addrspace(200)*))
  %tmp4 = call iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)* %tmp2)
  %add = add iCAPRANGE %tmp3, %tmp4
  %tmp5 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* bitcast (iCAPRANGE addrspace(200)* @d to i8 addrspace(200)*), iCAPRANGE %add)
  %tmp7 = load iCAPRANGE, iCAPRANGE addrspace(200)* %y.addr, align 4
  %tmp8 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* null, iCAPRANGE %tmp7)
  %tmp9 = call iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)* %tmp5)
  %tmp10 = call iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)* %tmp8)
  %add1 = add iCAPRANGE %tmp9, %tmp10
  %tmp11 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* %tmp5, iCAPRANGE %add1)
  store i8 addrspace(200)* %tmp11, i8 addrspace(200)* addrspace(200)* @e, align 32
  ret void
}

; define void @g(iCAPRANGE %x, iCAPRANGE %y)  nounwind {
; entry:
;   %tmp1 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.iCAPRANGE(i8 addrspace(200)* bitcast (iCAPRANGE addrspace(200)* @d to i8 addrspace(200)*), iCAPRANGE %x)
;   %tmp3 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.iCAPRANGE(i8 addrspace(200)* %tmp1, iCAPRANGE %y)
;   store i8 addrspace(200)* %tmp3, i8 addrspace(200)* addrspace(200)* @e, align 32
;   ret void
; }
;
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE) addrspace(200)
declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE) addrspace(200)
declare iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)*) addrspace(200)
