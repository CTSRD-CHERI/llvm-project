# Test that weak symbols defined by linker scripts are written correctly
# RUN: %cheri_purecap_llvm-mc -filetype=obj -defsym=MAIN_FILE=1 %s -o %t.o

# RUN: ld.lld --fatal-warnings -o %t.exe %t.o
# RUN: llvm-readobj -dyn-relocations -t %t.exe | FileCheck %s -check-prefixes STATIC-COMMON,UNDEFINED
# STATIC-COMMON:       Dynamic Relocations {
# STATIC-COMMON-NEXT:  }
# STATIC-COMMON:       Symbol {
# STATIC-COMMON:         Name: weak_symbol@CAPTABLE (
# STATIC-COMMON-NEXT:    Value: 0x120030000
# STATIC-COMMON-NEXT:    Size: {{16|32}}
# STATIC-COMMON-NEXT:    Binding: Local (0x0)
# STATIC-COMMON-NEXT:    Type: Object (0x1)
# STATIC-COMMON-NEXT:    Other: 0
# STATIC-COMMON-NEXT:    Section: .cap_table
# STATIC-COMMON-NEXT:  }
# UNDEFINED:       Symbol {
# UNDEFINED:         Name: weak_symbol (
# UNDEFINED-NEXT:    Value: 0x0
# UNDEFINED-NEXT:    Size: 0
# UNDEFINED-NEXT:    Binding: Weak (0x2)
# UNDEFINED-NEXT:    Type: None (0x0)
# UNDEFINED-NEXT:    Other: 0
# UNDEFINED-NEXT:    Section: Undefined (0x0)
# UNDEFINED-NEXT:  }


# An absolute symbol defined in the linker script is fine
# RUN: echo "weak_symbol = 64;" > %t.script
# RUN: ld.lld --fatal-warnings -o %t.exe %t.o -T %t.script
# RUN: llvm-readobj -dyn-relocations -t %t.exe | FileCheck %s -check-prefixes STATIC-COMMON,ABSOLUTE '-D$ABS_VALUE=0x40'
# ABSOLUTE:       Symbol {
# ABSOLUTE:         Name: weak_symbol (
# ABSOLUTE-NEXT:    Value: [[$ABS_VALUE]]
# ABSOLUTE-NEXT:    Size: 0
# ABSOLUTE-NEXT:    Binding: Global (0x1)
# ABSOLUTE-NEXT:    Type: None (0x0)
# ABSOLUTE-NEXT:    Other: 0
# ABSOLUTE-NEXT:    Section: Absolute (0xFFF1)
# ABSOLUTE-NEXT:  }

# And an absolute symbol set in the .o file is also fine
# RUN: %cheri_purecap_llvm-mc -filetype=obj -defsym=ABS_SYM_IN_FILE=1 %s -o %t2.o
# RUN: llvm-objdump -t %t2.o | FileCheck %s -check-prefix ABS_IN_FILE
# ABS_IN_FILE: 0000000000000050         *ABS*		 00000000 weak_symbol
# RUN: ld.lld --fatal-warnings -o %t.exe %t.o %t2.o
# RUN: llvm-readobj -dyn-relocations -t %t.exe | FileCheck %s -check-prefixes STATIC-COMMON,ABSOLUTE '-D$ABS_VALUE=0x50'


# Similarly an absolute -defsym= should also work:
# RUN: %cheri_purecap_llvm-mc -filetype=obj -defsym=ABS_SYM_IN_DEFSYM=1 -defsym=weak_symbol=0x60 %s -o %t2.o
# RUN: llvm-objdump -t %t2.o | FileCheck %s -check-prefix ABS_IN_DEFSYM
# ABS_IN_DEFSYM: 0000000000000060         *ABS*		 00000000 weak_symbol
# RUN: ld.lld --fatal-warnings -o %t.exe %t.o %t2.o
# RUN: llvm-readobj -dyn-relocations -t %t.exe | FileCheck %s -check-prefixes STATIC-COMMON,ABSOLUTE '-D$ABS_VALUE=0x60'


# A real symbol without a linker script
# RUN: %cheri_purecap_llvm-mc -filetype=obj -defsym=RESOLVE_FILE=1 -defsym=RESOLVE_FILE_SET_SIZE=1 %s -o %t2.o
# RUN: llvm-objdump -t %t2.o | FileCheck %s -check-prefix DEFINED_WITH_SIZE
# DEFINED_WITH_SIZE: 0000000000000000 .data 00000008 weak_symbol
# RUN: ld.lld --fatal-warnings -o %t.exe %t.o %t2.o
# RUN: llvm-readobj -dyn-relocations -t %t.exe | FileCheck %s -check-prefixes STATIC-COMMON,RESOLVED '-D$RESOLVED_SIZE=8'
# RESOLVED:       Symbol {
# RESOLVED:         Name: weak_symbol (
# RESOLVED-NEXT:    Value: 0x120020000
# RESOLVED-NEXT:    Size: [[$RESOLVED_SIZE]]
# RESOLVED-NEXT:    Binding: Global (0x1)
# RESOLVED-NEXT:    Type: None (0x0)
# RESOLVED-NEXT:    Other: 0
# RESOLVED-NEXT:    Section: .data (
# RESOLVED-NEXT:  }

# A real symbol without a size should give a warning:
# RUN: %cheri_purecap_llvm-mc -filetype=obj -defsym=RESOLVE_FILE=1 -defsym=RESOLVE_FILE_SET_SIZE=0 %s -o %t2.o
# RUN: llvm-objdump -t %t2.o | FileCheck %s -check-prefix DEFINED_NO_SIZE
# DEFINED_NO_SIZE: 0000000000000000 .data 00000000 weak_symbol
# --fatal-warnings should give an error:
# RUN: not ld.lld --fatal-warnings -o %t.exe %t.o %t2.o 2>&1 | FileCheck %s -check-prefix ERROR
# ERROR: ld.lld: error: could not determine size of cap reloc against <unknown kind> weak_symbol
# ERROR-NEXT: >>> defined in  ({{.+}}weak-symbols-2.s.tmp2.o:(.data+0x0))
# ERROR-NEXT: >>> referenced by <internal>:(.cap_table+0x0)
# RUN: ld.lld --no-fatal-warnings -o %t.exe %t.o %t2.o 2>%t.warnings
# RUN: FileCheck %s -check-prefix WARNING < %t.warnings
# WARNING: ld.lld: warning: could not determine size of cap reloc against <unknown kind> weak_symbol
# WARNING-NEXT: >>> defined in  ({{.+}}weak-symbols-2.s.tmp2.o:(.data+0x0))
# WARNING-NEXT: >>> referenced by <internal>:(.cap_table+0x0)
# RUN: llvm-readobj -dyn-relocations -t %t.exe | FileCheck %s -check-prefixes STATIC-COMMON,RESOLVED '-D$RESOLVED_SIZE=0'

.ifdef MAIN_FILE
.text
.weak weak_symbol
.global __start
__start:
  nop
  # This symbol should be resolved by the linker script / abs assignment in the .o
  clcbi $c1, %captab20(weak_symbol)($c26)
.else
# Add some dummy data
.text
nop
.endif


.ifdef ABS_SYM_IN_FILE
.global weak_symbol
weak_symbol = 0x50
.endif

.ifdef ABS_SYM_IN_DEFSYM
.global weak_symbol
.endif

.ifdef RESOLVE_FILE
.data
.global weak_symbol
weak_symbol:
.8byte 0
.if RESOLVE_FILE_SET_SIZE
.size weak_symbol, 8
.endif
.endif
