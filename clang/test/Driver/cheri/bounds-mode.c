// Check that we accept all the values for -cheri-bounds= (CC1 option)

// RUN: %cheri_purecap_cc1 -cheri-bounds=conservative -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=references-only -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=subobject-safe -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=very-aggressive -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=everywhere-unsafe -emit-llvm %s -o /dev/null

// RUN: not %cheri_purecap_cc1 -cheri-bounds=typo -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s -check-prefix ERR
// ERR: error: invalid value 'typo' in '-cheri-bounds=typo'

// RUN: %cheri_purecap_clang -E -dM -Xclang -cheri-bounds=subobject-safe %s 2>&1 | FileCheck --check-prefixes=CHECk,SUBOBJECT-SAFE %s
// RUN: %cheri_purecap_clang -E -dM -Xclang -cheri-bounds=conservative %s 2>&1 | FileCheck --check-prefixes=CHECK,SUBOBJECT-DISABLED %s
// Without any flag subobject bounds should currently be disabled
// RUN: %cheri_purecap_clang -E -dM %s 2>&1 | FileCheck --check-prefixes=SUBOBJECT-DISABLED %s
// CHECK: #define __CHERI_PURE_CAPABILITY__ 2
// SUBOBJECT-DISABLED-NOT: __CHERI_SUBOBJECT_BOUNDS__
// SUBOBJECT-SAFE: #define __CHERI_SUBOBJECT_BOUNDS__ 2
