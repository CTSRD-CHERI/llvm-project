; FreeBSD sed is different from GNU sed -> use the lowest common denominator
; RUN: sed 's/addrspace(200)//' %s | sed  's/addrspace(TLS)//' | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mxgot=false -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | FileCheck %s -check-prefixes=MIPS,COMMON
; RUN: sed 's/addrspace(TLS)//' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=legacy  -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=LEGACY,COMMON
; RUN: sed 's/addrspace(TLS)/addrspace(200)/' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PCREL,COMMON,CAP-TABLE,CAP-EQUIV
; RUN: sed 's/addrspace(TLS)/addrspace(200)/' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=plt     -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PLT,COMMON,CAP-TABLE,CAP-EQUIV
; RUN: sed 's/addrspace(TLS)/addrspace(200)/' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=fn-desc -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=FNDESC,COMMON,CAP-TABLE,CAP-EQUIV

@external_gd = external thread_local addrspace(TLS) global i64
@internal_gd = internal thread_local addrspace(TLS) global i64 42

@external_ld = external thread_local(localdynamic) addrspace(TLS) global i64
@internal_ld = internal thread_local(localdynamic) addrspace(TLS) global i64 42

@external_ie = external thread_local(initialexec) addrspace(TLS) global i64
@internal_ie = internal thread_local(initialexec) addrspace(TLS) global i64 42

@external_le = external thread_local(localexec) addrspace(TLS) global i64
@internal_le = internal thread_local(localexec) addrspace(TLS) global i64 42

; Function Attrs: nounwind
define i64 @test_gd() local_unnamed_addr {
entry:
  %loaded_tls_ext = load i64, i64 addrspace(TLS)* @external_gd, align 8
  %loaded_tls_int = load i64, i64 addrspace(TLS)* @internal_gd, align 8
  %result = add i64 %loaded_tls_ext, %loaded_tls_int
  ret i64 %result
}

; COMMON-LABEL: Machine code for function test_gd:
; MIPS:   Function Live Ins: $t9_64
; We currently fall back to tls via mips hwr $29 for all CHERI abis:
; LEGACY-HACK: Function Live Ins: $c12
; PLT: Function Live Ins: [[CGP:\$c26]]
; FNDESC: Function Live Ins: [[CGP:\$c26]]
; PCREL: Function Live Ins: $c12

; COMMON-LABEL: bb.0.entry:


; MIPS-NEXT:liveins: $t9_64
; MIPS-NEXT:  %12:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test_gd
; MIPS-NEXT:  %13:gpr64 = DADDu %12:gpr64, $t9_64
; MIPS-NEXT:  %0:gpr64 = DADDiu %13:gpr64, target-flags(mips-gpoff-lo) @test_gd
; MIPS-NEXT:  ADJCALLSTACKDOWN 0, 0, implicit-def dead $sp, implicit $sp
; MIPS-NEXT:  %1:gpr64 = DADDiu %0:gpr64, target-flags(mips-tlsgd) @external_gd
; MIPS-NEXT:  %2:gpr64 = LD %0:gpr64, target-flags(mips-got-call) &__tls_get_addr, implicit $ddc :: (load 8 from call-entry &__tls_get_addr)
; MIPS-NEXT:  $a0_64 = COPY %1:gpr64
; MIPS-NEXT:  $gp_64 = COPY %0:gpr64
; MIPS-NEXT:  JALR64Pseudo killed %2:gpr64, <regmask $fp $gp $ra $d12 $d13 $d14 $d15 $f24 $f25 $f26 $f27 $f28 $f29 $f30 $f31 $fp_64 $f_hi24 $f_hi25 $f_hi26 $f_hi27 $f_hi28 $f_hi29 $f_hi30 $f_hi31 $gp_64 $ra_64 $s0 $s1 $s2 $s3 $s4 $s5 $s6 and 17 more...>, target-flags(mips-jalr) <mcsymbol __tls_get_addr>, implicit-def dead $ra, implicit $a0_64, implicit $gp_64, implicit-def $sp, implicit-def $v0_64
; MIPS-NEXT:  ADJCALLSTACKUP 0, 0, implicit-def dead $sp, implicit $sp
; MIPS-NEXT:  %3:gpr64 = COPY $v0_64
; MIPS-NEXT:  %4:gpr64 = LD %3:gpr64, 0, implicit $ddc :: (dereferenceable load 8 from @external_gd)
; MIPS-NEXT:  ADJCALLSTACKDOWN 0, 0, implicit-def dead $sp, implicit $sp
; MIPS-NEXT:  %5:gpr64 = DADDiu %0:gpr64, target-flags(mips-tlsldm) @internal_gd
; MIPS-NEXT:  %6:gpr64 = LD %0:gpr64, target-flags(mips-got-call) &__tls_get_addr, implicit $ddc :: (load 8 from call-entry &__tls_get_addr)
; MIPS-NEXT:  $a0_64 = COPY %5:gpr64
; MIPS-NEXT:  $gp_64 = COPY %0:gpr64
; MIPS-NEXT:  JALR64Pseudo killed %6:gpr64, <regmask $fp $gp $ra $d12 $d13 $d14 $d15 $f24 $f25 $f26 $f27 $f28 $f29 $f30 $f31 $fp_64 $f_hi24 $f_hi25 $f_hi26 $f_hi27 $f_hi28 $f_hi29 $f_hi30 $f_hi31 $gp_64 $ra_64 $s0 $s1 $s2 $s3 $s4 $s5 $s6 and 17 more...>, target-flags(mips-jalr) <mcsymbol __tls_get_addr>, implicit-def dead $ra, implicit $a0_64, implicit $gp_64, implicit-def $sp, implicit-def $v0_64
; MIPS-NEXT:  ADJCALLSTACKUP 0, 0, implicit-def dead $sp, implicit $sp
; MIPS-NEXT:  %7:gpr64 = COPY $v0_64
; MIPS-NEXT:  %8:gpr64 = LUi64 target-flags(mips-dtprel-hi) @internal_gd
; MIPS-NEXT:  %9:gpr64 = DADDu killed %8:gpr64, %7:gpr64
; MIPS-NEXT:  %10:gpr64 = LD killed %9:gpr64, target-flags(mips-dtprel-lo) @internal_gd, implicit $ddc :: (dereferenceable load 8 from @internal_gd)
; MIPS-NEXT:  [[RESULT:%.+]] = DADDu killed %4:gpr64, killed %10:gpr64

