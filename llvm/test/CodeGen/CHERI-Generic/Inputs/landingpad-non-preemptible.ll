; RUN: llc @PURECAP_HARDFLOAT_ARGS@ --relocation-model=pic < %s -o - | FileCheck %s
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ --relocation-model=pic < %s -o - -filetype=obj | llvm-readobj --relocs --symbols - | FileCheck %s --check-prefix=RELOCS
; Capabilities for exception landing pads were using preemptible relocations such as
; .chericap foo + .Ltmp - .Lfunc_begin instead of using a local alias.
; https://github.com/CTSRD-CHERI/llvm-project/issues/512
; This test case was generated from the following C++ code:
; extern long foo();
; int do_catch() {
;     try {
;         return foo();
;     } catch(int &i) {
;         return 1;
;     } catch(...) {
;         return 2;
;     }
; }

@_ZTIi = external dso_local addrspace(200) constant ptr addrspace(200)
define dso_local noundef signext i32 @_Z8do_catchv() local_unnamed_addr addrspace(200) #0 personality ptr addrspace(200) @__gxx_personality_v0 {
entry:
  %call = invoke noundef signext i32 @_Z3foov()
          to label %return unwind label %lpad

lpad:                                             ; preds = %entry
  %0 = landingpad { ptr addrspace(200), i32 }
          catch ptr addrspace(200) @_ZTIi
          catch ptr addrspace(200) null
  %1 = extractvalue { ptr addrspace(200), i32 } %0, 0
  %2 = extractvalue { ptr addrspace(200), i32 } %0, 1
  %3 = tail call i32 @llvm.eh.typeid.for(ptr addrspacecast (ptr addrspace(200) @_ZTIi to ptr)) nounwind
  %matches = icmp eq i32 %2, %3
  %4 = tail call ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200) %1) nounwind
  br i1 %matches, label %catch1, label %catch

catch1:                                           ; preds = %lpad
  tail call void @__cxa_end_catch() nounwind
  br label %return

catch:                                            ; preds = %lpad
  tail call void @__cxa_end_catch()
  br label %return

return:                                           ; preds = %entry, %catch1, %catch
  %retval.0 = phi i32 [ 1, %catch1 ], [ 2, %catch ], [ %call, %entry ]
  ret i32 %retval.0
}

declare dso_local i32 @_Z3foov() local_unnamed_addr addrspace(200)

declare dso_local i32 @__gxx_personality_v0(...) addrspace(200)

declare i32 @llvm.eh.typeid.for(i8*) addrspace(200) nounwind readnone

declare dso_local ptr addrspace(200) @__cxa_begin_catch(ptr addrspace(200)) local_unnamed_addr addrspace(200)

declare dso_local void @__cxa_end_catch() local_unnamed_addr addrspace(200)

; UTC_ARGS: --disable
; CHECK: .Lfunc_end0:
; CHECK-NEXT: .size _Z8do_catchv, .Lfunc_end0-_Z8do_catchv
; CHECK-NEXT: .size .L_Z8do_catchv$local, .Lfunc_end0-_Z8do_catchv

