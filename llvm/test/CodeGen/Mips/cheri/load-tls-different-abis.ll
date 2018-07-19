; RUN: sed 's/addrspace(\(200\|TLS\))//' %s | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mxgot=false -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | FileCheck %s -check-prefixes=MIPS,COMMON
; RUN: sed 's/addrspace(TLS)//' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=legacy  -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=LEGACY,COMMON
; RUN: sed 's/addrspace(TLS)//' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=plt     -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PLT,COMMON,CAP-TABLE,CAP-TABLE-HACK
; RUN: sed 's/addrspace(TLS)//' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=fn-desc -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=FNDESC,COMMON,CAP-TABLE,CAP-TABLE-HACK
; RUN: sed 's/addrspace(TLS)//' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel   -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PCREL,COMMON,CAP-TABLE,CAP-TABLE-HACK
; RUN: sed 's/addrspace(TLS)/addrspace(200)/' %s | %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel   -cheri-cap-tls-abi=cap-equiv -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PCREL,COMMON,CAP-TABLE,CAP-EQUIV

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
; LEGACY-HACK: Function Live Ins: $c12
; PLT: Function Live Ins: $c12, $c26
; FN-DESC: Function Live Ins: $c12, $c26
; PCREL: Function Live Ins: $c12

; COMMON-LABEL: bb.0.entry:


; MIPS-NEXT:  liveins: $t9_64
; MIPS-NEXT:  %9:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; MIPS-NEXT:  %10:gpr64 = DADDu %9:gpr64, $t9_64
; MIPS-NEXT:  %0:gpr64 = DADDiu %10:gpr64, target-flags(mips-gpoff-lo) @test
; MIPS-NEXT:  %1:gpr64 = RDHWR64 $hwr29
; MIPS-NEXT:  $v1_64 = COPY %1
; MIPS-NEXT:  %2:gpr64 = LD %0:gpr64, target-flags(mips-gottprel) @global_tls, implicit $ddc :: (load 8)
; MIPS-NEXT:  %3:gpr64 = COPY $v1_64
; MIPS-NEXT:  %4:gpr64 = DADDu %3:gpr64, killed %2
; MIPS-NEXT:  %5:gpr64 = LD killed %4:gpr64, 0, implicit $ddc :: (dereferenceable load 8 from @global_tls)
; MIPS-NEXT:  %6:gpr64 = LD %0:gpr64, target-flags(mips-got-disp) @global_normal, implicit $ddc :: (load 8 from got)
; MIPS-NEXT:  %7:gpr64 = LD killed %6:gpr64, 0, implicit $ddc :: (dereferenceable load 8 from @global_normal)
; MIPS-NEXT:  [[RESULT:%8]]:gpr64 = DADDu killed %5:gpr64, killed %7

; We currently use the same legacy hack of using the MIPS hwr29 for all ABIs except the new cap-equiv ABI:

; LEGACY-NEXT:  liveins: $c12
; LEGACY-NEXT:  $t9_64 = CGetOffset $c12
; LEGACY-NEXT:  %17:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; LEGACY-NEXT:  %18:gpr64 = DADDu %17:gpr64, $t9_64
; LEGACY-NEXT:  %0:gpr64 = DADDiu %18:gpr64, target-flags(mips-gpoff-lo) @test
; LEGACY-NEXT:  %1:gpr64 = RDHWR64 $hwr29
; LEGACY-NEXT:  $v1_64 = COPY %1
; LEGACY-NEXT:  %2:gpr64 = LD %0:gpr64, target-flags(mips-gottprel) @global_tls, implicit $ddc :: (load 8)
; LEGACY-NEXT:  %3:gpr64 = COPY $v1_64
; LEGACY-NEXT:  %4:gpr64 = DADDu %3:gpr64, killed %2
; LEGACY-NEXT:  %5:gpr64 = LD killed %4:gpr64, 0, implicit $ddc :: (dereferenceable load 8 from @global_tls)
; LEGACY-NEXT:  %6:gpr64 = DADDiu %0:gpr64, target-flags(mips-got-disp) @.size.global_normal
; LEGACY-NEXT:  %7:cherigpr = CFromPtr $ddc, killed %6
; LEGACY-NEXT:  %8:gpr64 = CAPLOAD64 $zero_64, 0, killed %7:cherigpr :: (load 8 from got)
; LEGACY-NEXT:  %9:gpr64 = LD killed %8:gpr64, 0, implicit $ddc :: (load 8 from @.size.global_normal)
; LEGACY-NEXT:  %10:gpr64 = DADDiu %0:gpr64, target-flags(mips-got-disp) @global_normal
; LEGACY-NEXT:  %11:cherigpr = CFromPtr $ddc, killed %10
; LEGACY-NEXT:  %12:gpr64 = CAPLOAD64 $zero_64, 0, killed %11:cherigpr :: (load 8 from got)
; LEGACY-NEXT:  %13:cherigpr = CFromPtr $ddc, killed %12
; LEGACY-NEXT:  %14:cherigpr = CSetBounds killed %13:cherigpr, killed %9:gpr64
; LEGACY-NEXT:  %15:gpr64 = CAPLOAD64 $zero_64, 0, killed %14:cherigpr :: (dereferenceable load 8 from @global_normal, addrspace 200)
; LEGACY-NEXT:  [[RESULT:%16]]:gpr64 = DADDu killed %5:gpr64, killed %15


