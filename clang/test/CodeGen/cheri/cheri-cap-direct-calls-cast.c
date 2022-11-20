// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s -disable-llvm-optzns -O0 | FileCheck %s -check-prefix PCREL
// Test that Clang inserts a bitcast for when the type of the called function
// differs from the arguments that are passed.
// Ideally this should be in the LLVM unit tests but it doesn't seem to be
// possible to create IR whereby the arguments passed differs from what is
// allowed as per the function prototype.
void a() {
  // PCREL: call void bitcast (void () addrspace(200)* @a to void (i32) addrspace(200)*)(i32 noundef signext 3)
  a(3);
}
