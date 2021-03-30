# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared -o %t.so %t.o -captable-scope=all
# RUN: llvm-readobj -r --cap-table %t.so

# CHECK: Relocations [
# CHECK-NEXT:   Section ({{.+}}) .rel.dyn {
# CHECK-NEXT:     0x30010 R_MIPS_TLS_DTPMOD64/R_MIPS_NONE/R_MIPS_NONE -{{$}}
# CHECK-NEXT:     0x30000 R_MIPS_TLS_DTPMOD64/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# CHECK-NEXT:     0x30008 R_MIPS_TLS_DTPREL64/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# CHECK-NEXT:     0x30020 R_MIPS_TLS_TPREL64/R_MIPS_NONE/R_MIPS_NONE foo{{$}}
# CHECK-NEXT:   }
# CHECK-NEXT: ]
# TODO: TLS relocations aren't dumped correctly yet but at least it shows that they exist
# CHECK-NEXT: CHERI .captable [
# CHECK-NEXT:   0x0      <unknown symbol>                R_MIPS_TLS_DTPMOD64/R_MIPS_NONE/R_MIPS_NONE against foo
# CHECK-NEXT:   0x10     <unknown symbol>                R_MIPS_TLS_DTPMOD64/R_MIPS_NONE/R_MIPS_NONE against
# CHECK-NEXT:   0x20     <unknown symbol>                R_MIPS_TLS_TPREL64/R_MIPS_NONE/R_MIPS_NONE against foo
# CHECK-NEXT: ]

# RUN: not ld.lld -shared -o %t.so %t.o -captable-scope=file 2>&1 | FileCheck -check-prefix ERR %s
# RUN: not ld.lld -shared -o %t.so %t.o -captable-scope=function 2>&1 | FileCheck -check-prefix ERR %s
# ERR: ld.lld: error: TLS is not supported yet with per-file or per-function captable

.text
__start:
  lui $2, %captab_tlsgd_hi(foo)
  daddiu $3, $2, %captab_tlsgd_lo(foo)
  lui $2, %captab_tlsldm_hi(foo)
  daddiu $3, $2, %captab_tlsldm_lo(foo)
  lui $2, %captab_tprel_hi(foo)
  daddiu $3, $2, %captab_tprel_lo(foo)
