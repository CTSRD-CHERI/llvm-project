// RUN: %cheri_purecap_cc1 %s -emit-llvm -o - | %cheri_FileCheck %s
int main(void) {
  _Atomic(int*) p;
  // CHECK:  %p = alloca i32 addrspace(200)*, align [[$CAP_SIZE]], addrspace(200)
  // CHECK:  [[ATOMIC_TMP1:%.*]] = alloca i64, align 8, addrspace(200)
  // CHECK:  [[ATOMIC_TMP2:%.*]] = alloca i32 addrspace(200)*, align [[$CAP_SIZE]]
  // CHECK:  [[ATOMIC_TMP3:%.*]] = alloca i32 addrspace(200)*, align [[$CAP_SIZE]]
  // CHECK:  %c = alloca i32 addrspace(200)*, align [[$CAP_SIZE]]
  // CHECK:  [[ATOMIC_TMP4:%.*]] = alloca i32 addrspace(200)*, align [[$CAP_SIZE]]
  // CHECK:  [[ATOMIC_TMP5:%.*]] = alloca i32 addrspace(200)*, align [[$CAP_SIZE]]
  // CHECK:  %expected = alloca i32 addrspace(200)*, align [[$CAP_SIZE]], addrspace(200)
  // CHECK:  [[ATOMIC_TMP6:%.*]] = alloca i32 addrspace(200)*, align [[$CAP_SIZE]]

  // CHECK:  store i64 4, i64 addrspace(200)* [[ATOMIC_TMP1]], align 8
  // CHECK:  [[AS_I8PTR:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP2]] to i8 addrspace(200)*
  // CHECK:  [[AS_I8PTR2:%.*]] = bitcast i64 addrspace(200)* [[ATOMIC_TMP1]] to i8 addrspace(200)*
  // CHECK:  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align [[$CAP_SIZE]] [[AS_I8PTR]], i8 addrspace(200)* align 8 [[AS_I8PTR2]], i64 8, i1 false)
  // CHECK:  [[FETCHADD_ARG1:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
  // CHECK:  [[FETCHADD_ARG2:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP2]] to i8 addrspace(200)*
  // CHECK:  [[FETCHADD_ARG3:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP3]] to i8 addrspace(200)*
  // CHECK:  call void @__atomic_fetch_add(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* [[FETCHADD_ARG1]], i8 addrspace(200)* [[FETCHADD_ARG2]], i8 addrspace(200)* [[FETCHADD_ARG3]], i32 signext 5)
  __c11_atomic_fetch_add(&p, 1, __ATOMIC_SEQ_CST);

  // CHECK:  [[DEAD:%.*]] = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP3]], align [[$CAP_SIZE]]
  // CHECK:  [[LOAD_ARG1:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
  // CHECK:  [[LOAD_ARG2:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP4]] to i8 addrspace(200)*
  // CHECK:  call void @__atomic_load(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* [[LOAD_ARG1]], i8 addrspace(200)* [[LOAD_ARG2]], i32 signext 5)
  // CHECK:  [[TMP:%.*]] = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP4]], align [[$CAP_SIZE]]
  // CHECK:  store i32 addrspace(200)* [[TMP]], i32 addrspace(200)* addrspace(200)* %c, align [[$CAP_SIZE]]
  int *c = __c11_atomic_load(&p, __ATOMIC_SEQ_CST);

  // CHECK: store i32 addrspace(200)* null, i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP5]], align [[$CAP_SIZE]]
  // CHECK: [[STORE_ARG1:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
  // CHECK: [[STORE_ARG2:%.*]] = bitcast i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP5]] to i8 addrspace(200)*
  // CHECK: call void @__atomic_store(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* [[STORE_ARG1]], i8 addrspace(200)* [[STORE_ARG2]], i32 signext 5)
  __c11_atomic_store(&p, 0, __ATOMIC_SEQ_CST);

  // CHECK:  store i32 addrspace(200)* null, i32 addrspace(200)* addrspace(200)* %expected, align [[$CAP_SIZE]]
  // CHECK:  [[CMPXCHG_ARG1:%.*]]  = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
  // CHECK:  [[CMPXCHG_ARG2:%.*]]  = bitcast i32 addrspace(200)* addrspace(200)* %expected to i8 addrspace(200)*
  // CHECK:  [[CMPXCHG_ARG3:%.*]]  = bitcast i32 addrspace(200)* addrspace(200)* [[ATOMIC_TMP6]] to i8 addrspace(200)*
  // CHECK:  %call = call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* [[CMPXCHG_ARG1]], i8 addrspace(200)* [[CMPXCHG_ARG2]], i8 addrspace(200)* [[CMPXCHG_ARG3]], i32 signext 5, i32 signext 5)
  int* expected = 0;
  __c11_atomic_compare_exchange_strong(&p, &expected, c, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return 0;
}
