// RUN: llvm-mc -triple riscv64 -mattr=+zcheripurecap -filetype obj -o - %s | llvm-readobj --symbols - | FileCheck %s
// RUN: llvm-mc -triple riscv64 -mattr=+zcheripurecap -filetype obj -defsym=OBJ=1 -o - %s | llvm-readelf -s - | FileCheck %s --check-prefix=OBJ
// RUN: not llvm-mc -triple riscv64 -mattr=+zcheripurecap -filetype asm -defsym=ERR=1 -o - %s 2>&1 | FileCheck %s --check-prefix=ERR

.text
.dont_seal local
local:

// CHECK: Name: local
// CHECK: Other [ (0x40)

.ifdef OBJ
/// Binding directive before .dont_seal.
.global def1
.dont_seal def1
def1:

/// Binding directive after .dont_seal.
.dont_seal def2
.weak def2
def2:

.globl alias_def1
.set alias_def1, def1

.dont_seal undef

// OBJ:      NOTYPE LOCAL  DEFAULT [DONT_SEAL] [[#]] local
// OBJ-NEXT: NOTYPE GLOBAL DEFAULT [DONT_SEAL] [[#]] def1
// OBJ-NEXT: NOTYPE WEAK   DEFAULT [DONT_SEAL] [[#]] def2
// OBJ-NEXT: NOTYPE GLOBAL DEFAULT               [[#]] alias_def1
// OBJ-NEXT: NOTYPE GLOBAL DEFAULT [DONT_SEAL] UND   undef
.endif

.ifdef ERR
.dont_seal
// ERR: [[#@LINE-1]]:11: error: expected symbol name

.global fox
.dont_seal fox bar
// ERR: [[#@LINE-1]]:16: error: expected newline
.endif

