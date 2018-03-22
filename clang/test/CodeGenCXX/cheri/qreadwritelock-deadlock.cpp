// RUN: %cheri_purecap_cc1 -x c++ -std=c++11 -emit-llvm -ast-dump -fcolor-diagnostics -o /dev/null %s
// RUN: %cheri_purecap_cc1 -x c++ -std=c++11 -emit-llvm -o - %s
// RUN: %cheri_purecap_cc1 -x c++ -std=c++11 -emit-llvm -o - %s | FileCheck %s

// XFAIL: *
// QReadWriteLock was never being unlocked because we were write a $ddc derived
// value instead of null with offset 2 and therefore the code later assumed that
// the lock was still taken.

class QReadWriteLockPrivate;
typedef __uintcap_t quintptr;

namespace {
enum {
    StateMask = 0x3,
    StateLockedForRead = 0x1,
    StateLockedForWrite = 0x2,
};
const auto dummyLockedForRead = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForRead));
const auto dummyLockedForWrite = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForWrite));
}

bool testAndSetAcquire(void* expected, void* newValue);
void lockSlowPath();

void lock() {
  // This previously stored a $ddc derived value instead of a null-derived one
  // It was generating an i8 addrspace(200)* inttoptr (i64 2 to i8 addrspace(200)*)
  // instead of a csetoffset on null
  // CHECK-LABEL: @_Z4lockv()
  // CHECK: [[WRITE_DUMMY:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 2)
  // CHECK: call zeroext i1 @_Z17testAndSetAcquireU3capPvS_(i8 addrspace(200)* null, [[WRITE_DUMMY]])
  if (testAndSetAcquire(nullptr, dummyLockedForWrite))
    return;

  lockSlowPath();
}

enum class intcap_enum : __intcap_t {
  val1 = 5,
  val2 = 45,
};

void test2() {
  void *v = (void *)1;
  void *v2 = (void *)(__intcap_t)2;
  void *v3 = (void *)(intcap_enum)3;  // This previously crashed the compiler
  void* v4 = (void*)(long)4;
  void* v5 = (void*)intcap_enum::val1;
  void* v6 = (void*)(__intcap_t)intcap_enum::val2;

  __intcap_t i = 41;
  __intcap_t i2 = (__intcap_t)42;
  __intcap_t i3 = (__intcap_t)(void*)(intcap_enum)43;
  __intcap_t i4 = (__intcap_t)(long)44;
  __intcap_t i5 = (__intcap_t)intcap_enum::val2;


  void* bad = dummyLockedForRead;
  void* bad2 = dummyLockedForWrite;
}
