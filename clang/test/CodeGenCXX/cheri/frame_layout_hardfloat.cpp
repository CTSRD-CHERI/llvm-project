// RUN: %cheri_purecap_cc1 -target-cpu mips4 -std=c++11 -fdeprecated-macro -fcxx-exceptions -fexceptions -o - -O0  -S %s
// REQUIRES: asserts

void a(double) {}
