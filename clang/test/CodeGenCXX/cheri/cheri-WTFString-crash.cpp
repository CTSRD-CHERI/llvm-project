// RUN: %cheri_purecap_cc1 -O2 -std=c++11 -o - -emit-llvm %s -w | FileCheck %s
// https://github.com/CTSRD-CHERI/clang/issues/145
// This used to crash due to the __uintcap_t template argument
//
// From WebKit:

typedef __uintcap_t uintptr_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __SIZE_TYPE__ size_t;
typedef uint64_t vaddr_t;

template <uintptr_t mask>
inline bool isAlignedTo(const void* pointer)
{
#ifdef __CHERI_PURE_CAPABILITY__
    return !(reinterpret_cast<vaddr_t>(pointer) & mask);
#else
    return !(reinterpret_cast<uintptr_t>(pointer) & mask);
#endif
}

// Assuming that a pointer is the size of a "machine word", then
// uintptr_t is an integer type that is also a machine word.
#ifndef __CHERI_PURE_CAPABILITY__
typedef uintptr_t MachineWord;
#else
// XXXAR: MachineWord only seems to be used here and doesn't seem to require storing pointers
typedef uint64_t MachineWord;
#endif

const size_t machineWordAlignmentMask = sizeof(MachineWord) - 1;

bool isAlignedToMachineWord(const void* pointer)
{
    return isAlignedTo<machineWordAlignmentMask>(pointer);
}

// CHECK:      define zeroext i1 @_Z22isAlignedToMachineWord
// CHECK-NEXT:   entry:
// CHECK-NEXT:   [[ADDR:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %pointer) #2
// CHECK-NEXT:   [[AND:%.*]] = and i64 [[ADDR]], 7
// CHECK-NEXT:   [[TOBOOL:%.*]] = icmp eq i64 [[AND]], 0
// CHECK-NEXT:   ret i1 [[TOBOOL]]
// CHECK-NEXT: }
