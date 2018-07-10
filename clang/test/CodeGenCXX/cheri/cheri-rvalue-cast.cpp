// taken from temporaries.cpp (which crashed when run with target cheri)

// RUN: %cheri_cc1 -fno-rtti -target-abi purecap -std=c++11 -DCHECK_ERROR -fsyntax-only -verify %s
// RUN: %cheri_cc1 -fno-rtti -mllvm -cheri-cap-table-abi=legacy -emit-llvm %s -o -  -target-abi purecap -std=c++11 | %cheri_FileCheck %s -check-prefixes CHECK,LEGACY
// RUN: %cheri_cc1 -fno-rtti -mllvm -cheri-cap-table-abi=pcrel -emit-llvm %s -o -  -target-abi purecap -std=c++11 | %cheri_FileCheck %s -check-prefixes CHECK,NEWABI


namespace PR20227 {
  struct A { ~A(); };
  struct B { virtual ~B(); };
  struct C : B {};

  A &&a = dynamic_cast<A&&>(A{});  // this is valid even with -fno-rtti as it only does lifetime extension
  // CHECK: @_ZN7PR202271aE = addrspace(200) global %"struct.PR20227::A" addrspace(200)* null, align [[$CAP_SIZE]]
  // CHECK: @_ZGRN7PR202271aE_ = internal addrspace(200) global %"struct.PR20227::A" zeroinitializer, align 1

#ifdef CHECK_ERROR
  // RTTI should be off by default with CHERI
  B &&b = dynamic_cast<C&&>(dynamic_cast<B&&>(C{}));  // expected-error {{use of dynamic_cast requires -frtti}}
#endif

  B &&c = static_cast<C&&>(static_cast<B&&>(C{}));
  // CHECK: @_ZN7PR202271cE = addrspace(200) global %"struct.PR20227::B" addrspace(200)* null, align [[$CAP_SIZE]]
  // CHECK: @_ZGRN7PR202271cE_ = internal addrspace(200) global %"struct.PR20227::C" zeroinitializer, align [[$CAP_SIZE]]

}


// CHECK-LABEL: define internal void @__cxx_global_var_init()
// LEGACY:   [[PCC:%.+]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
// LEGACY:   [[WITH_OFFSET:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[PCC]], i64 ptrtoint (void (%"struct.PR20227::A" addrspace(200)*)* @_ZN7PR202271AD1Ev to i64))
// LEGACY:   [[AS_I8:%.+]] = bitcast i8 addrspace(200)* [[WITH_OFFSET]] to void (i8 addrspace(200)*) addrspace(200)*
// CHECK: call i32 @__cxa_atexit(void (i8 addrspace(200)*) addrspace(200)*
// LEGACY-SAME: [[AS_I8]],
// NEWABI-SAME: bitcast (void (%"struct.PR20227::A" addrspace(200)*) addrspace(200)* @_ZN7PR202271AD1Ev to void (i8 addrspace(200)*) addrspace(200)*),
// CHECK-SAME:  i8 addrspace(200)* getelementptr inbounds (%"struct.PR20227::A", %"struct.PR20227::A" addrspace(200)* @_ZGRN7PR202271aE_, i32 0, i32 0), i8 addrspace(200)* @__dso_handle) #2
// CHECK:   store %"struct.PR20227::A" addrspace(200)* @_ZGRN7PR202271aE_, %"struct.PR20227::A" addrspace(200)* addrspace(200)* @_ZN7PR202271aE, align [[$CAP_SIZE]]
// CHECK:   ret void

// CHECK: declare i32 @__cxa_atexit(void (i8 addrspace(200)*) addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*)

// CHECK-LABEL: define internal void @__cxx_global_var_init.1()
// CHECK:    call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align [[$CAP_SIZE]] bitcast (%"struct.PR20227::C" addrspace(200)* @_ZGRN7PR202271cE_ to i8 addrspace(200)*), i8 0, i64 [[$CAP_SIZE]], i1 false)
// CHECK:    call void @_ZN7PR202271CC1Ev(%"struct.PR20227::C" addrspace(200)* @_ZGRN7PR202271cE_) #2
// LEGACY:    [[PCC:%.+]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
// LEGACY:    [[WITH_OFFSET:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[PCC]], i64 ptrtoint (void (%"struct.PR20227::C" addrspace(200)*)* @_ZN7PR202271CD1Ev to i64))
// LEGACY:    [[AS_I8:%.+]] = bitcast i8 addrspace(200)* [[WITH_OFFSET]] to void (i8 addrspace(200)*) addrspace(200)*
// CHECK:    call i32 @__cxa_atexit(void (i8 addrspace(200)*) addrspace(200)*
// LEGACY-SAME: [[AS_I8]],
// NEWABI-SAME: bitcast (void (%"struct.PR20227::C" addrspace(200)*) addrspace(200)* @_ZN7PR202271CD1Ev to void (i8 addrspace(200)*) addrspace(200)*),
// CHECK-SAME:  i8 addrspace(200)* bitcast (%"struct.PR20227::C" addrspace(200)* @_ZGRN7PR202271cE_ to i8 addrspace(200)*), i8 addrspace(200)* @__dso_handle) #2
// CHECK:    store %"struct.PR20227::B" addrspace(200)* getelementptr inbounds (%"struct.PR20227::C", %"struct.PR20227::C" addrspace(200)* @_ZGRN7PR202271cE_, i32 0, i32 0), %"struct.PR20227::B" addrspace(200)* addrspace(200)* @_ZN7PR202271cE, align [[$CAP_SIZE]]
// CHECK:    ret void
