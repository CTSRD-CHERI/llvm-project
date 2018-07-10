; Check that the CheriPureCapABI pass doesn't break debug info
; It previoulsy moved all the llvm.debug.declare statements from the alloca
; to the csetbounds which caused the processDbgDeclares() function in SelectionDAGISel.cpp
; to not insert the appropriate local variable entries

; RUNNOT: %cheri_purecap_llc %S/Inputs/purecap-debuginfo-test1.ll -O0 -filetype=obj -o - | llvm-dwarfdump -debug-info -
; RUN: %cheri_llc %S/Inputs/hybrid-debuginfo-test1.ll -O0 -filetype=obj -o - | llvm-dwarfdump -debug-info - | FileCheck %s -check-prefixes CHECK,MIPS
; RUN: %cheri_purecap_llc %S/Inputs/purecap-debuginfo-test1.ll -O0 -filetype=obj -o - | llvm-dwarfdump -debug-info - | FileCheck %s -check-prefixes CHECK,CHERI


// CHECK-LABEL: .debug_info contents:
// CHECK-NEXT: 0x00000000: Compile Unit:
// CHECK-EMPTY:
// CHECK-NEXT: DW_TAG_compile_unit
// CHECK-NEXT:               DW_AT_producer	("clang version
// CHECK-NEXT:               DW_AT_language	(DW_LANG_C99)
// CHECK-NEXT:               DW_AT_name	("
// CHECK-NEXT:               DW_AT_stmt_list	(0x00000000)
// CHECK-NEXT:               DW_AT_comp_dir	("
// CHECK-NEXT:               DW_AT_GNU_pubnames	(0x01)
// CHECK-NEXT:               DW_AT_low_pc	(0x0000000000000000)
// CHECK-NEXT:               DW_AT_high_pc	(0x00000000000{{.+}})
// CHECK-EMPTY:
// CHECK-NEXT: [[INT_TYPE_INFO_ADDR:0x0000002f]]: DW_TAG_base_type
// CHECK-NEXT:                 DW_AT_name	("int")
// CHECK-NEXT:                 DW_AT_encoding	(DW_ATE_signed)
// CHECK-NEXT:                 DW_AT_byte_size	(0x04)
// CHECK-EMPTY:
// CHECK-NEXT: DW_TAG_subprogram
// CHECK-NEXT:                 DW_AT_low_pc	(0x0000000000000000)
// CHECK-NEXT:                 DW_AT_high_pc	(0x00000000000000{{.+}})
// MIPS-NEXT:            DW_AT_frame_base	(DW_OP_reg29 SP_64)
// CHERI-NEXT:           DW_AT_frame_base	(DW_OP_regx C11)
// CHECK-NEXT:                 DW_AT_name	("foo")
// CHECK-NEXT:                 DW_AT_decl_file	("/src/test/CodeGen/cheri/cheri-debug-info.c")
// CHECK-NEXT:                 DW_AT_decl_line	(20)
// CHECK-NEXT:                 DW_AT_prototyped	(0x01)
// CHECK-NEXT:                 DW_AT_type	([[INT_TYPE_INFO_ADDR]] "int")
// CHECK-NEXT:                 DW_AT_external	(0x01)
// CHECK-EMPTY:
// CHECK-NEXT: DW_TAG_formal_parameter
// MIPS-NEXT:              DW_AT_location	(DW_OP_fbreg +16)
// CHERI-NEXT:             DW_AT_location	(DW_OP_fbreg +{{80|192}})
// CHECK-NEXT:                   DW_AT_name	("i")
// CHECK-NEXT:                   DW_AT_decl_file	("/src/test/CodeGen/cheri/cheri-debug-info.c")
// CHECK-NEXT:                   DW_AT_decl_line	(20)
// CHECK-NEXT:                   DW_AT_type	([[INTPTR_TYPE_INFO_ADDR:0x000.+]] "int*")
// CHECK-EMPTY:
// CHECK-NEXT: DW_TAG_lexical_block
// MIPS-NEXT:                   DW_AT_low_pc	(0x0000000000000028)
// CHERI-NEXT:                  DW_AT_low_pc	(0x000000000000004c)
// MIPS-NEXT:                   DW_AT_high_pc	(0x0000000000000034)
// CHERI-NEXT:                  DW_AT_high_pc	(0x000000000000005c)
// CHECK-EMPTY:
// CHECK-NEXT: DW_TAG_variable
// MIPS-NEXT:                      DW_AT_location	(DW_OP_fbreg +12)
// CHERI-NEXT:                     DW_AT_location	(DW_OP_fbreg +{{76|188}})
// CHECK-NEXT:                     DW_AT_name	("j")
// CHECK-NEXT:                     DW_AT_decl_file	("/src/test/CodeGen/cheri/cheri-debug-info.c")
// CHECK-NEXT:                     DW_AT_decl_line	(22)
// CHECK-NEXT:                     DW_AT_type	([[INT_TYPE_INFO_ADDR]] "int")
// CHECK-EMPTY:
// CHECK-NEXT:  NULL
// CHECK-EMPTY:
// CHECK-NEXT:  DW_TAG_lexical_block
// MIPS-NEXT:                   DW_AT_low_pc	(0x0000000000000038)
// CHERI-NEXT:                  DW_AT_low_pc	(0x0000000000000060)
// MIPS-NEXT:                   DW_AT_high_pc	(0x0000000000000044)
// CHERI-NEXT:                  DW_AT_high_pc	(0x0000000000000070)
// CHECK-EMPTY:
// CHECK-NEXT:  DW_TAG_variable
// MIPS-NEXT:                      DW_AT_location	(DW_OP_fbreg +8)
// CHERI-NEXT:                     DW_AT_location	(DW_OP_fbreg +{{72|184}})
// CHECK-NEXT:                     DW_AT_name	("j")
// CHECK-NEXT:                     DW_AT_decl_file	("{{.+}}/CodeGen/cheri/cheri-debug-info.c")
// CHECK-NEXT:                     DW_AT_decl_line	(25)
// CHECK-NEXT:                     DW_AT_type	([[INT_TYPE_INFO_ADDR]] "int")
// CHECK-EMPTY:
// CHECK-NEXT:   NULL
// CHECK-EMPTY:
// CHECK-NEXT:   NULL
// CHECK-EMPTY:
// CHECK-NEXT:  [[INTPTR_TYPE_INFO_ADDR]]:  DW_TAG_pointer_type
// CHECK-NEXT:                 DW_AT_type	([[INT_TYPE_INFO_ADDR]] "int")
// CHERI includes the type size since it is non-default:
// CHERI-NEXT:           DW_AT_byte_size ({{0x10|0x20}})
// CHECK-EMPTY:
// CHECK-NEXT:   NULL
// CHECK-EMPTY:
