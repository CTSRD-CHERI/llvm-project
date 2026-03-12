; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - < %s | FileCheck %s --check-prefix=PURECAP
; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -o - < %s | FileCheck %s --check-prefix=HYBRID
; Check that the target-independent CHERI intrinsics are support for all architectures
; The grouping/ordering in this test is based on the RISC-V instruction listing
; in the CHERI ISA specification (Appendix C.1 in ISAv7).

; Capability-Inspection Instructions

declare iCAPRANGE @llvm.cheri.cap.perms.get.iCAPRANGE(ptr addrspace(200))
declare iCAPRANGE @llvm.cheri.cap.type.get.iCAPRANGE(ptr addrspace(200))
declare iCAPRANGE @llvm.cheri.cap.base.get.iCAPRANGE(ptr addrspace(200))
declare iCAPRANGE @llvm.cheri.cap.length.get.iCAPRANGE(ptr addrspace(200))
declare i1 @llvm.cheri.cap.tag.get(ptr addrspace(200))
declare i1 @llvm.cheri.cap.sealed.get(ptr addrspace(200))
declare iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(ptr addrspace(200))
declare iCAPRANGE @llvm.cheri.cap.flags.get.iCAPRANGE(ptr addrspace(200))
declare iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(ptr addrspace(200))
declare iCAPRANGE @llvm.cheri.cap.high.get.iCAPRANGE(ptr addrspace(200))

define iCAPRANGE @perms_get(ptr addrspace(200) %cap) nounwind {
  %perms = call iCAPRANGE @llvm.cheri.cap.perms.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %perms
}

define iCAPRANGE @type_get(ptr addrspace(200) %cap) nounwind {
  %type = call iCAPRANGE @llvm.cheri.cap.type.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %type
}

define iCAPRANGE @base_get(ptr addrspace(200) %cap) nounwind {
  %base = call iCAPRANGE @llvm.cheri.cap.base.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %base
}

define iCAPRANGE @length_get(ptr addrspace(200) %cap) nounwind {
  %length = call iCAPRANGE @llvm.cheri.cap.length.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %length
}

define iCAPRANGE @tag_get(ptr addrspace(200) %cap) nounwind {
  %tag = call i1 @llvm.cheri.cap.tag.get(ptr addrspace(200) %cap)
  %tag.zext = zext i1 %tag to iCAPRANGE
  ret iCAPRANGE %tag.zext
}

define iCAPRANGE @sealed_get(ptr addrspace(200) %cap) nounwind {
  %sealed = call i1 @llvm.cheri.cap.sealed.get(ptr addrspace(200) %cap)
  %sealed.zext = zext i1 %sealed to iCAPRANGE
  ret iCAPRANGE %sealed.zext
}

define iCAPRANGE @offset_get(ptr addrspace(200) %cap) nounwind {
  %offset = call iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %offset
}

define iCAPRANGE @flags_get(ptr addrspace(200) %cap) nounwind {
  %flags = call iCAPRANGE @llvm.cheri.cap.flags.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %flags
}

define iCAPRANGE @address_get(ptr addrspace(200) %cap) nounwind {
  %address = call iCAPRANGE @llvm.cheri.cap.address.get.iCAPRANGE(ptr addrspace(200) %cap)
  ret iCAPRANGE %address
}

@IF-MIPS@;; llvm.cheri.cap.high.get is not supported for MIPS - this could be added by
@IF-MIPS@;; spilling via memory but since CHERI-MIPS is EOL we skip this test instead.
@IFNOT-MIPS@define iCAPRANGE @high_get(ptr addrspace(200) %cap) nounwind {
@IFNOT-MIPS@  %high = call iCAPRANGE @llvm.cheri.cap.high.get.iCAPRANGE(ptr addrspace(200) %cap)
@IFNOT-MIPS@  ret iCAPRANGE %high
@IFNOT-MIPS@}

; Capability-Modification Instructions

