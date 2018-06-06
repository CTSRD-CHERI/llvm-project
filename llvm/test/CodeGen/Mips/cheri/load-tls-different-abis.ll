; RUN: sed 's/addrspace(200)//' %s | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mxgot=false -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | FileCheck %s -check-prefixes=MIPS,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=legacy  %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=LEGACY,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=plt     %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PLT,COMMON,CAP-TABLE-HACK
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=fn-desc %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=FNDESC,COMMON,CAP-TABLE-HACK
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel   %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PCREL,COMMON,CAP-TABLE-HACK

target triple = "cheri-unknown-freebsd"

@global_tls = external thread_local(initialexec) global i64, align 4
; TODO: @global_tls = external thread_local(initialexec) addrspace(200) global i64, align 4
@global_normal = external addrspace(200) global i64, align 4

; Function Attrs: nounwind
define i64 @test() local_unnamed_addr {
entry:
  %loaded_tls = load i64, i64* @global_tls, align 8
  ; TODO: %loaded_tls = load i64, i64 addrspace(200)* @global_tls, align 8
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
; MIPS-NEXT:  %10:gpr64 = DADDu %9, $t9_64
; MIPS-NEXT:  %0:gpr64 = DADDiu %10, target-flags(mips-gpoff-lo) @test
; MIPS-NEXT:  %1:gpr64 = RDHWR64 $hwr29
; MIPS-NEXT:  $v1_64 = COPY %1
; MIPS-NEXT:  %2:gpr64 = LD %0, target-flags(mips-gottprel) @global_tls, implicit $ddc; mem:LD8[<unknown>]
; MIPS-NEXT:  %3:gpr64 = COPY $v1_64
; MIPS-NEXT:  %4:gpr64 = DADDu %3, killed %2
; MIPS-NEXT:  %5:gpr64 = LD killed %4, 0, implicit $ddc; mem:LD8[@global_tls](dereferenceable)
; MIPS-NEXT:  %6:gpr64 = LD %0, target-flags(mips-got-disp) @global_normal, implicit $ddc; mem:LD8[GOT]
; MIPS-NEXT:  %7:gpr64 = LD killed %6, 0, implicit $ddc; mem:LD8[@global_normal](dereferenceable)
; MIPS-NEXT:  [[RESULT:%8]]:gpr64 = DADDu killed %5, killed %7

; We currently use the same legacy hack of using the MIPS hwr29 for all ABIs:

; LEGACY-NEXT:  liveins: $c12
; LEGACY-NEXT:  $t9_64 = CGetOffset $c12
; LEGACY-NEXT:  %17:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; LEGACY-NEXT:  %18:gpr64 = DADDu %17, $t9_64
; LEGACY-NEXT:  %0:gpr64 = DADDiu %18, target-flags(mips-gpoff-lo) @test
; LEGACY-NEXT:  %1:gpr64 = RDHWR64 $hwr29
; LEGACY-NEXT:  $v1_64 = COPY %1
; LEGACY-NEXT:  %2:gpr64 = LD %0, target-flags(mips-gottprel) @global_tls, implicit $ddc; mem:LD8[<unknown>]
; LEGACY-NEXT:  %3:gpr64 = COPY $v1_64
; LEGACY-NEXT:  %4:gpr64 = DADDu %3, killed %2
; LEGACY-NEXT:  %5:gpr64 = LD killed %4, 0, implicit $ddc; mem:LD8[@global_tls](dereferenceable)
; LEGACY-NEXT:  %6:gpr64 = DADDiu %0, target-flags(mips-got-disp) @.size.global_normal
; LEGACY-NEXT:  %7:cheriregs = CFromPtr $c0, killed %6
; LEGACY-NEXT:  %8:gpr64 = CAPLOAD64 $zero_64, 0, killed %7; mem:LD8[GOT]
; LEGACY-NEXT:  %9:gpr64 = LD killed %8, 0, implicit $ddc; mem:LD8[@.size.global_normal]
; LEGACY-NEXT:  %10:gpr64 = DADDiu %0, target-flags(mips-got-disp) @global_normal
; LEGACY-NEXT:  %11:cheriregs = CFromPtr $c0, killed %10
; LEGACY-NEXT:  %12:gpr64 = CAPLOAD64 $zero_64, 0, killed %11; mem:LD8[GOT]
; LEGACY-NEXT:  %13:cheriregs = CFromPtr $c0, killed %12
; LEGACY-NEXT:  %14:cheriregs = CSetBounds killed %13, killed %9
; LEGACY-NEXT:  %15:gpr64 = CAPLOAD64 $zero_64, 0, killed %14; mem:LD8[@global_normal(addrspace=200)](dereferenceable)
; LEGACY-NEXT:  [[RESULT:%16]]:gpr64 = DADDu killed %5, killed %15


; PCREL, PLT and FNDESC only differ in the prologue since they all use the same hack:
; PLT-NEXT: liveins: $c12, $c26
; PLT-NEXT:  %0:cheriregs = COPY $c26
; FNDESC-NEXT: liveins: $c12, $c26
; FNDESC-NEXT:  %0:cheriregs = COPY $c26
; PCREL needs to derive $cgp so it's not live-in:
; PCREL-NEXT: liveins: $c12
; PCREL-NEXT:  %12:gpr64 = LUi64 target-flags(mips-captable-off-hi) @test
; PCREL-NEXT: %13:gpr64 = DADDiu %12, target-flags(mips-captable-off-lo) @test
; PCREL-NEXT: $c26 = CIncOffset $c12, %13
; PCREL-NEXT: %0:cheriregs = COPY $c26


; CAP-TABLE-HACK-NEXT:  $t9_64 = CGetOffset $c12
; CAP-TABLE-HACK-NEXT:  %10:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; CAP-TABLE-HACK-NEXT:  %11:gpr64 = DADDu %10, $t9_64
; CAP-TABLE-HACK-NEXT:  %1:gpr64 = DADDiu %11, target-flags(mips-gpoff-lo) @test
; CAP-TABLE-HACK-NEXT:  %2:gpr64 = RDHWR64 $hwr29
; CAP-TABLE-HACK-NEXT:  $v1_64 = COPY %2
; CAP-TABLE-HACK-NEXT:  %3:gpr64 = LD %1, target-flags(mips-gottprel) @global_tls, implicit $ddc; mem:LD8[<unknown>]
; CAP-TABLE-HACK-NEXT:  %4:gpr64 = COPY $v1_64
; CAP-TABLE-HACK-NEXT:  %5:gpr64 = DADDu %4, killed %3
; CAP-TABLE-HACK-NEXT:  %6:gpr64 = LD killed %5, 0, implicit $ddc; mem:LD8[@global_tls](dereferenceable)
; CAP-TABLE-HACK-NEXT:  %7:cheriregs = LOADCAP_BigImm target-flags(mips-captable20) @global_normal, %0; mem:LD[[$CAP_SIZE]][JumpTable]
; CAP-TABLE-HACK-NEXT:  %8:gpr64 = CAPLOAD64 $zero_64, 0, killed %7; mem:LD8[@global_normal(addrspace=200)](dereferenceable)
; CAP-TABLE-HACK-NEXT:  [[RESULT:%9]]:gpr64 = DADDu killed %6, killed %8


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
; CAP-TABLE-HACK-NEXT:  clcbi   $c1, %captab20(global_normal)($c26)
; CAP-TABLE-HACK-NEXT:               #   fixup A - offset: 0, value: %captab20(global_normal), kind: fixup_CHERI_CAPTABLE20
; CAP-TABLE-HACK-NEXT:  daddu   $1, [[HWREG]], [[TLSPTR]]
; CAP-TABLE-HACK-NEXT:  ld      $1, 0($1)
; CAP-TABLE-HACK-NEXT:  cld     $2, $zero, 0($c1)
; CAP-TABLE-HACK-NEXT:  cjr     $c17
; CAP-TABLE-HACK-NEXT:  daddu   $2, $1, $2
