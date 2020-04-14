## Previously having a linker-script defined symbol point before the start of the section would crash
## This was found when using linkcmds.base from RTEMS
## A simpler version exposing the same issue:
# RUN: echo "SECTIONS { \
# RUN:  .tdata : { \
# RUN:    _TLS_Data_begin = .; \
# RUN:    *(.tdata .tdata.*); \
# RUN:    _TLS_Data_end = .; \
# RUN:  } \
# RUN:  _TLS_Data_begin = _TLS_Data_begin - 8; \
# RUN:  _TLS_Data_end = _TLS_Data_end + 1; \
# RUN:  _TLS_Data_size = SIZEOF(.tdata); \
# RUN: }" > %t.ldscript

# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s --defsym=TDATA=1 -o %t-tdata.o

# RUN: ld.lld -T %t.ldscript %t.o -o %t-no-tdata.exe 2>&1 | FileCheck %s --check-prefix=WARN
# RUN: llvm-objdump -t -r --cap-relocs %t-no-tdata.exe | FileCheck %s --check-prefix=DUMP-NO-TDATA
# RUN: ld.lld -T %t.ldscript %t.o %t-tdata.o -o %t-with-tdata.exe 2>&1 | FileCheck %s --check-prefix=WARN
# RUN: llvm-objdump -t -r --cap-relocs %t-with-tdata.exe | FileCheck %s --check-prefix=DUMP-TDATA

.ifdef TDATA
.section .tdata
.8byte 0
.else
# empty (must exist to trigger this bug)
.section .tbss
.8byte 0

# WARN-NOT:  warning:
# WARN:      ld.lld: warning: Symbol <unknown kind> _TLS_Data_begin
# WARN-NEXT: >>> defined in <internal> is defined as being in section .tdata but the value ({{0xFFFFFFFFFFFFFFF8|0xA8}}) is outside this section. Will create a zero-size capability.
# WARN-NEXT: >>> referenced by <internal>:(.captable+0x0)
# WARN-EMPTY:
# WARN-NEXT: warning: Symbol <unknown kind> _TLS_Data_end
# WARN-NEXT: >>> defined in <internal> is defined as being in section .tdata but the value ({{0x1|0xB9}}) is outside this section. Will create a zero-size capability.
# WARN-NEXT: >>> referenced by <internal>:(.captable+0x10)
# WARN-EMPTY:
# WARN-NOT:  warning:

# DUMP-TDATA-LABEL: SYMBOL TABLE:
# DUMP-TDATA: 00000000000000a8 g       .tdata	0000000000000000 _TLS_Data_begin
# DUMP-TDATA: 00000000000000b9 g       .tdata	0000000000000000 _TLS_Data_end
# DUMP-TDATA: 0000000000000008 g       *ABS*	0000000000000000 _TLS_Data_size
# DUMP-TDATA-LABEL: CAPABILITY RELOCATION RECORDS:
# DUMP-TDATA-NEXT: 0x{{.+}} Base: _TLS_Data_begin (0x00000000000000a8)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x4000000000000000 (Constant)
# DUMP-TDATA-NEXT: 0x{{.+}} Base: _TLS_Data_end (0x00000000000000b9)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x4000000000000000 (Constant)
# DUMP-TDATA-EMPTY:

# DUMP-NO-TDATA-LABEL: SYMBOL TABLE:
# DUMP-NO-TDATA: fffffffffffffff8 g       .tdata	0000000000000000 _TLS_Data_begin
# DUMP-NO-TDATA: 0000000000000001 g       .tdata	0000000000000000 _TLS_Data_end
# DUMP-NO-TDATA: 0000000000000000 g       *ABS*	0000000000000000 _TLS_Data_size
# DUMP-NO-TDATA-LABEL: CAPABILITY RELOCATION RECORDS:
# DUMP-NO-TDATA-NEXT: 0x{{.+}} Base: _TLS_Data_begin (0xfffffffffffffff8)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x4000000000000000 (Constant)
# DUMP-NO-TDATA-NEXT: 0x{{.+}} Base: _TLS_Data_end (0x0000000000000001)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x4000000000000000 (Constant)
# DUMP-NO-TDATA-EMPTY:

.text
.global __start
.ent __start
__start:
  nop
  clcbi $c1, %captab20(_TLS_Data_begin)($cgp)
  nop
  clcbi $c2, %captab20(_TLS_Data_end)($cgp)
  nop
  ori $zero, $zero, %lo(_TLS_Data_size)
  nop
.end __start

.endif
