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

// function& operator=(function &&);

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
      global = std::function<void()>(nullptr);
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
  A::global = std::function<void()>(nullptr);
#ifndef TEST_HAS_NO_RTTI
  assert(!A::global.target<A>());
#endif
}
