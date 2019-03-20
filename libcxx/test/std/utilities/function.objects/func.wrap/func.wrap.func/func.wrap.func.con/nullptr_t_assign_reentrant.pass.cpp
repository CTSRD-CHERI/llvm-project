//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// function& operator=(nullptr_t);

#include <functional>
#include <cassert>

#include "test_macros.h"

struct A
{
  static std::function<void()> global;
  static bool cancel;

  ~A() {
    DoNotOptimize(cancel);
    if (cancel)
      global = nullptr;
  }
  void operator()() {}
};

std::function<void()> A::global;
bool A::cancel = false;

int main()
{
  A::global = A();
#ifndef TEST_HAS_NO_RTTI
  assert(A::global.target<A>());
#endif

  // Check that we don't recurse in A::~A().
  A::cancel = true;
  A::global = nullptr;

#ifndef TEST_HAS_NO_RTTI
  assert(!A::global.target<A>());
#endif
}
