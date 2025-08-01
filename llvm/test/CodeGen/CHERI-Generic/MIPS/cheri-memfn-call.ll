; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/cheri-memfn-call.ll
; Check that we call memset_c/memmove_c/memcpy_c in hybrid mode.
; RUN: llc -mtriple=mips64 -mcpu=cheri128 -mattr=+cheri128 --relocation-model=pic -target-abi purecap %s -o - | FileCheck %s --check-prefix=PURECAP
; RUN: llc -mtriple=mips64 -mcpu=cheri128 -mattr=+cheri128 --relocation-model=pic -target-abi n64 %s -o - | FileCheck %s --check-prefix=HYBRID
%struct.x = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }

declare void @llvm.memmove.p200.p200.i64(ptr addrspace(200) nocapture, ptr addrspace(200) nocapture readonly, i64, i1)
declare void @llvm.memset.p200.i64(ptr addrspace(200) nocapture, i8, i64, i1)
declare void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) nocapture, ptr addrspace(200) nocapture readonly, i64, i1)

define void @call_memset(ptr addrspace(200) align 4 %dst) nounwind {
; PURECAP-LABEL: call_memset:
; PURECAP:       # %bb.0: # %entry
; PURECAP-NEXT:    cincoffset $c11, $c11, -16
; PURECAP-NEXT:    csc $c17, $zero, 0($c11) # 16-byte Folded Spill
; PURECAP-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PURECAP-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PURECAP-NEXT:    cgetpccincoffset $c1, $1
; PURECAP-NEXT:    clcbi $c12, %capcall20(memset)($c1)
; PURECAP-NEXT:    daddiu $4, $zero, 0
; PURECAP-NEXT:    cjalr $c12, $c17
; PURECAP-NEXT:    daddiu $5, $zero, 40
; PURECAP-NEXT:    clc $c17, $zero, 0($c11) # 16-byte Folded Reload
; PURECAP-NEXT:    cjr $c17
; PURECAP-NEXT:    cincoffset $c11, $c11, 16
;
; HYBRID-LABEL: call_memset:
; HYBRID:       # %bb.0: # %entry
; HYBRID-NEXT:    daddiu $sp, $sp, -16
; HYBRID-NEXT:    sd $ra, 8($sp) # 8-byte Folded Spill
; HYBRID-NEXT:    sd $gp, 0($sp) # 8-byte Folded Spill
; HYBRID-NEXT:    lui $1, %hi(%neg(%gp_rel(call_memset)))
; HYBRID-NEXT:    daddu $1, $1, $25
; HYBRID-NEXT:    daddiu $gp, $1, %lo(%neg(%gp_rel(call_memset)))
; HYBRID-NEXT:    ld $25, %call16(memset_c)($gp)
; HYBRID-NEXT:    daddiu $4, $zero, 0
; HYBRID-NEXT:    .reloc .Ltmp0, R_MIPS_JALR, memset_c
; HYBRID-NEXT:  .Ltmp0:
; HYBRID-NEXT:    jalr $25
; HYBRID-NEXT:    daddiu $5, $zero, 40
; HYBRID-NEXT:    ld $gp, 0($sp) # 8-byte Folded Reload
; HYBRID-NEXT:    ld $ra, 8($sp) # 8-byte Folded Reload
; HYBRID-NEXT:    jr $ra
; HYBRID-NEXT:    daddiu $sp, $sp, 16
entry:
  call void @llvm.memset.p200.i64(ptr addrspace(200) align 4 %dst, i8 0, i64 40, i1 false)
  ret void
}

