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
