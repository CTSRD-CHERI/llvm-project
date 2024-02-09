/// Check that  CHERI LLVM prohibits __int128 for 32-bit targets
// RUN: %riscv32_cheri_cc1 -fsyntax-only -verify=expected32 %s
// RUN: %riscv32_cheri_purecap_cc1 -fsyntax-only -verify=fixme32 %s
// RUN: %riscv32_cheri_cc1 -fsyntax-only -fforce-enable-int128 -verify=force-enable %s
// RUN: %riscv32_cheri_purecap_cc1 -fsyntax-only -fforce-enable-int128 -verify=force-enable %s
// RUN: %riscv64_cheri_cc1 -fsyntax-only -verify=expected64 %s
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only -verify=expected64 %s

// FIXME: Should not enable int128 for 32-bit purecap
// fixme32-no-diagnostics
// expected64-no-diagnostics
// force-enable-no-diagnostics

void test(__int128 arg); // expected32-error{{__int128 is not supported on this target}}
