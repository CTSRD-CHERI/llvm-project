	.text
	.file	"/Users/theraven/Documents/Research/cheri/llvm/test/CodeGen/PowerPC/extra-toc-reg-deps.ll"
	.globl	_ZN4Foam13checkTopologyERKNS_8polyMeshEbb
	.p2align	4
	.type	_ZN4Foam13checkTopologyERKNS_8polyMeshEbb,@function
	.section	.opd,"aw",@progbits
_ZN4Foam13checkTopologyERKNS_8polyMeshEbb: # @_ZN4Foam13checkTopologyERKNS_8polyMeshEbb
	.p2align	3
	.quad	.Lfunc_begin0
	.quad	.TOC.@tocbase
	.quad	0
	.text
.Lfunc_begin0:
	.cfi_startproc
	.cfi_personality 148, DW.ref.__gxx_personality_v0
	.cfi_lsda 20, .Lexception0
# BB#0:                                 # %entry
	mflr 0
	mfcr 12
	std 31, -8(1)
	std 0, 16(1)
	stw 12, 8(1)
	stdu 1, -128(1)
.Lcfi0:
	.cfi_def_cfa_offset 128
.Lcfi1:
	.cfi_offset r31, -8
.Lcfi2:
	.cfi_offset lr, 16
	mr 31, 1
.Lcfi3:
	.cfi_def_cfa_register r31
.Lcfi4:
	.cfi_offset cr2, 8
	andi. 3, 3, 1
	crmove	 8, 1
	bc 4, 20, .LBB0_2
# BB#1:                                 # %for.body
.LBB0_2:                                # %for.cond.cleanup
	bc 4, 20, .LBB0_5
# BB#3:                                 # %_ZN4Foam12returnReduceIiNS_5sumOpIiEEEET_RKS3_RKT0_ii.exit
	bc 4, 20, .LBB0_6
# BB#4:                                 # %for.body.i
.LBB0_5:                                # %if.else.i
.LBB0_6:                                # %_ZNK4Foam8ZoneMeshINS_8cellZoneENS_8polyMeshEE15checkDefinitionEb.exit
	bc 12, 20, .LBB0_8
# BB#7:                                 # %for.body.i1480
.LBB0_8:                                # %_ZNK4Foam8ZoneMeshINS_8faceZoneENS_8polyMeshEE15checkDefinitionEb.exit
	bc 12, 20, .LBB0_10
# BB#9:                                 # %for.body.i1504
.LBB0_10:                               # %_ZNK4Foam8ZoneMeshINS_9pointZoneENS_8polyMeshEE15checkDefinitionEb.exit
.Ltmp0:
	bl _ZN4Foam4word12stripInvalidEv
	nop
.Ltmp1:
# BB#11:                                # %_ZN4Foam4wordC2EPKcb.exit
.Ltmp3:
	bl _ZN4Foam7cellSetC1ERKNS_8polyMeshERKNS_4wordEiNS_8IOobject11writeOptionE
	nop
.Ltmp4:
# BB#12:                                # %invoke.cont59
	bc 4, 20, .LBB0_14
# BB#13:                                # %for.body70
.LBB0_14:                               # %for.cond.cleanup69
	bc 4, 20, .LBB0_16
# BB#15:                                # %if.then121
.LBB0_16:                               # %if.else
.Ltmp6:
	bl _ZN4Foam4word12stripInvalidEv
	nop
.Ltmp7:
# BB#17:                                # %_ZN4Foam4wordC2EPKcb.exit1701
.Ltmp9:
	bl _ZN4Foam8pointSetC1ERKNS_8polyMeshERKNS_4wordEiNS_8IOobject11writeOptionE
	nop
.Ltmp10:
# BB#18:                                # %invoke.cont169
.Ltmp12:
	ld 11, 0(3)
	std 2, 40(1)
	li 4, 1
	mr 2, 11
	mtctr 11
	bctrl
	ld 2, 40(1)
	andi. 3, 3, 1
.Ltmp13:
# BB#19:                                # %invoke.cont176
	bc 4, 1, .LBB0_21
# BB#20:                                # %if.then178
.LBB0_21:                               # %if.end213
.Ltmp17:
	bl _ZN4Foam4word12stripInvalidEv
	nop
.Ltmp18:
# BB#22:                                # %_ZN4Foam4wordC2EPKcb.exit1777
.Ltmp20:
	bl _ZN4Foam7faceSetC1ERKNS_8polyMeshERKNS_4wordEiNS_8IOobject11writeOptionE
	nop
