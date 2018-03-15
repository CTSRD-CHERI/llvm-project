# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -defsym=MAIN=1 -o %t.o

# Linking with elf relocs should not work for static binaries
# RUN: not ld.lld -preemptible-caprelocs=elf -local-caprelocs=elf %t.o -o %t.exe 2>&1 | FileCheck %s -check-prefix ERR
# ERR: error: attempting to emit a R_CAPABILITY relocation against symbol __start in binary without a dynamic linker; try removing -Wl,-local-caprelocs=elf
# ERR-NEXT: >>> referenced by <unknown kind> capsym
# ERR-NEXT: >>> defined in {{.+}}dyn-relocs-in-exe.s.tmp.o:(.data+0x0)


# But -pie is fine
# RUN: ld.lld -pie -preemptible-caprelocs=elf -local-caprelocs=elf %t.o -o %t.exe
# RUN: llvm-readobj -dyn-relocations %t.exe | FileCheck %s -check-prefix DYN-RELOCS

# If we link at least one shared library we will have a dynamic linker
# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -defsym=SHLIB=1 -o %t2.o
# RUN: ld.lld -shared %t2.o -o %t.so
# RUN: ld.lld -preemptible-caprelocs=elf -local-caprelocs=elf %t.o %t.so -o %t2.exe
# RUN: llvm-readobj -dyn-relocations %t2.exe | FileCheck %s -check-prefix DYN-RELOCS


.ifdef MAIN
.global __start
__start:
  nop

.data
.global capsym
capsym:
.chericap __start

.endif


.ifdef SHLIB
.global dummy_shlib
dummy_shlib:
  nop
.endif

# DYN-RELOCS:      Dynamic Relocations {
# DYN-RELOCS-NEXT:   {{0x120020000|0x20000}} R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __start 0x0
# DYN-RELOCS-NEXT: }
