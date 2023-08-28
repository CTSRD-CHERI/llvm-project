// taken from temporaries.cpp (which crashed when run with target cheri)
// RUN: %cheri_cc1 -fno-rtti -target-abi purecap -std=c++11 -DCHECK_ERROR -fsyntax-only -verify %s
// RUN: %cheri_cc1 -fno-rtti -mllvm -cheri-cap-table-abi=pcrel -emit-llvm %s -o -  -target-abi purecap -std=c++11 | FileCheck %s

namespace PR20227 {
struct A {
  ~A();
};
struct B {
  virtual ~B();
};
struct C : B {};

A &&a = dynamic_cast<A &&>(A{}); // this is valid even with -fno-rtti as it only does lifetime extension
// CHECK: @_ZN7PR202271aE = addrspace(200) global ptr addrspace(200) null, align 16
// CHECK: @_ZGRN7PR202271aE_ = internal addrspace(200) global %"struct.PR20227::A" zeroinitializer, align 1

#ifdef CHECK_ERROR
// RTTI should be off by default with CHERI
B &&b = dynamic_cast<C &&>(dynamic_cast<B &&>(C{})); // expected-error {{use of dynamic_cast requires -frtti}}
#endif

B &&c = static_cast<C &&>(static_cast<B &&>(C{}));
// CHECK: @_ZN7PR202271cE = addrspace(200) global ptr addrspace(200) null, align 16
// CHECK: @_ZGRN7PR202271cE_ = internal addrspace(200) global %"struct.PR20227::C" zeroinitializer, align 16
} // namespace PR20227

// CHECK-LABEL: define internal void @__cxx_global_var_init()
// CHECK: call i32 @__cxa_atexit(ptr addrspace(200) @_ZN7PR202271AD1Ev,
// CHECK-SAME:  ptr addrspace(200) @_ZGRN7PR202271aE_, ptr addrspace(200) @__dso_handle) #2
// CHECK:   store ptr addrspace(200) @_ZGRN7PR202271aE_, ptr addrspace(200) @_ZN7PR202271aE, align 16
// CHECK:   ret void

// CHECK: declare i32 @__cxa_atexit(ptr addrspace(200), ptr addrspace(200), ptr addrspace(200))

// CHECK-LABEL: define internal void @__cxx_global_var_init.1()
// CHECK:    call void @llvm.memset.p200.i64(ptr addrspace(200) align 16 @_ZGRN7PR202271cE_, i8 0, i64 16, i1 false)
// CHECK:    call void @_ZN7PR202271CC1Ev(ptr addrspace(200) noundef nonnull align 16 dereferenceable(16) @_ZGRN7PR202271cE_)
// CHECK:    call i32 @__cxa_atexit(ptr addrspace(200) @_ZN7PR202271CD1Ev,
// CHECK-SAME:  ptr addrspace(200) @_ZGRN7PR202271cE_, ptr addrspace(200) @__dso_handle) #2
// CHECK:    store ptr addrspace(200) @_ZGRN7PR202271cE_, ptr addrspace(200) @_ZN7PR202271cE, align 16
// CHECK:    ret void
