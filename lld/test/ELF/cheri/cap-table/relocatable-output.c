// REQUIRES: clang, mips

// RUN: %cheri_purecap_cc1 -DFIRST -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t1.o
// RUN: %cheri_purecap_cc1 -DSECOND -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t2.o


// Test that we can link the two files into an executable
// RUN: ld.lld  -o - %t1.o %t2.o | llvm-objdump -t -d -

// Now check that we don't get a duplicate symbol _CHERI_CAPABILITY_TABLE_ when using -r
// RUN: ld.lld -r -o %t-combined.o %t1.o %t2.o
// RUN: llvm-objdump -t %t-combined.o | FileCheck %s --check-prefix RELOCATABLE-SYMS
// RELOCATABLE-SYMS: 0000000000000000 *UND* 0000000000000000 _CHERI_CAPABILITY_TABLE_
// RUN: ld.lld -o %t.exe %t-combined.o
// This previously failed with a duplicate _CHERI_CAPABILITY_TABLE_ symbol error
// RUN: llvm-objdump -t %t.exe | FileCheck %s
// CHECK: 0000000000030320 l        .captable		 00000000000000{{3|6}}0 .hidden _CHERI_CAPABILITY_TABLE_

#ifdef FIRST
int global = 1;
long global2 = 3;

int __start(void) { return global + global2; }
#endif
#ifdef SECOND
extern char _CHERI_CAPABILITY_TABLE_[];
void *get_captable(void) { return &_CHERI_CAPABILITY_TABLE_; }
#endif
