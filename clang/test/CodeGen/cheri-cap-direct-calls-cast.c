// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s
// Test that the cheri-direct-calls LLVM pass inserts a bitcast for when the
// type of the called function differs from the arguments that are passed.
// Ideally this should be in the LLVM unit tests but it doesn't seem to be
// possible to create IR whereby the arguments passed differs from what is
// allowed as per the function prototype.
void a() {
  // CHECK: call void bitcast (void ()* @a to void (i32)*)(i32 signext 3)
  a(3);
}
