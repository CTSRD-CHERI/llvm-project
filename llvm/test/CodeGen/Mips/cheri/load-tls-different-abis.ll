; FreeBSD sed is different from GNU sed -> use the lowest common denominator
; RUN: sed 's/addrspace(200)//' %s | sed  's/addrspace(TLS)//' | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mattr=-xgot -o - -show-mc-encoding -print-after=finalize-isel 2>&1  | FileCheck %s -check-prefixes=MIPS,COMMON
; RUN: sed 's/addrspace(TLS)/addrspace(200)/' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=PCREL,COMMON,CAP-TABLE
; RUN: sed 's/addrspace(TLS)/addrspace(200)/' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=plt     -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=PLT,COMMON,CAP-TABLE
; RUN: sed 's/addrspace(TLS)/addrspace(200)/' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=fn-desc -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=FNDESC,COMMON,CAP-TABLE


target triple = "cheri-unknown-freebsd"

@global_tls = external thread_local(initialexec) addrspace(TLS) global i64, align 4
@global_normal = external addrspace(200) global i64, align 4

; Function Attrs: nounwind
define i64 @test() local_unnamed_addr {
entry:
  %loaded_tls = load i64, i64 addrspace(TLS)* @global_tls, align 8
  %loaded_normal = load i64, i64 addrspace(200)* @global_normal, align 8
  %result = add i64 %loaded_tls, %loaded_normal
  ret i64 %result
}

; COMMON-LABEL: Machine code for function test:
; MIPS:   Function Live Ins: $t9_64
; We currently fall back to tls via mips hwr $29 for all CHERI abis:
; PLT: Function Live Ins: [[CGP:\$c26]]
; FNDESC: Function Live Ins: [[CGP:\$c26]]

; COMMON-LABEL: bb.0.entry:


; MIPS-NEXT:  liveins: $t9_64
; MIPS-NEXT:  %9:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; MIPS-NEXT:  %10:gpr64 = DADDu %9:gpr64, $t9_64
; MIPS-NEXT:  %0:gpr64 = DADDiu %10:gpr64, target-flags(mips-gpoff-lo) @test
; MIPS-NEXT:  %1:gpr64 = RDHWR64 $hwr29
; MIPS-NEXT:  $v1_64 = COPY %1
; MIPS-NEXT:  %2:gpr64 = LD %0:gpr64, target-flags(mips-gottprel) @global_tls :: (load (s64))
; MIPS-NEXT:  %3:gpr64 = COPY $v1_64
; MIPS-NEXT:  %4:gpr64 = DADDu %3:gpr64, killed %2
; MIPS-NEXT:  %5:gpr64 = LD killed %4:gpr64, 0 :: (dereferenceable load (s64) from @global_tls)
; MIPS-NEXT:  %6:gpr64 = LD %0:gpr64, target-flags(mips-got-disp) @global_normal :: (load (s64) from got)
; MIPS-NEXT:  %7:gpr64 = LD killed %6:gpr64, 0 :: (dereferenceable load (s64) from @global_normal)
; MIPS-NEXT:  [[RESULT:%8]]:gpr64 = DADDu killed %5:gpr64, killed %7

; PCREL, PLT and FNDESC only differ in the prologue since they all use the same TLS mechanism:
; PLT-NEXT: liveins: $c26
; PLT-NEXT:  [[CGP_VREG:%[0-9]+]]:cherigpr = COPY $c26
; FNDESC-NEXT: liveins: $c26
; FNDESC-NEXT:  [[CGP_VREG:%[0-9]+]]:cherigpr = COPY $c26
; PCREL needs to derive $cgp so it's not live-in:
; PCREL-NEXT: [[CGP_VREG:%[0-9]+]]:cherigpr = PseudoPccRelativeAddressPostRA &_CHERI_CAPABILITY_TABLE_, implicit-def dead early-clobber %1:gpr64