; LEGACY-NEXT:  liveins: $c12
; LEGACY-NEXT:  $t9_64 = CGetOffset $c12
; LEGACY-NEXT:  %22:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test_gd
; LEGACY-NEXT:  %23:gpr64 = DADDu %22:gpr64, $t9_64
; LEGACY-NEXT:  %0:gpr64 = DADDiu %23:gpr64, target-flags(mips-gpoff-lo) @test_gd
; LEGACY-NEXT:  ADJCALLSTACKCAPDOWN 0, 0, implicit-def dead $c11, implicit $c11
; LEGACY-NEXT:  %1:gpr64 = DADDiu %0:gpr64, target-flags(mips-tlsgd) @external_gd
; LEGACY-NEXT:  %2:cherigpr = CFromPtr $ddc, killed %1:gpr64
; LEGACY-NEXT:  %3:cherigpr = CSetBoundsImm killed %2:cherigpr, 16
; LEGACY-NEXT:  %4:gpr64 = DADDiu %0:gpr64, target-flags(mips-got-call) &__tls_get_addr
; LEGACY-NEXT:  %5:cherigpr = CFromPtr $ddc, killed %4:gpr64
; LEGACY-NEXT:  %6:gpr64 = CAPLOAD64 $zero_64, 0, %5:cherigpr :: (load 8 from call-entry &__tls_get_addr)
; LEGACY-NEXT:  %7:cherigpr = CSetPCCOffset killed %6:gpr64
; LEGACY-NEXT:  $c3 = COPY %3:cherigpr
; LEGACY-NEXT:  $gp_64 = COPY %0:gpr64
; LEGACY-NEXT:  CapJumpLinkPseudo killed %7:cherigpr, <regmask $fp $gp $ra $c17 $c18 $c19 $c20 $c21 $c22 $c23 $c24 $c25 $d12 $d13 $d14 $d15 $f24 $f25 $f26 $f27 $f28 $f29 $f30 $f31 $fp_64 $f_hi24 $f_hi25 $f_hi26 $f_hi27 $f_hi28 $f_hi29 $f_hi30 $f_hi31 and 26 more...>, implicit-def dead $c17, implicit-def dead $c26, implicit $c3, implicit $gp_64, implicit-def $c11, implicit-def $c3
; LEGACY-NEXT:  ADJCALLSTACKCAPUP 0, 0, implicit-def dead $c11, implicit $c11
; LEGACY-NEXT:  %8:cherigpr = COPY $c3
; LEGACY-NEXT:  %9:gpr64 = CGetAddr %8:cherigpr
; LEGACY-NEXT:  %10:gpr64 = LD killed %9:gpr64, 0, implicit $ddc :: (dereferenceable load 8 from @external_gd)
; LEGACY-NEXT:  ADJCALLSTACKCAPDOWN 0, 0, implicit-def dead $c11, implicit $c11
; LEGACY-NEXT:  %11:gpr64 = DADDiu %0:gpr64, target-flags(mips-tlsldm) @internal_gd
; LEGACY-NEXT:  %12:cherigpr = CFromPtr $ddc, killed %11:gpr64
; LEGACY-NEXT:  %13:cherigpr = CSetBoundsImm killed %12:cherigpr, 16
; LEGACY-NEXT:  %14:gpr64 = CAPLOAD64 $zero_64, 0, %5:cherigpr :: (load 8 from call-entry &__tls_get_addr)
; LEGACY-NEXT:  %15:cherigpr = CSetPCCOffset killed %14:gpr64
; LEGACY-NEXT:  $c3 = COPY %13:cherigpr
; LEGACY-NEXT:  $gp_64 = COPY %0:gpr64
; LEGACY-NEXT:  CapJumpLinkPseudo killed %15:cherigpr, <regmask $fp $gp $ra $c17 $c18 $c19 $c20 $c21 $c22 $c23 $c24 $c25 $d12 $d13 $d14 $d15 $f24 $f25 $f26 $f27 $f28 $f29 $f30 $f31 $fp_64 $f_hi24 $f_hi25 $f_hi26 $f_hi27 $f_hi28 $f_hi29 $f_hi30 $f_hi31 and 26 more...>, implicit-def dead $c17, implicit-def dead $c26, implicit $c3, implicit $gp_64, implicit-def $c11, implicit-def $c3
; LEGACY-NEXT:  ADJCALLSTACKCAPUP 0, 0, implicit-def dead $c11, implicit $c11
; LEGACY-NEXT:  %16:cherigpr = COPY $c3
; LEGACY-NEXT:  %17:gpr64 = CGetAddr %16:cherigpr
; LEGACY-NEXT:  %18:gpr64 = LUi64 target-flags(mips-dtprel-hi) @internal_gd
; LEGACY-NEXT:  %19:gpr64 = DADDu killed %18:gpr64, killed %17:gpr64
; LEGACY-NEXT:  %20:gpr64 = LD killed %19:gpr64, target-flags(mips-dtprel-lo) @internal_gd, implicit $ddc :: (dereferenceable load 8 from @internal_gd)
; LEGACY-NEXT:  [[RESULT:%.+]] = DADDu killed %10:gpr64, killed %20:gpr64


