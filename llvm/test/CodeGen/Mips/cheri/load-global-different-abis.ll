; RUN: sed 's/addrspace(200)//' %s | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mxgot -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | FileCheck %s -check-prefixes=MIPS,MIPS-MXGOT,COMMON
; RUN: sed 's/addrspace(200)//' %s | llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mxgot=false -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | FileCheck %s -check-prefixes=MIPS,MIPS-SMALLGOT,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=legacy  %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=LEGACY,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=plt     %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PLT,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=fn-desc %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=FNDESC,COMMON
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel   %s -o - -show-mc-encoding -print-machineinstrs=expand-isel-pseudos 2>&1 | %cheri_FileCheck %s -check-prefixes=PCREL,COMMON

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
; Legacy gets the GOT via $c12 (->$t9_64)
; LEGACY: Function Live Ins: $c12
; PLT and FNDESC modes both expect $c26 to be live-in
; PLT: Function Live Ins: $c26
; FNDESC: Function Live Ins: $c26
; However, PCREL derives it from $pcc so only needs $c12
; PCREL: Function Live Ins: $c12

; COMMON-LABEL: bb.0.entry:


; MIPS-NEXT: liveins: $t9_64
; MIPS-NEXT: %{{5|3}}:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; MIPS-NEXT: %{{6|4}}:gpr64 = DADDu %{{5|3}}:gpr64, $t9_64
; MIPS-NEXT: %0:gpr64 = DADDiu %{{6|4}}:gpr64, target-flags(mips-gpoff-lo) @test
; MIPS-MXGOT-NEXT: %1:gpr64 = LUi64 target-flags(mips-got-hi16) @global
; MIPS-MXGOT-NEXT: %2:gpr64 = DADDu killed %1:gpr64, %0
; MIPS-MXGOT-NEXT: [[ADDR:%3]]:gpr64 = LD killed %2:gpr64, target-flags(mips-got-lo16) @global, implicit $ddc :: (load 8 from got)
; MIPS-SMALLGOT-NEXT: [[ADDR:%1]]:gpr64 = LD %0:gpr64, target-flags(mips-got-disp) @global, implicit $ddc :: (load 8 from got)
; MIPS-NEXT: [[RESULT:%.+]]:gpr64 = LD killed [[ADDR]]:gpr64, 0, implicit $ddc :: (dereferenceable load 8 from @global)