declare ptr addrspace(200) @llvm.cheri.cap.seal(ptr addrspace(200), ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.unseal(ptr addrspace(200), ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.perms.and.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.cap.flags.set.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.cap.offset.set.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.cap.address.set.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.cap.bounds.set.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.cap.bounds.set.exact.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.cap.high.set.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.cap.tag.clear(ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.build(ptr addrspace(200), ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.type.copy(ptr addrspace(200), ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.conditional.seal(ptr addrspace(200), ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.seal.entry(ptr addrspace(200))

define ptr addrspace(200) @seal(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %sealed = call ptr addrspace(200) @llvm.cheri.cap.seal(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  ret ptr addrspace(200) %sealed
}

define ptr addrspace(200) @unseal(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %unsealed = call ptr addrspace(200) @llvm.cheri.cap.unseal(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  ret ptr addrspace(200) %unsealed
}

define ptr addrspace(200) @perms_and(ptr addrspace(200) %cap, iCAPRANGE %perms) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.perms.and.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %perms)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @flags_set(ptr addrspace(200) %cap, iCAPRANGE %flags) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.flags.set.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %flags)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @offset_set(ptr addrspace(200) %cap, iCAPRANGE %offset) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.offset.set.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %offset)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @address_set(ptr addrspace(200) %cap, iCAPRANGE %address) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.address.set.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %address)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @bounds_set(ptr addrspace(200) %cap, iCAPRANGE %bounds) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %bounds)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @bounds_set_exact(ptr addrspace(200) %cap, iCAPRANGE %bounds) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.exact.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %bounds)
  ret ptr addrspace(200) %newcap
}

@IF-MIPS@;; llvm.cheri.cap.high.set is not supported for MIPS - this could be added by
@IF-MIPS@;; spilling via memory but since CHERI-MIPS is EOL we skip this test instead.
@IFNOT-MIPS@define ptr addrspace(200) @high_set(ptr addrspace(200) %cap, iCAPRANGE %high) nounwind {
@IFNOT-MIPS@  %newcap = call ptr addrspace(200) @llvm.cheri.cap.high.set.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %high)
@IFNOT-MIPS@  ret ptr addrspace(200) %newcap
@IFNOT-MIPS@}

define ptr addrspace(200) @bounds_set_immediate(ptr addrspace(200) %cap) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE 42)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @tag_clear(ptr addrspace(200) %cap) nounwind {
  %untagged = call ptr addrspace(200) @llvm.cheri.cap.tag.clear(ptr addrspace(200) %cap)
  ret ptr addrspace(200) %untagged
}

define ptr addrspace(200) @build(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %built = call ptr addrspace(200) @llvm.cheri.cap.build(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  ret ptr addrspace(200) %built
}

define ptr addrspace(200) @type_copy(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.type.copy(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @conditional_seal(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.conditional.seal(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @seal_entry(ptr addrspace(200) %cap) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.seal.entry(ptr addrspace(200) %cap)
  ret ptr addrspace(200) %newcap
}

; Pointer-Arithmetic Instructions

declare iCAPRANGE @llvm.cheri.cap.to.pointer(ptr addrspace(200), ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.cap.from.pointer(ptr addrspace(200), iCAPRANGE)
declare iCAPRANGE @llvm.cheri.cap.diff(ptr addrspace(200), ptr addrspace(200))
declare ptr addrspace(200) @llvm.cheri.ddc.get()
declare ptr addrspace(200) @llvm.cheri.pcc.get()

define iCAPRANGE @to_pointer(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %ptr = call iCAPRANGE @llvm.cheri.cap.to.pointer(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  ret iCAPRANGE %ptr
}

define iCAPRANGE @to_pointer_ddc_relative(ptr addrspace(200) %cap) nounwind {
  %ddc = call ptr addrspace(200) @llvm.cheri.ddc.get()
  %ptr = call iCAPRANGE @llvm.cheri.cap.to.pointer(ptr addrspace(200) %ddc, ptr addrspace(200) %cap)
  ret iCAPRANGE %ptr
}

define ptr addrspace(200) @from_pointer(ptr addrspace(200) %cap, iCAPRANGE %ptr) nounwind {
  %newcap = call ptr addrspace(200) @llvm.cheri.cap.from.pointer(ptr addrspace(200) %cap, iCAPRANGE %ptr)
  ret ptr addrspace(200) %newcap
}

define ptr addrspace(200) @from_ddc(iCAPRANGE %ptr) nounwind {
  %ddc = call ptr addrspace(200) @llvm.cheri.ddc.get()
  %cap = call ptr addrspace(200) @llvm.cheri.cap.from.pointer(ptr addrspace(200) %ddc, iCAPRANGE %ptr)
  ret ptr addrspace(200) %cap
}

define iCAPRANGE @diff(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %diff = call iCAPRANGE @llvm.cheri.cap.diff(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  ret iCAPRANGE %diff
}

define ptr addrspace(200) @ddc_get() nounwind {
  %cap = call ptr addrspace(200) @llvm.cheri.ddc.get()
  ret ptr addrspace(200) %cap
}

define ptr addrspace(200) @pcc_get() nounwind {
  %cap = call ptr addrspace(200) @llvm.cheri.pcc.get()
  ret ptr addrspace(200) %cap
}

; Assertion Instructions

declare i1 @llvm.cheri.cap.subset.test(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)

define iCAPRANGE @subset_test(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %subset = call i1 @llvm.cheri.cap.subset.test(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  %subset.zext = zext i1 %subset to iCAPRANGE
  ret iCAPRANGE %subset.zext
}

declare i1 @llvm.cheri.cap.equal.exact(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)

define iCAPRANGE @equal_exact(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2) nounwind {
  %eqex = call i1 @llvm.cheri.cap.equal.exact(ptr addrspace(200) %cap1, ptr addrspace(200) %cap2)
  %eqex.zext = zext i1 %eqex to iCAPRANGE
  ret iCAPRANGE %eqex.zext
}
