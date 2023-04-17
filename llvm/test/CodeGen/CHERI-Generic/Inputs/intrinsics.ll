; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - < %s | FileCheck %s --check-prefix=PURECAP
; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -o - < %s | FileCheck %s --check-prefix=HYBRID
; Check that the target-independent CHERI intrinsics are support for all architectures
; The grouping/ordering in this test is based on the RISC-V instruction listing
; in the CHERI ISA specification (Appendix C.1 in ISAv7).

; Capability-Inspection Instructions

declare iCAPRANGE @llvm.cheri.cap.perms.get.iCAPRANGE(i8 addrspace(200)*)
declare iCAPRANGE @llvm.cheri.cap.type.get.iCAPRANGE(i8 addrspace(200)*)
declare iCAPRANGE @llvm.cheri.cap.base.get.iCAPRANGE(i8 addrspace(200)*)
declare iCAPRANGE @llvm.cheri.cap.length.get.iCAPRANGE(i8 addrspace(200)*)
declare i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)*)
declare i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)*)
declare iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)*)
declare iCAPRANGE @llvm.cheri.cap.flags.get.iCAPRANGE(i8 addrspace(200)*)
declare iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(i8 addrspace(200)*)
declare iCAPRANGE @llvm.cheri.cap.high.get.iCAPRANGE(i8 addrspace(200)*)

define iCAPRANGE @perms_get(i8 addrspace(200)* %cap) nounwind {
  %perms = call iCAPRANGE @llvm.cheri.cap.perms.get.iCAPRANGE(i8 addrspace(200)* %cap)
  ret iCAPRANGE %perms
}

define iCAPRANGE @type_get(i8 addrspace(200)* %cap) nounwind {
  %type = call iCAPRANGE @llvm.cheri.cap.type.get.iCAPRANGE(i8 addrspace(200)* %cap)
  ret iCAPRANGE %type
}

define iCAPRANGE @base_get(i8 addrspace(200)* %cap) nounwind {
  %base = call iCAPRANGE @llvm.cheri.cap.base.get.iCAPRANGE(i8 addrspace(200)* %cap)
  ret iCAPRANGE %base
}

define iCAPRANGE @length_get(i8 addrspace(200)* %cap) nounwind {
  %length = call iCAPRANGE @llvm.cheri.cap.length.get.iCAPRANGE(i8 addrspace(200)* %cap)
  ret iCAPRANGE %length
}

define iCAPRANGE @tag_get(i8 addrspace(200)* %cap) nounwind {
  %tag = call i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)* %cap)
  %tag.zext = zext i1 %tag to iCAPRANGE
  ret iCAPRANGE %tag.zext
}

define iCAPRANGE @sealed_get(i8 addrspace(200)* %cap) nounwind {
  %sealed = call i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* %cap)
  %sealed.zext = zext i1 %sealed to iCAPRANGE
  ret iCAPRANGE %sealed.zext
}

define iCAPRANGE @offset_get(i8 addrspace(200)* %cap) nounwind {
  %offset = call iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)* %cap)
  ret iCAPRANGE %offset
}

define iCAPRANGE @flags_get(i8 addrspace(200)* %cap) nounwind {
  %flags = call iCAPRANGE @llvm.cheri.cap.flags.get.iCAPRANGE(i8 addrspace(200)* %cap)
  ret iCAPRANGE %flags
}

define iCAPRANGE @address_get(i8 addrspace(200)* %cap) nounwind {
  %address = call iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(i8 addrspace(200)* %cap)
  ret iCAPRANGE %address
}

@IF-MIPS@;; llvm.cheri.cap.high.get is not supported for MIPS - this could be added by
@IF-MIPS@;; spilling via memory but since CHERI-MIPS is EOL we skip this test instead.
@IFNOT-MIPS@define iCAPRANGE @high_get(i8 addrspace(200)* %cap) nounwind {
@IFNOT-MIPS@  %high = call iCAPRANGE @llvm.cheri.cap.high.get.iCAPRANGE(i8 addrspace(200)* %cap)
@IFNOT-MIPS@  ret iCAPRANGE %high
@IFNOT-MIPS@}

; Capability-Modification Instructions