define void @call_memcpy(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src) nounwind {
; PURECAP-LABEL: call_memcpy:
; PURECAP:       # %bb.0: # %entry
; PURECAP-NEXT:    cincoffset $c11, $c11, -16
; PURECAP-NEXT:    csc $c17, $zero, 0($c11) # 16-byte Folded Spill
; PURECAP-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PURECAP-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PURECAP-NEXT:    cgetpccincoffset $c1, $1
; PURECAP-NEXT:    clcbi $c12, %capcall20(memcpy)($c1)
; PURECAP-NEXT:    cjalr $c12, $c17
; PURECAP-NEXT:    daddiu $4, $zero, 40
; PURECAP-NEXT:    clc $c17, $zero, 0($c11) # 16-byte Folded Reload
; PURECAP-NEXT:    cjr $c17
; PURECAP-NEXT:    cincoffset $c11, $c11, 16
;
; HYBRID-LABEL: call_memcpy:
; HYBRID:       # %bb.0: # %entry
; HYBRID-NEXT:    daddiu $sp, $sp, -16
; HYBRID-NEXT:    sd $ra, 8($sp) # 8-byte Folded Spill
; HYBRID-NEXT:    sd $gp, 0($sp) # 8-byte Folded Spill
; HYBRID-NEXT:    lui $1, %hi(%neg(%gp_rel(call_memcpy)))
; HYBRID-NEXT:    daddu $1, $1, $25
; HYBRID-NEXT:    daddiu $gp, $1, %lo(%neg(%gp_rel(call_memcpy)))
; HYBRID-NEXT:    ld $25, %call16(memcpy_c)($gp)
; HYBRID-NEXT:    .reloc .Ltmp1, R_MIPS_JALR, memcpy_c
; HYBRID-NEXT:  .Ltmp1:
; HYBRID-NEXT:    jalr $25
; HYBRID-NEXT:    daddiu $4, $zero, 40
; HYBRID-NEXT:    ld $gp, 0($sp) # 8-byte Folded Reload
; HYBRID-NEXT:    ld $ra, 8($sp) # 8-byte Folded Reload
; HYBRID-NEXT:    jr $ra
; HYBRID-NEXT:    daddiu $sp, $sp, 16
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src, i64 40, i1 false)
  ret void
}

define void @call_memmove(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src) nounwind {
; PURECAP-LABEL: call_memmove:
; PURECAP:       # %bb.0: # %entry
; PURECAP-NEXT:    cincoffset $c11, $c11, -16
; PURECAP-NEXT:    csc $c17, $zero, 0($c11) # 16-byte Folded Spill
; PURECAP-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; PURECAP-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; PURECAP-NEXT:    cgetpccincoffset $c1, $1
; PURECAP-NEXT:    clcbi $c12, %capcall20(memmove)($c1)
; PURECAP-NEXT:    cjalr $c12, $c17
; PURECAP-NEXT:    daddiu $4, $zero, 40
; PURECAP-NEXT:    clc $c17, $zero, 0($c11) # 16-byte Folded Reload
; PURECAP-NEXT:    cjr $c17
; PURECAP-NEXT:    cincoffset $c11, $c11, 16
;
; HYBRID-LABEL: call_memmove:
; HYBRID:       # %bb.0: # %entry
; HYBRID-NEXT:    daddiu $sp, $sp, -16
; HYBRID-NEXT:    sd $ra, 8($sp) # 8-byte Folded Spill
; HYBRID-NEXT:    sd $gp, 0($sp) # 8-byte Folded Spill
; HYBRID-NEXT:    lui $1, %hi(%neg(%gp_rel(call_memmove)))
; HYBRID-NEXT:    daddu $1, $1, $25
; HYBRID-NEXT:    daddiu $gp, $1, %lo(%neg(%gp_rel(call_memmove)))
; HYBRID-NEXT:    ld $25, %call16(memmove_c)($gp)
; HYBRID-NEXT:    .reloc .Ltmp2, R_MIPS_JALR, memmove_c
; HYBRID-NEXT:  .Ltmp2:
; HYBRID-NEXT:    jalr $25
; HYBRID-NEXT:    daddiu $4, $zero, 40
; HYBRID-NEXT:    ld $gp, 0($sp) # 8-byte Folded Reload
; HYBRID-NEXT:    ld $ra, 8($sp) # 8-byte Folded Reload
; HYBRID-NEXT:    jr $ra
; HYBRID-NEXT:    daddiu $sp, $sp, 16
entry:
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src, i64 40, i1 false)
  ret void
}

