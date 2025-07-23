# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -defsym=MAIN=1 -o %t.o

# Linking with elf relocs should not work for static binaries
# RUN: not ld.lld -local-caprelocs=elf %t.o -o %t.exe 2>&1 | FileCheck %s -check-prefix ERR
# ERR: error: local-cap-relocs=elf is not implemented yet
# ERRTODO: error: attempting to emit a R_CAPABILITY relocation against symbol __start in binary without a dynamic linker; try removing -Wl,-local-caprelocs=elf
# ERRTODO-NEXT: >>> referenced by <unknown kind> capsym
# ERRTODO-NEXT: >>> defined in {{.+}}dyn-relocs-in-exe.s.tmp.o:(.data+0x0)


# But -pie is fine
# RUN: ld.lld -pie %t.o -o %t.exe
# RUN: llvm-readobj --dyn-relocations --cap-relocs %t.exe | FileCheck %s --check-prefix DYN-RELOCS

# If we link at least one shared library we will have a dynamic linker
# RUN: %cheri128_purecap_llvm-mc %s -filetype=obj -defsym=SHLIB=1 -o %t2.o
# RUN: ld.lld -shared %t2.o -o %t.so
# RUN: ld.lld %t.o %t.so -o %t2.exe
# RUN: llvm-readobj --dyn-relocations --cap-relocs %t2.exe | FileCheck %s --check-prefix DYN-RELOCS


.ifdef MAIN
.global __start
.ent __start
__start:
  nop
.end __start

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
# DYN-RELOCS-NEXT: }
# DYN-RELOCS:      CHERI __cap_relocs [
# DYN-RELOCS-NEXT:   0x{{.+}} (capsym) Base: 0x{{.+}} (__start+0) Length: 4 Perms: Function
# DYN-RELOCS:      ]
