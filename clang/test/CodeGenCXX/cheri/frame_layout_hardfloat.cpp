// RUN: not --crash %cheri_purecap_cc1 -target-cpu mips4 -std=c++11 -fdeprecated-macro -fcxx-exceptions -fexceptions -o - -O0  -S %s
// REQUIRES: asserts
// Crashes with UNREACHABLE: Remaining virtual register operands

void a(double) {}