; PCREL, PLT and FNDESC only differ in the prologue since they all use the same hack:
; PLT-NEXT: liveins: $c12, $c26
; PLT-NEXT:  %0:cherigpr = COPY $c26
; FNDESC-NEXT: liveins: $c12, $c26
; FNDESC-NEXT:  %0:cherigpr = COPY $c26
; PCREL needs to derive $cgp so it's not live-in:
; PCREL-NEXT: liveins: $c12
; PCREL-NEXT: [[CGPOFF_HI:%[0-9]+]]:gpr64 = LUi64 target-flags(mips-captable-off-hi) @test
; PCREL-NEXT: [[CGPOFF_LO:%[0-9]+]]:gpr64 = DADDiu [[CGPOFF_HI]]:gpr64, target-flags(mips-captable-off-lo) @test
; PCREL-NEXT: $c26 = CIncOffset $c12, [[CGPOFF_LO]]:gpr64
; PCREL-NEXT: %0:cherigpr = COPY $c26


; CAP-TABLE-HACK-NEXT:  $t9_64 = CGetOffset $c12
; CAP-TABLE-HACK-NEXT:  %10:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; CAP-TABLE-HACK-NEXT:  %11:gpr64 = DADDu %10:gpr64, $t9_64
; CAP-TABLE-HACK-NEXT:  %1:gpr64 = DADDiu %11:gpr64, target-flags(mips-gpoff-lo) @test
; CAP-TABLE-HACK-NEXT:  %2:gpr64 = RDHWR64 $hwr29
; CAP-TABLE-HACK-NEXT:  $v1_64 = COPY %2
; CAP-TABLE-HACK-NEXT:  %3:gpr64 = LD %1:gpr64, target-flags(mips-gottprel) @global_tls, implicit $ddc :: (load 8)
; CAP-TABLE-HACK-NEXT:  %4:gpr64 = COPY $v1_64
; CAP-TABLE-HACK-NEXT:  %5:gpr64 = DADDu %4:gpr64, killed %3
; CAP-TABLE-HACK-NEXT:  %6:gpr64 = LD killed %5:gpr64, 0, implicit $ddc :: (dereferenceable load 8 from @global_tls)
; CAP-TABLE-HACK-NEXT:  %7:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global_normal, %0:cherigpr :: (load {{16|32}} from cap-table)
; CAP-TABLE-HACK-NEXT:  %8:gpr64 = CAPLOAD64 $zero_64, 0, killed %7:cherigpr :: (dereferenceable load 8 from @global_normal, addrspace 200)
; CAP-TABLE-HACK-NEXT:  [[RESULT:%9]]:gpr64 = DADDu killed %6:gpr64, killed %8:gpr64


; CAP-EQUIV-NEXT:  %1:gpr64 = LUi64 target-flags(mips-captable-gottprel-hi16) @global_tls
; CAP-EQUIV-NEXT:  %2:gpr64 = DADDiu killed %1:gpr64, target-flags(mips-captable-gottprel-lo16) @global_tls
; CAP-EQUIV-NEXT:  %3:cherigpr = CIncOffset %0:cherigpr, killed %2:gpr64
; CAP-EQUIV-NEXT:  %4:gpr64 = CAPLOAD64 $zero_64, 0, killed %3:cherigpr :: (load 8 from cap-table)
; CAP-EQUIV-NEXT:  %5:cherigpr = CReadHwr $caphwr1
; CAP-EQUIV-NEXT:  %6:cherigpr = CIncOffset killed %5:cherigpr, killed %4:gpr64
; CAP-EQUIV-NEXT:  %7:gpr64 = CAPLOAD64 $zero_64, 0, killed %6:cherigpr :: (dereferenceable load 8 from @global_tls, addrspace 200)
; CAP-EQUIV-NEXT:  %8:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global_normal, %0:cherigpr :: (load {{16|32}} from cap-table)
; CAP-EQUIV-NEXT:  %9:gpr64 = CAPLOAD64 $zero_64, 0, killed %8:cherigpr :: (dereferenceable load 8 from @global_normal, addrspace 200)
; CAP-EQUIV-NEXT:  [[RESULT:%10]]:gpr64 = DADDu killed %7:gpr64, killed %9:gpr64