.Ltmp21:
# BB#23:                                # %invoke.cont221
.Ltmp23:
	ld 11, 0(3)
	std 2, 40(1)
	li 4, 1
	mr 2, 11
	mtctr 11
	bctrl
	ld 2, 40(1)
.Ltmp24:
# BB#24:                                # %invoke.cont231
.Ltmp25:
	bl _ZN4Foam6reduceIiNS_5sumOpIiEEEEvRKNS_4ListINS_8UPstream11commsStructEEERT_RKT0_ii
	nop
.Ltmp26:
# BB#25:                                # %invoke.cont243
.Ltmp30:
	bl _ZN4Foam4word12stripInvalidEv
	nop
.Ltmp31:
# BB#26:                                # %_ZN4Foam4wordC2EPKcb.exit1862
.Ltmp33:
	bl _ZN4Foam7faceSetC1ERKNS_8polyMeshERKNS_4wordEiNS_8IOobject11writeOptionE
	nop
.Ltmp34:
# BB#27:                                # %invoke.cont280
	bc 4, 20, .LBB0_30
# BB#28:                                # %_ZNSsD2Ev.exit1877
	bc 12, 20, .LBB0_31
# BB#29:                                # %if.then292
.LBB0_30:                               # %if.then.i.i1869
.LBB0_31:                               # %if.end328
	bc 4, 8, .LBB0_33
# BB#32:                                # %if.then331
.LBB0_33:                               # %if.end660
.Ltmp36:
	bl _ZN4Foam13messageStreamcvRNS_8OSstreamEEv
	nop
.Ltmp37:
# BB#34:                                # %invoke.cont668
.Ltmp38:
	bl _ZN4FoamlsERNS_7OstreamEPKc
	nop
.Ltmp39:
# BB#35:                                # %invoke.cont670
.Ltmp40:
	bl _ZN4FoamlsERNS_7OstreamEi
	nop
.Ltmp41:
# BB#36:                                # %invoke.cont674
.Ltmp42:
	bl _ZN4FoamlsERNS_7OstreamEPKc
	nop
.Ltmp43:
# BB#37:                                # %invoke.cont676
.Ltmp44:
	ld 11, 0(3)
	std 2, 40(1)
	mr 2, 11
	mtctr 11
	bctrl
	ld 2, 40(1)
.Ltmp45:
# BB#38:                                # %if.end878
	bc 4, 20, .LBB0_41
# BB#39:                                # %_ZN4Foam11regionSplitD2Ev.exit
	bc 12, 20, .LBB0_42
# BB#40:                                # %if.then883
.LBB0_41:                               # %delete.notnull.i.i3056
.LBB0_42:                               # %if.else888
.Ltmp49:
	bl _ZN4Foam4word12stripInvalidEv
	nop
.Ltmp50:
# BB#43:                                # %_ZN4Foam4wordC2EPKcb.exit3098
.Ltmp52:
	bl _ZN4Foam8pointSetC1ERKNS_8polyMeshERKNS_4wordEiNS_8IOobject11writeOptionE
	nop
.Ltmp53:
# BB#44:                                # %invoke.cont902
	bc 12, 20, .LBB0_47
# BB#45:                                # %if.then.i.i3105
	bc 12, 20, .LBB0_47
# BB#46:                                # %if.then4.i.i3112
.LBB0_47:                               # %_ZNSsD2Ev.exit3113
.Ltmp55:
	addis 3, 2, .LC0@toc@ha
	std 2, 40(1)
	ld 3, .LC0@toc@l(3)
	ld 11, 0(3)
	mr 2, 11
	mtctr 11
	bctrl
	ld 2, 40(1)
.Ltmp56:
# BB#48:                                # %call.i3116.noexc
.Ltmp57:
	addis 3, 2, .LC0@toc@ha
	ld 5, 0(0)
	ld 11, 16(0)
	std 2, 40(1)
	ld 3, .LC0@toc@l(3)
	ld 2, 8(0)
	li 4, 0
	mtctr 5
	bctrl
	ld 2, 40(1)
.Ltmp58:
# BB#49:                                # %invoke.cont906
.LBB0_50:                               # %lpad898
.Ltmp54:
	bc 4, 20, .LBB0_52
# BB#51:                                # %_ZNSsD2Ev.exit3204
.LBB0_52:                               # %if.then.i.i3196
.LBB0_53:                               # %lpad.i3086
.Ltmp51:
.LBB0_54:                               # %lpad905.loopexit.split-lp
.Ltmp59:
.Ltmp60:
	bl _ZN4Foam8pointSetD1Ev
	nop
.Ltmp61:
	b .LBB0_65