; CAP-TABLE-NEXT:  %[[A:([0-9])]]:gpr64 = LUi64 target-flags(mips-captable-gottprel-hi16) @global_tls
; CAP-TABLE-NEXT:  %[[B:([0-9])]]:gpr64 = DADDiu killed %[[A]]:gpr64, target-flags(mips-captable-gottprel-lo16) @global_tls
; CAP-TABLE-NEXT:  %[[C:([0-9])]]:cherigpr = CIncOffset [[CGP_VREG]]:cherigpr, killed %[[B]]:gpr64
; CAP-TABLE-NEXT:  %[[D:([0-9])]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %[[C]]:cherigpr :: (load (s64) from cap-table)
; CAP-TABLE-NEXT:  %[[E:([0-9])]]:cherigpr = CReadHwr $caphwr1
; CAP-TABLE-NEXT:  %[[F:([0-9])]]:cherigpr = CIncOffset killed %[[E]]:cherigpr, killed %[[D]]:gpr64
; CAP-TABLE-NEXT:  %[[G:([0-9])]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %[[F]]:cherigpr :: (dereferenceable load (s64) from @global_tls, addrspace 200)
; CAP-TABLE-NEXT:  %[[H:([0-9])+]]:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global_normal, [[CGP_VREG]]:cherigpr :: (load (s128) from cap-table)
; CAP-TABLE-NEXT:  %[[I:([0-9])+]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %[[H]]:cherigpr :: (dereferenceable load (s64) from @global_normal, addrspace 200)
; CAP-TABLE-NEXT:  [[RESULT:%([0-9])+]]:gpr64 = DADDu killed %[[G]]:gpr64, killed %[[I]]:gpr64


; COMMON-NEXT:   $v0_64 = COPY [[RESULT]]
; COMMON-NEXT:   {{CapRetPseudo|RetRA}} implicit $v0_64


; Now check the assembly text output:

; COMMON: .text
; COMMON: test:
; COMMON-LABEL: # %bb.0:


; MIPS-NEXT: lui	$1, %hi(%neg(%gp_rel(test))) # encoding: [0x3c,0x01,A,A]
; MIPS-NEXT:           #   fixup A - offset: 0, value: %hi(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_HI
; MIPS-NEXT: daddu	$1, $1, $25
; MIPS-NEXT: daddiu	$1, $1, %lo(%neg(%gp_rel(test))) # encoding: [0x64,0x21,A,A]
; MIPS-NEXT:           #   fixup A - offset: 0, value: %lo(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_LO
; MIPS-NEXT: .set	push
; MIPS-NEXT: .set	mips32r2
; MIPS-NEXT: rdhwr	$3, $29
; MIPS-NEXT: .set	pop
; MIPS-NEXT: ld	$2, %gottprel(global_tls)($1) # encoding: [0xdc,0x22,A,A]
; MIPS-NEXT:           #   fixup A - offset: 0, value: %gottprel(global_tls), kind: fixup_Mips_GOTTPREL
; MIPS-NEXT: daddu	$2, $3, $2
; MIPS-NEXT: ld	$2, 0($2)
; MIPS-NEXT: ld	$1, %got_disp(global_normal)($1) # encoding: [0xdc,0x21,A,A]
; MIPS-NEXT:                                        #   fixup A - offset: 0, value: %got_disp(global_normal), kind: fixup_Mips_GOT_DISP
; MIPS-NEXT: ld	$1, 0($1)
; MIPS-NEXT: jr	$ra
; MIPS-NEXT: daddu	$2, $2, $1


; For PCREL derive $cgp from $pcc now:
; PCREL-NEXT:           lui     $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8) # encoding: [0x3c,0x01,A,A]
; PCREL-NEXT:                   # fixup A - offset: 0, value: %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8), kind: fixup_MIPS_PCHI16
; PCREL-NEXT:           daddiu  $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4) # encoding: [0x64,0x21,A,A]
; PCREL-NEXT:                   # fixup A - offset: 0, value: %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4), kind: fixup_MIPS_PCLO16
; PCREL-NEXT:           cgetpccincoffset [[CGP:\$c1]], $1 # encoding: [0x48,0x01,0x0c,0xff]
; CAP-TABLE-NEXT:       lui     [[CAPTAB_TPREL:\$[0-9]+]], %captab_tprel_hi(global_tls)
; CAP-TABLE-NEXT:                                       #   fixup A - offset: 0, value: %captab_tprel_hi(global_tls), kind: fixup_CHERI_CAPTAB_TPREL_HI16
; CAP-TABLE-NEXT:       daddiu  [[CAPTAB_TPREL]], [[CAPTAB_TPREL]], %captab_tprel_lo(global_tls)
; CAP-TABLE-NEXT:                                       #   fixup A - offset: 0, value: %captab_tprel_lo(global_tls), kind: fixup_CHERI_CAPTAB_TPREL_LO16
; CAP-TABLE-NEXT:       cld     [[TLSOFF:\$[0-9]+]], [[CAPTAB_TPREL]], 0([[CGP]])
; CAP-TABLE-NEXT:       clcbi   $c1, %captab20(global_normal)([[CGP]])
; CAP-TABLE-NEXT:                    #   fixup A - offset: 0, value: %captab20(global_normal), kind: fixup_CHERI_CAPTABLE20
; CAP-TABLE-NEXT:       creadhwr [[TLSBASE:\$c[0-9]+]], $chwr_userlocal
; CAP-TABLE-NEXT:       cld     $1, [[TLSOFF]], 0([[TLSBASE]])
; CAP-TABLE-NEXT:       cld     $2, $zero, 0($c1)
; CAP-TABLE-NEXT:       cjr     $c17
; CAP-TABLE-NEXT:       daddu   $2, $1, $2