; COMMON-NEXT:   $v0_64 = COPY [[RESULT]]
; COMMON-NEXT:   {{CapRetPseudo|RetRA}} implicit $v0_64


; Now check the asssembly text output:

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



; Get $t9 for the tls hack:
; CAP-TABLE-HACK-NEXT:  cgetoffset      $25, $c12
; PCREL-NEXT:           lui     $1, %hi(%neg(%captab_rel(test))) # encoding: [0x3c,0x01,A,A]
; PCREL-NEXT:                   #   fixup A - offset: 0, value: %hi(%neg(%captab_rel(test))), kind: fixup_Mips_CAPTABLEREL_HI
; PCREL-NEXT:           daddiu  $1, $1, %lo(%neg(%captab_rel(test))) # encoding: [0x64,0x21,A,A]
; PCREL-NEXT:                   #   fixup A - offset: 0, value: %lo(%neg(%captab_rel(test))), kind: fixup_Mips_CAPTABLEREL_LO
; From now on it's all the same TLS hack:
; CAP-TABLE-HACK-NEXT:  lui     [[TLSADDR:\$.+]], %hi(%neg(%gp_rel(test)))
; CAP-TABLE-HACK-NEXT:                                  #   fixup A - offset: 0, value: %hi(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_HI
; CAP-TABLE-HACK-NEXT:  daddu   [[TLSADDR]], [[TLSADDR]], $25
; CAP-TABLE-HACK-NEXT:  daddiu  [[TLSADDR]], [[TLSADDR]], %lo(%neg(%gp_rel(test)))
; CAP-TABLE-HACK-NEXT:                                  #   fixup A - offset: 0, value: %lo(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_LO
; CAP-TABLE-HACK-NEXT:  .set    push
; CAP-TABLE-HACK-NEXT:  .set    mips32r2
; CAP-TABLE-HACK-NEXT:  rdhwr   [[HWREG:\$3]], $29
; CAP-TABLE-HACK-NEXT:  .set    pop
; CAP-TABLE-HACK-NEXT:  ld      [[TLSPTR:\$.+]], %gottprel(global_tls)([[TLSADDR]])
; CAP-TABLE-HACK-NEXT:                       #   fixup A - offset: 0, value: %gottprel(global_tls), kind: fixup_Mips_GOTTPREL
; For PCREL derive $cgp from $c12 now:
; PCREL-NEXT:           cincoffset      $c26, $c12, $1
; CAP-EQUIV-NEXT:       lui     [[CAPTAB_TPREL:\$[0-9]+]], %captab_tprel_hi(global_tls)
; CAP-EQUIV-NEXT:                                       #   fixup A - offset: 0, value: %captab_tprel_hi(global_tls), kind: fixup_CHERI_CAPTAB_TPREL_HI16
; CAP-EQUIV-NEXT:       daddiu  [[CAPTAB_TPREL]], [[CAPTAB_TPREL]], %captab_tprel_lo(global_tls)
; CAP-EQUIV-NEXT:                                       #   fixup A - offset: 0, value: %captab_tprel_lo(global_tls), kind: fixup_CHERI_CAPTAB_TPREL_LO16
; CAP-EQUIV-NEXT:       cld     [[TLSOFF:\$[0-9]+]], [[CAPTAB_TPREL]], 0($c26)
; CAP-TABLE-NEXT:       clcbi   $c1, %captab20(global_normal)($c26)
; CAP-TABLE-NEXT:                    #   fixup A - offset: 0, value: %captab20(global_normal), kind: fixup_CHERI_CAPTABLE20
; CAP-TABLE-HACK-NEXT:  daddu   $1, [[HWREG]], [[TLSPTR]]
; CAP-TABLE-HACK-NEXT:  ld      $1, 0($1)
; CAP-EQUIV-NEXT:       creadhwr [[TLSBASE:\$c[0-9]+]], $chwr_userlocal
; CAP-EQUIV-NEXT:       cld     $1, [[TLSOFF]], 0([[TLSBASE]])
; CAP-TABLE-NEXT:       cld     $2, $zero, 0($c1)
; CAP-TABLE-NEXT:       cjr     $c17
; CAP-TABLE-NEXT:       daddu   $2, $1, $2