.LBB0_55:                               # %lpad663
.Ltmp46:
	bc 4, 20, .LBB0_57
# BB#56:                                # %_ZN4Foam4ListIiED2Ev.exit.i3073
.Ltmp47:
	bl _ZN4Foam11regIOobjectD2Ev
	nop
.Ltmp48:
	b .LBB0_65
.LBB0_57:                               # %delete.notnull.i.i3071
.LBB0_58:                               # %lpad276
.Ltmp35:
.LBB0_59:                               # %lpad.i1850
.Ltmp32:
.LBB0_60:                               # %lpad217
.Ltmp22:
                                        # implicit-def: %X3
	bl _Unwind_Resume
	nop
.LBB0_61:                               # %lpad.i1765
.Ltmp19:
	bc 12, 20, .LBB0_74
# BB#62:                                # %if.then.i.i.i1767
.LBB0_63:                               # %lpad230
.Ltmp27:
.Ltmp28:
	bl _ZN4Foam7faceSetD1Ev
	nop
.Ltmp29:
	b .LBB0_65
.LBB0_64:                               # %lpad175
.Ltmp14:
.Ltmp15:
	bl _ZN4Foam8pointSetD1Ev
	nop
.Ltmp16:
.LBB0_65:                               # %eh.resume
                                        # implicit-def: %X3
	bl _Unwind_Resume
	nop
.LBB0_66:                               # %terminate.lpad
.Ltmp62:
.LBB0_67:                               # %lpad165
.Ltmp11:
.LBB0_68:                               # %lpad.i1689
.Ltmp8:
.LBB0_69:                               # %lpad
.Ltmp5:
	bc 12, 20, .LBB0_72
# BB#70:                                # %if.then.i.i1570
	bc 12, 20, .LBB0_72
# BB#71:                                # %if.then4.i.i1577
.LBB0_72:                               # %_ZNSsD2Ev.exit1578
.LBB0_73:                               # %lpad.i
.Ltmp2:
.LBB0_74:                               # %unwind_resume
	bl _Unwind_Resume
	nop
	.long	0
	.quad	0
.Lfunc_end0:
	.size	_ZN4Foam13checkTopologyERKNS_8polyMeshEbb, .Lfunc_end0-.Lfunc_begin0
	.cfi_endproc
	.section	.gcc_except_table,"a",@progbits
	.p2align	2
