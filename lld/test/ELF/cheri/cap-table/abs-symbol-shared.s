# XFAIL: *
# TODO: need to look at x86 behaviour

# RUN: %cheri_purecap_llvm-mc -filetype=obj -defsym=MAIN_FILE=1 %s -o %t.o
# RUN: ld.lld --fatal-warnings -pie %t.o -o %t.exe
# RUN: llvm-readobj -dyn-relocations -t %t.exe
# RUN: ld.lld --fatal-warnings -shared %t.o -o %t.so
# RUN: llvm-readobj -dyn-relocations -t %t.so

# RUN: echo "weak_symbol = 64;" > %t.script
# RUN: ld.lld --fatal-warnings -pie -o %t.exe %t.o -T %t.script
# RUN: llvm-readobj -dyn-relocations -t %t.exe
# RUN: ld.lld --fatal-warnings -shared -o %t.so %t.o -T %t.script
# RUN: llvm-readobj -dyn-relocations -t %t.so

# what happens with x86?
# RUN: llvm-mc -triple=x86_64-unknown-freebsd -filetype=obj -defsym=MAIN_FILE=1 %s -o %t.o
# RUN: echo "weak_symbol = 64;" > %t.script
# RUN: ld.lld --fatal-warnings -pie -o %t.exe %t.o -T %t.script
# RUN: llvm-readobj -dyn-relocations -t %t.exe
# RUN: ld.lld --fatal-warnings -shared -o %t.so %t.o -T %t.script
# RUN: llvm-readobj -dyn-relocations -t %t.so

.text
.weak weak_symbol
.global __start
__start:
  nop
  # This symbol should be resolved by the linker script / abs assignment in the .o
  clcbi $c1, %captab20(weak_symbol)($c26)
