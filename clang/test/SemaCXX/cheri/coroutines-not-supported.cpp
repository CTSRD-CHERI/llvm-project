/// C++20 coroutines are not yet supported for purecap.
/// Check that we do not accept the new keywords even in C++20 mode.
/// See https://github.com/CTSRD-CHERI/llvm-project/issues/717

// RUN: %riscv64_cheri_cc1 -fsyntax-only %s -std=c++20 -verify=hybrid
// RUN: %riscv64_cheri_cc1 -fsyntax-only %s -std=c++17 -verify=no-coroutines
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only %s -std=c++20 -verify=no-coroutines
// hybrid-no-diagnostics

#include "../Inputs/std-coroutine.h"

struct promise_void {
  void get_return_object();
  std::suspend_always initial_suspend();
  std::suspend_always final_suspend() noexcept;
  void return_void();
  void unhandled_exception();
};

template <>
struct std::coroutine_traits<void> { using promise_type = promise_void; };

void test() {
  co_return; // no-coroutines-error{{use of undeclared identifier 'co_return'; did you mean 'return'?}}
}