GCC_except_table0:
.Lexception0:
	.byte	255                     # @LPStart Encoding = omit
	.byte	148                     # @TType Encoding = indirect pcrel udata8
	.asciz	"\367\201"              # @TType base offset
	.byte	3                       # Call site Encoding = udata4
	.ascii	"\352\001"              # Call site table length
	.long	.Ltmp0-.Lfunc_begin0    # >> Call Site 1 <<
	.long	.Ltmp1-.Ltmp0           #   Call between .Ltmp0 and .Ltmp1
	.long	.Ltmp2-.Lfunc_begin0    #     jumps to .Ltmp2
	.byte	0                       #   On action: cleanup
	.long	.Ltmp3-.Lfunc_begin0    # >> Call Site 2 <<
	.long	.Ltmp4-.Ltmp3           #   Call between .Ltmp3 and .Ltmp4
	.long	.Ltmp5-.Lfunc_begin0    #     jumps to .Ltmp5
	.byte	0                       #   On action: cleanup
	.long	.Ltmp6-.Lfunc_begin0    # >> Call Site 3 <<
	.long	.Ltmp7-.Ltmp6           #   Call between .Ltmp6 and .Ltmp7
	.long	.Ltmp8-.Lfunc_begin0    #     jumps to .Ltmp8
	.byte	0                       #   On action: cleanup
	.long	.Ltmp9-.Lfunc_begin0    # >> Call Site 4 <<
	.long	.Ltmp10-.Ltmp9          #   Call between .Ltmp9 and .Ltmp10
	.long	.Ltmp11-.Lfunc_begin0   #     jumps to .Ltmp11
	.byte	0                       #   On action: cleanup
	.long	.Ltmp12-.Lfunc_begin0   # >> Call Site 5 <<
	.long	.Ltmp13-.Ltmp12         #   Call between .Ltmp12 and .Ltmp13
	.long	.Ltmp14-.Lfunc_begin0   #     jumps to .Ltmp14
	.byte	0                       #   On action: cleanup
	.long	.Ltmp17-.Lfunc_begin0   # >> Call Site 6 <<
	.long	.Ltmp18-.Ltmp17         #   Call between .Ltmp17 and .Ltmp18
	.long	.Ltmp19-.Lfunc_begin0   #     jumps to .Ltmp19
	.byte	0                       #   On action: cleanup
	.long	.Ltmp20-.Lfunc_begin0   # >> Call Site 7 <<
	.long	.Ltmp21-.Ltmp20         #   Call between .Ltmp20 and .Ltmp21
	.long	.Ltmp22-.Lfunc_begin0   #     jumps to .Ltmp22
	.byte	0                       #   On action: cleanup
	.long	.Ltmp23-.Lfunc_begin0   # >> Call Site 8 <<
	.long	.Ltmp26-.Ltmp23         #   Call between .Ltmp23 and .Ltmp26
	.long	.Ltmp27-.Lfunc_begin0   #     jumps to .Ltmp27
	.byte	0                       #   On action: cleanup
	.long	.Ltmp30-.Lfunc_begin0   # >> Call Site 9 <<
	.long	.Ltmp31-.Ltmp30         #   Call between .Ltmp30 and .Ltmp31
	.long	.Ltmp32-.Lfunc_begin0   #     jumps to .Ltmp32
	.byte	0                       #   On action: cleanup
	.long	.Ltmp33-.Lfunc_begin0   # >> Call Site 10 <<
	.long	.Ltmp34-.Ltmp33         #   Call between .Ltmp33 and .Ltmp34
	.long	.Ltmp35-.Lfunc_begin0   #     jumps to .Ltmp35
	.byte	0                       #   On action: cleanup
	.long	.Ltmp36-.Lfunc_begin0   # >> Call Site 11 <<
	.long	.Ltmp45-.Ltmp36         #   Call between .Ltmp36 and .Ltmp45
	.long	.Ltmp46-.Lfunc_begin0   #     jumps to .Ltmp46
	.byte	0                       #   On action: cleanup
	.long	.Ltmp49-.Lfunc_begin0   # >> Call Site 12 <<
	.long	.Ltmp50-.Ltmp49         #   Call between .Ltmp49 and .Ltmp50
	.long	.Ltmp51-.Lfunc_begin0   #     jumps to .Ltmp51
	.byte	0                       #   On action: cleanup
	.long	.Ltmp52-.Lfunc_begin0   # >> Call Site 13 <<
	.long	.Ltmp53-.Ltmp52         #   Call between .Ltmp52 and .Ltmp53
	.long	.Ltmp54-.Lfunc_begin0   #     jumps to .Ltmp54
	.byte	0                       #   On action: cleanup
	.long	.Ltmp55-.Lfunc_begin0   # >> Call Site 14 <<
	.long	.Ltmp58-.Ltmp55         #   Call between .Ltmp55 and .Ltmp58
	.long	.Ltmp59-.Lfunc_begin0   #     jumps to .Ltmp59
	.byte	0                       #   On action: cleanup
	.long	.Ltmp60-.Lfunc_begin0   # >> Call Site 15 <<
	.long	.Ltmp48-.Ltmp60         #   Call between .Ltmp60 and .Ltmp48
	.long	.Ltmp62-.Lfunc_begin0   #     jumps to .Ltmp62
	.byte	1                       #   On action: 1
	.long	.Ltmp48-.Lfunc_begin0   # >> Call Site 16 <<
	.long	.Ltmp28-.Ltmp48         #   Call between .Ltmp48 and .Ltmp28
	.long	0                       #     has no landing pad
	.byte	0                       #   On action: cleanup
	.long	.Ltmp28-.Lfunc_begin0   # >> Call Site 17 <<
	.long	.Ltmp16-.Ltmp28         #   Call between .Ltmp28 and .Ltmp16
	.long	.Ltmp62-.Lfunc_begin0   #     jumps to .Ltmp62
	.byte	1                       #   On action: 1
	.long	.Ltmp16-.Lfunc_begin0   # >> Call Site 18 <<
	.long	.Lfunc_end0-.Ltmp16     #   Call between .Ltmp16 and .Lfunc_end0
	.long	0                       #     has no landing pad
	.byte	0                       #   On action: cleanup
	.byte	1                       # >> Action Record 1 <<
                                        #   Catch TypeInfo 1
	.byte	0                       #   No further actions
                                        # >> Catch TypeInfos <<
	.quad	0                       # TypeInfo 1
	.p2align	2

	.section	.toc,"aw",@progbits
.LC0:
	.tc _ZN4Foam4PoutE[TC],_ZN4Foam4PoutE
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.DW.ref.__gxx_personality_v0,"aGw",@progbits,DW.ref.__gxx_personality_v0,comdat
	.p2align	3
	.type	DW.ref.__gxx_personality_v0,@object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0

	.section	".note.GNU-stack","",@progbits
