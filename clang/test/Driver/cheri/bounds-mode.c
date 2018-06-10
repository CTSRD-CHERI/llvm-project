// Check that we accept all the values for -cheri-bounds= (CC1 option)

// RUN: %cheri_purecap_cc1 -cheri-bounds=conservative -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=references-only -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=subobject-safe -emit-llvm %s -o /dev/null
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -emit-llvm %s -o /dev/null

// RUN: not %cheri_purecap_cc1 -cheri-bounds=typo -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s -check-prefix ERR
// ERR: error: invalid value 'typo' in '-cheri-bounds=typo'