; CHECK:      GCC_except_table0:
; CHECK-NEXT: .Lexception0:
; CHECK-NEXT:  .byte 255                             # @LPStart Encoding = omit
; CHECK-NEXT:  .byte 155                             # @TType Encoding = indirect pcrel sdata4
; CHECK-NEXT:  .uleb128 .Lttbase0-.Lttbaseref0
; CHECK-NEXT: .Lttbaseref0:
; RISC-V uses DW_EH_PE_udata4 instead of uleb128 since uleb128 causes issues with linker relaxations.
@IF-RISCV@; CHECK-NEXT:     .byte 3                   # Call site Encoding = udata4
@IFNOT-RISCV@; CHECK-NEXT:  .byte 1                   # Call site Encoding = uleb128
; CHECK-NEXT:  .uleb128 .Lcst_end0-.Lcst_begin0
; CHECK-NEXT: .Lcst_begin0:
; CHECK-NEXT:  [[CS_DIRECTIVE:(\.uleb128)|(\.word)]] .Ltmp0-.Lfunc_begin0           # >> Call Site 1 <<
; CHECK-NEXT:  [[CS_DIRECTIVE]] .Ltmp1-.Ltmp0                  #   Call between .Ltmp0 and .Ltmp1
; Note: RISC-V uses DW_EH_PE_udata4, so the 0xc marker uses 4 bytes instead of 1
; CHECK-NEXT:  [[SMALL_CS_DIRECTIVE:(\.byte)|(\.word)]] 12     # (landing pad is a capability)
; Note: the following line should not be using _Z8do_catchv, but a local alias
; CHECK-NEXT:  .chericap  .L_Z8do_catchv$local+(.Ltmp2-.Lfunc_begin0)   #     jumps to .Ltmp2
; CHECK-NEXT:  .byte 3                               #   On action: 2
; CHECK-NEXT:  [[CS_DIRECTIVE]] .Ltmp1-.Lfunc_begin0           # >> Call Site 2 <<
; CHECK-NEXT:  [[CS_DIRECTIVE]] .Lfunc_end0-.Ltmp1             #   Call between .Ltmp1 and .Lfunc_end0
; CHECK-NEXT:  [[SMALL_CS_DIRECTIVE]] 0                        #     has no landing pad
; CHECK-NEXT:  .byte 0                               #   On action: cleanup
; CHECK-NEXT: .Lcst_end0:
; CHECK-NEXT:  .byte 1                               # >> Action Record 1 <<
; CHECK-NEXT:                                         #   Catch TypeInfo 1
; CHECK-NEXT:  .byte 0                               #   No further actions
; CHECK-NEXT:  .byte 2                               # >> Action Record 2 <<
; CHECK-NEXT:                                         #   Catch TypeInfo 2
; CHECK-NEXT:  .byte 125                             #   Continue to action 1
; CHECK-NEXT:  .p2align 2
; CHECK-NEXT:                                         # >> Catch TypeInfos <<
; CHECK-NEXT: [[TI_LABEL:\.Ltmp[0-9]+]]:          # TypeInfo 2
; CHECK-NEXT:  .{{4byte|word}} .L_ZTIi.DW.stub-[[TI_LABEL]]
; CHECK-NEXT:  .{{4byte|word}} 0                      # TypeInfo 1
; CHECK-NEXT: .Lttbase0:
; CHECK-NEXT:  .p2align 2
; CHECK-NEXT:                                         # -- End function




; RELOCS-LABEL: Relocations [
; RELOCS-LABEL:  Section ({{.+}}) .rela.gcc_except_table {
@IF-MIPS@; RELOCS-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE  .L_Z8do_catchv$local 0x4C
@IF-MIPS@; RELOCS-NEXT:    R_MIPS_PC32/R_MIPS_NONE/R_MIPS_NONE .L_ZTIi.DW.stub 0x0
@IF-RISCV@; RELOCS-NEXT:   R_RISCV_CHERI_CAPABILITY  .L_Z8do_catchv$local 0x34
@IF-RISCV@; RELOCS-NEXT:   R_RISCV_ADD32 <null> 0x0
@IF-RISCV@; RELOCS-NEXT:   R_RISCV_SUB32 <null> 0x0
@IF-RISCV@; RELOCS-NEXT:   R_RISCV_ADD32 .L_ZTIi.DW.stub 0x0
@IF-RISCV@; RELOCS-NEXT:   R_RISCV_SUB32 <null> 0x0
; RELOCS-NEXT:  }

; The local alias should have the same type and non-zero size as the real function:
; RELOCS:       Symbol {
; RELOCS-LABEL:   Name:  .L_Z8do_catchv$local (
; RELOCS-NEXT:    Value: 0x0
; RELOCS-NEXT:    Size: [[FN_SIZE:[1-9][0-9]*]]
; RELOCS-NEXT:    Binding: Local (0x0)
; RELOCS-NEXT:    Type: Function (0x2)
; RELOCS-NEXT:    Other: 0
; RELOCS-NEXT:    Section: .text (0x2)
; RELOCS-NEXT:  }
; RELOCS:       Symbol {
; RELOCS-LABEL:   Name: _Z8do_catchv (
; RELOCS-NEXT:    Value: 0x0
; RELOCS-NEXT:    Size: [[FN_SIZE]]
; RELOCS-NEXT:    Binding: Global (0x1)
; RELOCS-NEXT:    Type: Function (0x2)
; RELOCS-NEXT:    Other: 0
; RELOCS-NEXT:    Section: .text (0x2)
; RELOCS-NEXT:  }
