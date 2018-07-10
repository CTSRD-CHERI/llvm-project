// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -emit-llvm -o - %s | FileCheck %s -check-prefix PCREL
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-llvm -o - %s | FileCheck %s -check-prefix LEGACY
// Test that the cheri-direct-calls LLVM pass inserts a bitcast for when the
// type of the called function differs from the arguments that are passed.
// Ideally this should be in the LLVM unit tests but it doesn't seem to be
// possible to create IR whereby the arguments passed differs from what is
// allowed as per the function prototype.
void a() {
  // LEGACY: call void bitcast (void ()* @a to void (i32)*)(i32 signext 3)
  // PCREL: call void bitcast (void () addrspace(200)* @a to void (i32) addrspace(200)*)(i32 signext 3)
  a(3);
}
