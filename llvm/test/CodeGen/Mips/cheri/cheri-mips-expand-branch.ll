; PostgreSQL was broken in purecap mode because the MipsBranchExpansion pass was inserting MIPS instructions
; to expand a long branch:

; RUN: %cheri_llc -cheri-cap-table-abi=pcrel %s -o - -relocation-model=pic | FileCheck %s -check-prefixes CHECK,N64
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - | FileCheck %s -check-prefixes CHECK,PURECAP


; Function Attrs: nounwind
define i32 @no_expand(i32 signext %arg) local_unnamed_addr addrspace(200) #0 {
entry:
  %cmp = icmp sgt i32 %arg, 1234
  br i1 %cmp, label %if.then, label %out

if.then:                                          ; preds = %entry
  %add = add nsw i32 %arg, 3
  tail call void asm sideeffect ".space 100", "~{$1}"() #1
  br label %out

out:                                              ; preds = %entry, %if.then
  %arg.addr.0 = phi i32 [ %add, %if.then ], [ %arg, %entry ]
  %add1 = add nsw i32 %arg.addr.0, 2
  ret i32 %add1
}

; In this function the branch target is in range -> no need to expand it

; CHECK-LABEL: no_expand:
; CHECK:      # %bb.0:
; CHECK-NEXT:    slti	$1, $4, 1235
; CHECK-NEXT:    bnez	$1, .LBB0_2
; CHECK-NEXT:    nop
; CHECK-NEXT: # %bb.1:
; CHECK-NEXT:    addiu	$4, $4, 3
; CHECK-NEXT:    #APP
; CHECK:         .space	100
; CHECK:         #NO_APP
; CHECK-NEXT:  .LBB0_2:
; N64-NEXT:      jr	$ra
; PURECAP-NEXT:  cjr	$c17
; CHECK-NEXT:    addiu	$2, $4, 2
; CHECK: .end no_expand


define i32 @needs_expand(i32 signext %arg) local_unnamed_addr addrspace(200) #0 {
entry:
  %cmp = icmp sgt i32 %arg, 1234
  br i1 %cmp, label %if.then, label %out

if.then:                                          ; preds = %entry
  %add = add nsw i32 %arg, 3
  tail call void asm sideeffect ".space 1000000", "~{$1}"() #1
  br label %out

out:                                              ; preds = %entry, %if.then
  %arg.addr.0 = phi i32 [ %add, %if.then ], [ %arg, %entry ]
  %add1 = add nsw i32 %arg.addr.0, 2
  ret i32 %add1
}

; CHECK-LABEL: needs_expand:
; CHECK:     # %bb.0:
; CHECK-NEXT: 	slti	$1, $4, 1235
; CHECK-NEXT: 	beqz	$1, .LBB1_3
; CHECK-NEXT: 	nop
; CHECK-NEXT: # %bb.1:
; N64-NEXT: 	daddiu	$sp, $sp, -16
; N64-NEXT: 	sd	$ra, 0($sp)
; CHECK-NEXT: 	daddiu	$1, $zero, %hi(.LBB1_4-.LBB1_2)
; CHECK-NEXT: 	dsll	$1, $1, 16
; N64-NEXT: 	bal	.LBB1_2
; This branch is needed even in purecap since otherwise .LBB1_2 is not emitted... and the relocations break
; PURECAP-NEXT: b .LBB1_2
; CHECK-NEXT: 	daddiu	$1, $1, %lo(.LBB1_4-.LBB1_2)
; CHECK-NEXT: .LBB1_2:
; N64-NEXT: 	daddu	$1, $ra, $1
; N64-NEXT: 	ld	$ra, 0($sp)
; N64-NEXT: 	jr	$1
; N64-NEXT: 	daddiu	$sp, $sp, 16
; PURECAP-NEXT: cgetpcc	$c12
; PURECAP-NEXT: cincoffset	$c12, $c12, $1
; PURECAP-NEXT: cjr	$c12
; PURECAP-NEXT: nop
; CHECK-NEXT: .LBB1_3:
; CHECK-NEXT:  addiu	$4, $4, 3
; CHECK-NEXT:  #APP
; CHECK:       .space	1000000
; CHECK:       #NO_APP
; CHECK-NEXT:  .LBB1_4:
; N64-NEXT:      jr	$ra
; PURECAP-NEXT:  cjr	$c17
; CHECK-NEXT:  addiu	$2, $4, 2
; CHECK: .end needs_expand