; Legacy loads the vaddr of global from the got then loads .size.global and
; does a cfromptr + csetbounds with those two values:
; LEGACY-NEXT:   liveins: $c12
; LEGACY-NEXT:   $t9_64 = CGetOffset $c12
; LEGACY-NEXT:   %11:gpr64 = LUi64 target-flags(mips-gpoff-hi) @test
; LEGACY-NEXT:   %12:gpr64 = DADDu %11:gpr64, $t9_64
; LEGACY-NEXT:   %0:gpr64 = DADDiu %12:gpr64, target-flags(mips-gpoff-lo) @test
; LEGACY-NEXT:   %1:gpr64 = DADDiu %0:gpr64, target-flags(mips-got-disp) @.size.global
; LEGACY-NEXT:   %2:cherigpr = CFromPtr $ddc, killed %1
; LEGACY-NEXT:   %3:gpr64 = CAPLOAD64 $zero_64, 0, killed %2:cherigpr :: (load 8 from got)
; LEGACY-NEXT:   %4:gpr64 = LD killed %3:gpr64, 0, implicit $ddc :: (load 8 from @.size.global)
; LEGACY-NEXT:   %5:gpr64 = DADDiu %0:gpr64, target-flags(mips-got-disp) @global
; LEGACY-NEXT:   %6:cherigpr = CFromPtr $ddc, killed %5
; LEGACY-NEXT:   %7:gpr64 = CAPLOAD64 $zero_64, 0, killed %6:cherigpr :: (load 8 from got)
; LEGACY-NEXT:   %8:cherigpr = CFromPtr $ddc, killed %7
; LEGACY-NEXT:   %9:cherigpr = CSetBounds killed %8:cherigpr, killed %4:gpr64
; LEGACY-NEXT:   [[RESULT:%10]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %9:cherigpr :: (dereferenceable load 8 from @global, addrspace 200)


; Due to the $cgp live-in the PLT and FNDESC functions are much shorter:

; PLT-NEXT: liveins: $c26
; PLT-NEXT: %0:cherigpr = COPY $c26
; PLT-NEXT: %1:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global, %0:cherigpr :: (load [[$CAP_SIZE]] from cap-table)
; PLT-NEXT: [[RESULT:%2]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %1:cherigpr :: (dereferenceable load 8 from @global, addrspace 200)

; FNDESC-NEXT:  liveins: $c26
; FNDESC-NEXT:  %0:cherigpr = COPY $c26
; FNDESC-NEXT:  %1:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global, %0:cherigpr :: (load [[$CAP_SIZE]] from cap-table)
; FNDESC-NEXT:  [[RESULT:%2]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %1:cherigpr :: (dereferenceable load 8 from @global, addrspace 200)

; PCREL-NEXT: liveins: $c12
; PCREL-NEXT: %3:gpr64 = LUi64 target-flags(mips-captable-off-hi) @test
; PCREL-NEXT: %4:gpr64 = DADDiu %3:gpr64, target-flags(mips-captable-off-lo) @test
; PCREL-NEXT: $c26 = CIncOffset $c12, %4:gpr64
; PCREL-NEXT: %0:cherigpr = COPY $c26
; PCREL-NEXT: %1:cherigpr = LOADCAP_BigImm target-flags(mips-captable20) @global, %0:cherigpr :: (load [[$CAP_SIZE]] from cap-table)
; PCREL-NEXT: [[RESULT:%2]]:gpr64 = CAPLOAD64 $zero_64, 0, killed %1:cherigpr :: (dereferenceable load 8 from @global, addrspace 200)


; COMMON-NEXT:   $v0_64 = COPY [[RESULT]]
; COMMON-NEXT:   {{CapRetPseudo|RetRA}} implicit $v0_64


; Now check the asssembly text output:

; COMMON: .text
; COMMON: test:
; COMMON-LABEL: # %bb.0:


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


; LEGACY-NEXT: cgetoffset	$25, $c12
; LEGACY-NEXT: lui	$1, %hi(%neg(%gp_rel(test))) # encoding: [0x3c,0x01,A,A]
; LEGACY-NEXT:          #   fixup A - offset: 0, value: %hi(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_HI
; LEGACY-NEXT: daddu	$1, $1, $25
; LEGACY-NEXT: daddiu	$1, $1, %lo(%neg(%gp_rel(test))) # encoding: [0x64,0x21,A,A]
; LEGACY-NEXT:                #   fixup A - offset: 0, value: %lo(%neg(%gp_rel(test))), kind: fixup_Mips_GPOFF_LO
; LEGACY-NEXT: ld	$2, %got_disp(.size.global)($1)
; LEGACY-NEXT:         #   fixup A - offset: 0, value: %got_disp(.size.global), kind: fixup_Mips_GOT_DISP
; LEGACY-NEXT: ld	$1, %got_disp(global)($1)
; LEGACY-NEXT:        #   fixup A - offset: 0, value: %got_disp(global), kind: fixup_Mips_GOT_DISP
; LEGACY-NEXT: ld	$2, 0($2)
; LEGACY-NEXT: cfromddc	$c1, $1
; LEGACY-NEXT: csetbounds	$c1, $c1, $2
; LEGACY-NEXT: cjr	$c17
; LEGACY-NEXT: cld	$2, $zero, 0($c1)

; PLT-NEXT:	clcbi	$c1, %captab20(global)($c26) # encoding: [0x74,0x3a,A,A]
; PLT-NEXT:                #   fixup A - offset: 0, value: %captab20(global), kind: fixup_CHERI_CAPTABLE20
; PLT-NEXT:	cjr	$c17
; PLT-NEXT:	cld	$2, $zero, 0($c1)

; FNDESC-NEXT:	clcbi	$c1, %captab20(global)($c26) # encoding: [0x74,0x3a,A,A]
; FNDESC-NEXT:             #   fixup A - offset: 0, value: %captab20(global), kind: fixup_CHERI_CAPTABLE20
; FNDESC-NEXT:	cjr	$c17
; FNDESC-NEXT:	cld	$2, $zero, 0($c1)


; PCREL-NEXT: lui	$1, %hi(%neg(%captab_rel(test))) # encoding: [0x3c,0x01,A,A]
; PCREL-NEXT:         #   fixup A - offset: 0, value: %hi(%neg(%captab_rel(test))), kind:
; PCREL-NEXT: daddiu	$1, $1, %lo(%neg(%captab_rel(test))) # encoding: [0x64,0x21,A,A]
; PCREL-NEXT:                 #   fixup A - offset: 0, value: %lo(%neg(%captab_rel(test))), kind:
; PCREL-NEXT: cincoffset	$[[CGP:c26]], $c12, $1
; PCREL-NEXT: clcbi	$c1, %captab20(global)($[[CGP]]) # encoding: [0x74,0x3a,A,A]
; PCREL-NEXT:            #   fixup A - offset: 0, value: %captab20(global), kind: fixup_CHERI_CAPTABLE20
; PCREL-NEXT: cjr	$c17
; PCREL-NEXT: cld	$2, $zero, 0($c1)
