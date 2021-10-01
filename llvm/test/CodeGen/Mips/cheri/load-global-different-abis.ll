; RUN: sed 's/addrspace(200)//' %s | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mattr=+xgot -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=MIPS,MIPS-MXGOT,COMMON
; RUN: sed 's/addrspace(200)//' %s | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mattr=-xgot -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=MIPS,MIPS-SMALLGOT,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=plt     %s -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=PLT,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=fn-desc %s -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=FNDESC,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel   %s -o - -show-mc-encoding -print-after=finalize-isel 2>&1 | FileCheck %s -check-prefixes=PCREL,COMMON

target triple = "cheri-unknown-freebsd"

@global = local_unnamed_addr addrspace(200) global i64 123, align 8

declare void @extern_func()

; Function Attrs: nounwind
define i64 @test() local_unnamed_addr {
entry:
  %loaded = load i64, i64 addrspace(200)* @global, align 8
  ret i64 %loaded
}
; COMMON-LABEL: Machine code for function test:
; MIPS:   Function Live Ins: $t9_64
; PLT and FNDESC modes both expect $c26 to be live-in
; PLT: Function Live Ins: $c26
; FNDESC: Function Live Ins: $c26


; COMMON-LABEL: bb.0.entry:

; MIPS-NEXT: liveins: $t9_64
; MIPS-NEXT: %{{5|3}}:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; MIPS-NEXT: %{{6|4}}:gpr64 = DADDu %{{5|3}}:gpr64, $t9_64
; MIPS-NEXT: %0:gpr64 = DADDiu %{{6|4}}:gpr64, target-flags(mips-gpoff-lo) @test
; MIPS-MXGOT-NEXT: %1:gpr64 = LUi64 target-flags(mips-got-hi16) @global
; MIPS-MXGOT-NEXT: %2:gpr64 = DADDu killed %1:gpr64, %0
; MIPS-MXGOT-NEXT: [[ADDR:%3]]:gpr64 = LD killed %2:gpr64, target-flags(mips-got-lo16) @global :: (load (s64) from got)
; MIPS-SMALLGOT-NEXT: [[ADDR:%1]]:gpr64 = LD %0:gpr64, target-flags(mips-got-disp) @global :: (load (s64) from got)
; MIPS-NEXT: [[RESULT:%.+]]:gpr64 = LD killed [[ADDR]]:gpr64, 0 :: (dereferenceable load (s64) from @global)


; Due to the $cgp live-in the PLT and FNDESC functions are much shorter:

; PLT-NEXT: liveins: $c26
; PLT-NEXT: %0:cherigpr = COPY $c26
; PLT-NEXT: %1:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global, %0:cherigpr :: (load (s128) from cap-table)
; PLT-NEXT: [[RESULT:%2]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %1:cherigpr :: (dereferenceable load (s64) from @global, addrspace 200)

; FNDESC-NEXT:  liveins: $c26
; FNDESC-NEXT:  %0:cherigpr = COPY $c26
; FNDESC-NEXT:  %1:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global, %0:cherigpr :: (load (s128) from cap-table)
; FNDESC-NEXT:  [[RESULT:%2]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %1:cherigpr :: (dereferenceable load (s64) from @global, addrspace 200)

; PCREL-NEXT: [[CAPTABLE:%.+:cherigpr]] = PseudoPccRelativeAddressPostRA &_CHERI_CAPABILITY_TABLE_, implicit-def dead early-clobber %1:gpr64
; PCREL-NEXT: [[GLOBAL_CAP:%.+:cherigpr]] = LOADCAP_BigImm target-flags(mips-captable20) @global, [[CAPTABLE]] :: (load (s128) from cap-table)
; PCREL-NEXT: [[RESULT:%([0-9]+)]]:gpr64 = CAPLOAD64 $zero_64, 0, killed [[GLOBAL_CAP]] :: (dereferenceable load (s64) from @global, addrspace 200)


; COMMON-NEXT:   $v0_64 = COPY [[RESULT]]
; COMMON-NEXT:   {{CapRetPseudo|RetRA}} implicit $v0_64


; Now check the asssembly text output:

; COMMON: .text
; COMMON: test:
; COMMON-LABEL: # %bb.0


; MIPS-NEXT: lui	$1, %hi(%neg(%gp_rel(test))) # encoding: [0x3c,0x01,A,A]
; MIPS-NEXT:          #   fixup A - offset: 0, value: %hi(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_HI
; MIPS-NEXT: daddu	$1, $1, $25
; MIPS-NEXT: daddiu	$1, $1, %lo(%neg(%gp_rel(test))) # encoding: [0x64,0x21,A,A]
; MIPS-NEXT:                #   fixup A - offset: 0, value: %lo(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_LO
; MIPS-MXGOT-NEXT: lui	$2, %got_hi(global)     # encoding: [0x3c,0x02,A,A]
; MIPS-MXGOT-NEXT:          #   fixup A - offset: 0, value: %got_hi(global), kind: fixup_Mips_GOT_HI16
; MIPS-MXGOT-NEXT: daddu	$1, $2, $1
; MIPS-MXGOT-NEXT: ld	$1, %got_lo(global)($1) # encoding: [0xdc,0x21,A,A]
; MIPS-MXGOT-NEXT:        #   fixup A - offset: 0, value: %got_lo(global), kind: fixup_Mips_GOT_LO16
; MIPS-SMALLGOT-NEXT: ld	$1, %got_disp(global)($1) # encoding: [0xdc,0x21,A,A]
; MIPS-SMALLGOT-NEXT:         #   fixup A - offset: 0, value: %got_disp(global), kind: fixup_Mips_GOT_DISP
; MIPS-NEXT: jr	$ra
; MIPS-NEXT: ld	$2, 0($1)


; PLT-NEXT:	clcbi	$c1, %captab20(global)($c26) # encoding: [0x74,0x3a,A,A]
; PLT-NEXT:                #   fixup A - offset: 0, value: %captab20(global), kind: fixup_CHERI_CAPTABLE20
; PLT-NEXT:	cjr	$c17
; PLT-NEXT:	cld	$2, $zero, 0($c1)

; FNDESC-NEXT:	clcbi	$c1, %captab20(global)($c26) # encoding: [0x74,0x3a,A,A]
; FNDESC-NEXT:             #   fixup A - offset: 0, value: %captab20(global), kind: fixup_CHERI_CAPTABLE20
; FNDESC-NEXT:	cjr	$c17
; FNDESC-NEXT:	cld	$2, $zero, 0($c1)


; PCREL-NEXT: lui	$1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8) # encoding: [0x3c,0x01,A,A]
; PCREL-NEXT:         #   fixup A - offset: 0, value: %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8), kind:
; PCREL-NEXT: daddiu	$1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4) # encoding: [0x64,0x21,A,A]
; PCREL-NEXT:                 #   fixup A - offset: 0, value: %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4), kind:
; PCREL-NEXT: cgetpccincoffset	$[[CGP:c1]], $1
; PCREL-NEXT: clcbi	$c1, %captab20(global)($[[CGP]]) # encoding: [0x74,0x21,A,A]
; PCREL-NEXT:            #   fixup A - offset: 0, value: %captab20(global), kind: fixup_CHERI_CAPTABLE20
; PCREL-NEXT: cjr	$c17
; PCREL-NEXT: cld	$2, $zero, 0($c1)