; PCREL, PLT and FNDESC only differ in the prologue since they all use the same TLS mechanism:
; PLT-NEXT: liveins: $c26
; PLT-NEXT:  [[CAPTABLE:%0:cherigpr]] = COPY $c26
; FNDESC-NEXT: liveins: $c26
; FNDESC-NEXT:  [[CAPTABLE:%0:cherigpr]] = COPY $c26
; PCREL needs to derive $cgp so it's not live-in:
; PCREL-NEXT: liveins: $c12
; PCREL-NEXT: [[CGPOFF_HI:%[0-9]+]]:gpr64 = LUi64 target-flags(mips-captable-off-hi) @test
; PCREL-NEXT: [[CGPOFF_LO:%[0-9]+]]:gpr64 = DADDiu [[CGPOFF_HI]]:gpr64, target-flags(mips-captable-off-lo) @test
; PCREL-NEXT: [[CAPTABLE:%0:cherigpr]] = CIncOffset $c12, [[CGPOFF_LO]]:gpr64

; CAP-TABLE-NEXT:   ADJCALLSTACKCAPDOWN 0, 0, implicit-def dead $c11, implicit $c11
; CAP-TABLE-NEXT:   %1:gpr64 = LUi64 target-flags(mips-captable-tlsgd-hi16) @external_gd
; CAP-TABLE-NEXT:   %2:gpr64 = DADDiu killed %1:gpr64, target-flags(mips-captable-tlsgd-lo16) @external_gd
; CAP-TABLE-NEXT:   %3:cherigpr = CIncOffset [[CAPTABLE]], killed %2:gpr64
; CAP-TABLE-NEXT:   %4:cherigpr = CSetBoundsImm killed %3:cherigpr, 16
; CAP-TABLE-NEXT:   %5:cherigpr = LOADCAP_BigImm target-flags(mips-captable20-call) &__tls_get_addr, [[CAPTABLE]] :: (load 16 from call-entry &__tls_get_addr)
; CAP-TABLE-NEXT:   $c3 = COPY %4:cherigpr
; CAP-TABLE-NEXT:   CapJumpLinkPseudo killed %5:cherigpr, <regmask $fp $gp $ra $c17 $c18 $c19 $c20 $c21 $c22 $c23 $c24 $c25 $d12 $d13 $d14 $d15 $f24 $f25 $f26 $f27 $f28 $f29 $f30 $f31 $fp_64 $f_hi24 $f_hi25 $f_hi26 $f_hi27 $f_hi28 $f_hi29 $f_hi30 $f_hi31 and 26 more...>, implicit-def dead $c17, implicit-def dead $c26, implicit $c3, implicit-def $c11, implicit-def $c3
; CAP-TABLE-NEXT:   ADJCALLSTACKCAPUP 0, 0, implicit-def dead $c11, implicit $c11
; PLT-NEXT:         $c26 = COPY [[CAPTABLE]]
; FNDESC-NEXT:      $c26 = COPY [[CAPTABLE]]
; CAP-TABLE-NEXT:   %6:cherigpr = COPY $c3
; CAP-TABLE-NEXT:   %7:gpr64 = CAPLOAD64 $zero_64, 0, %6:cherigpr :: (dereferenceable load 8 from @external_gd, addrspace 200)
; CAP-TABLE-NEXT:   ADJCALLSTACKCAPDOWN 0, 0, implicit-def dead $c11, implicit $c11
; CAP-TABLE-NEXT:   %8:gpr64 = LUi64 target-flags(mips-captable-tlsldm-hi16) @internal_gd
; CAP-TABLE-NEXT:   %9:gpr64 = DADDiu killed %8:gpr64, target-flags(mips-captable-tlsldm-lo16) @internal_gd
; CAP-TABLE-NEXT:   %10:cherigpr = CIncOffset [[CAPTABLE]], killed %9:gpr64
; CAP-TABLE-NEXT:   %11:cherigpr = CSetBoundsImm killed %10:cherigpr, 16
; CAP-TABLE-NEXT:   %12:cherigpr = LOADCAP_BigImm target-flags(mips-captable20-call) &__tls_get_addr, [[CAPTABLE]] :: (load 16 from call-entry &__tls_get_addr)
; CAP-TABLE-NEXT:   $c3 = COPY %11:cherigpr
; CAP-TABLE-NEXT:   CapJumpLinkPseudo killed %12:cherigpr, <regmask $fp $gp $ra $c17 $c18 $c19 $c20 $c21 $c22 $c23 $c24 $c25 $d12 $d13 $d14 $d15 $f24 $f25 $f26 $f27 $f28 $f29 $f30 $f31 $fp_64 $f_hi24 $f_hi25 $f_hi26 $f_hi27 $f_hi28 $f_hi29 $f_hi30 $f_hi31 and 26 more...>, implicit-def dead $c17, implicit-def dead $c26, implicit $c3, implicit-def $c11, implicit-def $c3
; CAP-TABLE-NEXT:   ADJCALLSTACKCAPUP 0, 0, implicit-def dead $c11, implicit $c11
; PLT-NEXT:         $c26 = COPY [[CAPTABLE]]
; FNDESC-NEXT:      $c26 = COPY [[CAPTABLE]]
; CAP-TABLE-NEXT:   %13:cherigpr = COPY $c3
; CAP-TABLE-NEXT:   %14:gpr64 = LUi64 target-flags(mips-dtprel-hi) @internal_gd
; CAP-TABLE-NEXT:   %15:gpr64 = DADDiu killed %14:gpr64, target-flags(mips-dtprel-lo) @internal_gd
; CAP-TABLE-NEXT:   %16:cherigpr = CIncOffset %13:cherigpr, killed %15:gpr64
; CAP-TABLE-NEXT:   %17:gpr64 = CAPLOAD64 $zero_64, 0, killed %16:cherigpr :: (dereferenceable load 8 from @internal_gd, addrspace 200)
; CAP-TABLE-NEXT:   [[RESULT:%.+]] = DADDu killed %7:gpr64, killed %17:gpr64

; COMMON-NEXT:   $v0_64 = COPY [[RESULT]]
; COMMON-NEXT:   {{CapRetPseudo|RetRA}} implicit $v0_64