declare i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.cap.flags.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.cap.address.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.cap.high.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.cap.tag.clear(i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.build(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.type.copy(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.conditional.seal(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.seal.entry(i8 addrspace(200)*)

define i8 addrspace(200)* @seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %sealed = call i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %sealed
}

define i8 addrspace(200)* @unseal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %unsealed = call i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %unsealed
}

define i8 addrspace(200)* @perms_and(i8 addrspace(200)* %cap, iCAPRANGE %perms) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.perms.and.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %perms)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @flags_set(i8 addrspace(200)* %cap, iCAPRANGE %flags) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.flags.set.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %flags)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @offset_set(i8 addrspace(200)* %cap, iCAPRANGE %offset) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %offset)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @address_set(i8 addrspace(200)* %cap, iCAPRANGE %address) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.address.set.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %address)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @bounds_set(i8 addrspace(200)* %cap, iCAPRANGE %bounds) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %bounds)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @bounds_set_exact(i8 addrspace(200)* %cap, iCAPRANGE %bounds) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %bounds)
  ret i8 addrspace(200)* %newcap
}

@IF-MIPS@;; llvm.cheri.cap.high.set is not supported for MIPS - this could be added by
@IF-MIPS@;; spilling via memory but since CHERI-MIPS is EOL we skip this test instead.
@IFNOT-MIPS@define i8 addrspace(200)* @high_set(i8 addrspace(200)* %cap, iCAPRANGE %high) nounwind {
@IFNOT-MIPS@  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.high.set.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %high)
@IFNOT-MIPS@  ret i8 addrspace(200)* %newcap
@IFNOT-MIPS@}

define i8 addrspace(200)* @bounds_set_immediate(i8 addrspace(200)* %cap) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE 42)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @tag_clear(i8 addrspace(200)* %cap) nounwind {
  %untagged = call i8 addrspace(200)* @llvm.cheri.cap.tag.clear(i8 addrspace(200)* %cap)
  ret i8 addrspace(200)* %untagged
}

define i8 addrspace(200)* @build(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %built = call i8 addrspace(200)* @llvm.cheri.cap.build(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %built
}

define i8 addrspace(200)* @type_copy(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.type.copy(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @conditional_seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.conditional.seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @seal_entry(i8 addrspace(200)* %cap) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.seal.entry(i8 addrspace(200)* %cap)
  ret i8 addrspace(200)* %newcap
}

; Pointer-Arithmetic Instructions

declare iCAPRANGE @llvm.cheri.cap.to.pointer(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)*, iCAPRANGE)
declare iCAPRANGE @llvm.cheri.cap.diff(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.ddc.get()
declare i8 addrspace(200)* @llvm.cheri.pcc.get()

define iCAPRANGE @to_pointer(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %ptr = call iCAPRANGE @llvm.cheri.cap.to.pointer(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret iCAPRANGE %ptr
}

define iCAPRANGE @to_pointer_ddc_relative(i8 addrspace(200)* %cap) nounwind {
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %ptr = call iCAPRANGE @llvm.cheri.cap.to.pointer(i8 addrspace(200)* %ddc, i8 addrspace(200)* %cap)
  ret iCAPRANGE %ptr
}

define i8 addrspace(200)* @from_pointer(i8 addrspace(200)* %cap, iCAPRANGE %ptr) nounwind {
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)* %cap, iCAPRANGE %ptr)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @from_ddc(iCAPRANGE %ptr) nounwind {
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %cap = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)* %ddc, iCAPRANGE %ptr)
  ret i8 addrspace(200)* %cap
}

define iCAPRANGE @diff(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %diff = call iCAPRANGE @llvm.cheri.cap.diff(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret iCAPRANGE %diff
}

define i8 addrspace(200)* @ddc_get() nounwind {
  %cap = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  ret i8 addrspace(200)* %cap
}

define i8 addrspace(200)* @pcc_get() nounwind {
  %cap = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  ret i8 addrspace(200)* %cap
}

; Assertion Instructions

declare i1 @llvm.cheri.cap.subset.test(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)

define iCAPRANGE @subset_test(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %subset = call i1 @llvm.cheri.cap.subset.test(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  %subset.zext = zext i1 %subset to iCAPRANGE
  ret iCAPRANGE %subset.zext
}

declare i1 @llvm.cheri.cap.equal.exact(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)

define iCAPRANGE @equal_exact(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
  %eqex = call i1 @llvm.cheri.cap.equal.exact(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  %eqex.zext = zext i1 %eqex to iCAPRANGE
  ret iCAPRANGE %eqex.zext
}
