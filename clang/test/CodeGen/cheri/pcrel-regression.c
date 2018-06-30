// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -o - -O0 -emit-llvm -disable-O0-optnone  %s | FileCheck %s -check-prefix LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -o - -O0 -emit-llvm -disable-O0-optnone  %s | FileCheck %s -check-prefix CAPTABLE
// Also check that we can emit asm without crashing:
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -o /dev/null -O0 -S %s


// Calling a function with old-style prototypes would produce an invalid bitcast after commit 1f6d142a46bb24c067aacc50282388ee16065eed
void a(b) {
   a();
   // LEGACY: call void bitcast (void (i32)* @a to void ()*)()
   // CAPTABLE: call void bitcast (void (i32) addrspace(200)* @a to void () addrspace(200)*)()
}
