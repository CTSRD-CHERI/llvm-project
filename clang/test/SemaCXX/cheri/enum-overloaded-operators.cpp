// RUN: %cheri_cc1 -x c++ %s -target-abi n64 -std=c++11 -o - -ast-dump | FileCheck %s
// This used to crash the compiler when targeting a non-cheri triple because the
// overloaded builtin operators for __(u)intcap_t were added unconditionally
// RUN: %clang_cc1 -x c++ %s -triple mips64-unknown-freebsd12.0 -target-cpu mips4 -target-abi n64 -std=c++11 -o - -ast-dump > /dev/null

enum dwarf_data_relative
{
    /// Value is omitted
      DW_EH_PE_omit     = 0xff,
    /// Value relative to program counter
      DW_EH_PE_pcrel    = 0x10,
    /// Value relative to the text segment
      DW_EH_PE_textrel  = 0x20,
    /// Value relative to the data segment
      DW_EH_PE_datarel  = 0x30,
    /// Value relative to the start of the function
      DW_EH_PE_funcrel  = 0x40,
    /// Aligned pointer (Not supported yet - are they actually used?)
      DW_EH_PE_aligned  = 0x50,
    /// Pointer points to address of real value
      DW_EH_PE_indirect = 0x80
};

static int is_indirect(unsigned char x)
{
  // CHECK-LABEL: `-FunctionDecl {{.+}} is_indirect 'int (unsigned char)' static
  // CHECK: `-BinaryOperator {{.+}} <col:11, col:38> 'bool' '=='
  // CHECK:     `-BinaryOperator {{.+}} <col:12, col:16> 'int' '&'
  // CHECK:        `-ImplicitCastExpr {{.+}} <col:16> 'int' <IntegralCast>
  // CHECK-NEXT:    `-DeclRefExpr {{.+}} <col:16> 'dwarf_data_relative' EnumConstant {{.+}} 'DW_EH_PE_indirect' 'dwarf_data_relative'
  // CHECK:      `-ImplicitCastExpr {{.+}} <col:38> 'int' <IntegralCast>
  // CHECK-NEXT:  `-DeclRefExpr {{.+}} <col:38> 'dwarf_data_relative' EnumConstant {{.+}} 'DW_EH_PE_indirect' 'dwarf_data_relative'


  return ((x & DW_EH_PE_indirect) == DW_EH_PE_indirect);
}
