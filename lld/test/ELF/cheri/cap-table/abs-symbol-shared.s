# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld --fatal-warnings -pie %t.o -o %t.exe
# RUN: llvm-readobj --dyn-relocations -t %t.exe | FileCheck %s --check-prefix=INTERPOSABLE
# RUN: ld.lld --fatal-warnings -shared %t.o -o %t.so
# RUN: llvm-readobj --dyn-relocations -t %t.so | FileCheck %s --check-prefix=INTERPOSABLE

# RUN: echo "weak_symbol = 64;" > %t.script
# RUN: ld.lld --fatal-warnings -pie -o %t.exe %t.o -T %t.script
# RUN: llvm-readobj --dyn-relocations -t %t.exe | FileCheck %s --check-prefix=DEFINED-EXE
# RUN: ld.lld --fatal-warnings -shared -o %t.so %t.o -T %t.script
# RUN: llvm-readobj --dyn-relocations -t %t.so | FileCheck %s --check-prefix=INTERPOSABLE


## Check that there is a dynamic relocation against the weak symbol
## even though it is defined here.
# INTERPOSABLE-LABEL: Dynamic Relocations {
# INTERPOSABLE-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE weak_symbol{{$}}
# INTERPOSABLE-NEXT:  }
## However, in an executable where it is defined there is no need for dynamic relocation:
# DEFINED-EXE-LABEL: Dynamic Relocations {
# DEFINED-EXE-NEXT:  }

.text
.weak weak_symbol
.global __start
__start:
  nop
  # This symbol should be resolved by the linker script / abs assignment in the .o
  clcbi $c1, %captab20(weak_symbol)($c26)
