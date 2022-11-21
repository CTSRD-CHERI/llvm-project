// RUN: %cheri_purecap_cc1 -x c++ -std=c++11 -emit-llvm -o - %s | FileCheck %s

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

// Try as an exported variable:
extern "C" const auto dummyLockedForReadExported = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForRead));
extern "C" const auto dummyLockedForWriteExported = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForWrite));
// CHECK: @dummyLockedForReadExported = addrspace(200) constant %class.QReadWriteLockPrivate addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1) to %class.QReadWriteLockPrivate addrspace(200)*)
// CHECK: @dummyLockedForWriteExported = addrspace(200) constant %class.QReadWriteLockPrivate addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2) to %class.QReadWriteLockPrivate addrspace(200)*)

// Try as an exported variable (but non-const so it actually needs to be loaded:
extern "C" auto dummyLockedForReadExportedNonConst = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForRead));
extern "C" auto dummyLockedForWriteExportedNonConst = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForWrite));
// CHECK: @dummyLockedForReadExportedNonConst = addrspace(200) global %class.QReadWriteLockPrivate addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1) to %class.QReadWriteLockPrivate addrspace(200)*)
// CHECK: @dummyLockedForWriteExportedNonConst = addrspace(200) global %class.QReadWriteLockPrivate addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2) to %class.QReadWriteLockPrivate addrspace(200)*)

namespace {
// and again as a local one
static auto dummyLockedForReadLocalNonConst = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForRead));
static auto dummyLockedForWriteLocalNonConst = reinterpret_cast<QReadWriteLockPrivate *>(quintptr(StateLockedForWrite));
// CHECK: @_ZN12_GLOBAL__N_131dummyLockedForReadLocalNonConstE = internal addrspace(200) global %class.QReadWriteLockPrivate addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1) to %class.QReadWriteLockPrivate addrspace(200)*)
// CHECK: @_ZN12_GLOBAL__N_132dummyLockedForWriteLocalNonConstE = internal addrspace(200) global %class.QReadWriteLockPrivate addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2) to %class.QReadWriteLockPrivate addrspace(200)*)
}


bool testAndSetAcquire(void* expected, void* newValue);
void lockSlowPath();

void lock() {
  // This previously stored a $ddc derived value instead of a null-derived one
  // It was generating an i8 addrspace(200)* inttoptr (i64 2 to i8 addrspace(200)*)
  // instead of a csetoffset on null
  // CHECK-LABEL: @_Z4lockv()
  // CHECK: entry:
  // CHECK-NEXT: call noundef zeroext i1 @_Z17testAndSetAcquirePvS_(i8 addrspace(200)* noundef null, i8 addrspace(200)* noundef getelementptr (i8, i8 addrspace(200)* null, i64 2))
  if (testAndSetAcquire(nullptr, dummyLockedForWrite))
    return;

  lockSlowPath();
}

enum class intcap_enum : __intcap_t {
  val1 = 5,
  val2 = 6,
  val3 = 15,
  val4 = 16,
};

// None of the casts in the following function should generate an inttoptr:
void test2() {
  // CHECK-LABEL: @_Z5test2v()
  void *v = (void *)1;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1), i8 addrspace(200)* addrspace(200)* %v
  void *v2 = (void *)(__intcap_t)2;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2), i8 addrspace(200)* addrspace(200)* %v2
  void *v3 = (void *)(intcap_enum)3;  // This previously crashed the compiler
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 3), i8 addrspace(200)* addrspace(200)* %v3
  void *v4 = (void *)(long)4;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 4), i8 addrspace(200)* addrspace(200)* %v4
  void *v5 = (void *)intcap_enum::val1;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 5), i8 addrspace(200)* addrspace(200)* %v5
  void *v6 = (void *)(__intcap_t)intcap_enum::val2;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 6), i8 addrspace(200)* addrspace(200)* %v6

  __intcap_t i = 11;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 11), i8 addrspace(200)* addrspace(200)* %i
  __intcap_t i2 = (__intcap_t)12;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 12), i8 addrspace(200)* addrspace(200)* %i2
  __intcap_t i3 = (__intcap_t)(void *)(intcap_enum)13;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 13), i8 addrspace(200)* addrspace(200)* %i3
  __intcap_t i4 = (__intcap_t)(long)14;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 14), i8 addrspace(200)* addrspace(200)* %i4
  __intcap_t i5 = (__intcap_t)intcap_enum::val3;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 15), i8 addrspace(200)* addrspace(200)* %i5
  __intcap_t i6 = (__intcap_t)(void *)intcap_enum::val4;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 16), i8 addrspace(200)* addrspace(200)* %i6

  void* constant1 = dummyLockedForRead;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1), i8 addrspace(200)* addrspace(200)* %constant1
  void* constant2 = dummyLockedForWrite;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2), i8 addrspace(200)* addrspace(200)* %constant2

  // Since these variables are const, the load can be elided:
  void *constant3 = dummyLockedForReadExported;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1), i8 addrspace(200)* addrspace(200)* %constant3
  void *constant4 = dummyLockedForWriteExported;
  // CHECK: store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 2), i8 addrspace(200)* addrspace(200)* %constant4

  // However, these are non-const and exported so actually need to be loaded:
  void *constant5 = dummyLockedForReadExportedNonConst;
  // CHECK: load %class.QReadWriteLockPrivate addrspace(200)*, %class.QReadWriteLockPrivate addrspace(200)* addrspace(200)* @dummyLockedForReadExportedNonConst
  void *constant6 = dummyLockedForWriteExportedNonConst;
  // CHECK: load %class.QReadWriteLockPrivate addrspace(200)*, %class.QReadWriteLockPrivate addrspace(200)* addrspace(200)* @dummyLockedForWriteExportedNonConst

  // Non-const locals also need to be loaded (at least for -O0):
  void *constant7 = dummyLockedForReadLocalNonConst;
  // CHECK: load %class.QReadWriteLockPrivate addrspace(200)*, %class.QReadWriteLockPrivate addrspace(200)* addrspace(200)* @_ZN12_GLOBAL__N_131dummyLockedForReadLocalNonConstE
  void *constant8 = dummyLockedForWriteLocalNonConst;
  // CHECK: load %class.QReadWriteLockPrivate addrspace(200)*, %class.QReadWriteLockPrivate addrspace(200)* addrspace(200)* @_ZN12_GLOBAL__N_132dummyLockedForWriteLocalNonConstE
}
