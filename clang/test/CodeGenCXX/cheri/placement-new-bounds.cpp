// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py
// RUN: %cheri_purecap_cc1 %s -emit-llvm -std=c++2a -cheri-bounds=aggressive -o - -std=c++11 -O2 | FileCheck -check-prefixes CHECK,AGGRESSIVE %s
// RUN: %cheri_purecap_cc1 %s -emit-llvm -std=c++2a -cheri-bounds=conservative -o - -std=c++11 -O2 | FileCheck -check-prefixes CHECK,CONSERVATIVE %s
namespace std {
typedef decltype(sizeof(0)) size_t;
}

void* operator new(std::size_t count, void* p);
void* operator new[](std::size_t count, void* p);
// CHECK-LABEL: @_ZdlPvS_(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    ret void
//
void operator delete(void* ptr, void* place) {}
// CHECK-LABEL: @_ZdaPvS_(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    ret void
//
void operator delete[](void* ptr, void* place) {}

struct Foo {
  int x;
};

// AGGRESSIVE-LABEL: @_Z20test_foo_not_alignedv(
// AGGRESSIVE-NEXT:  entry:
// AGGRESSIVE-NEXT:    [[CALL:%.*]] = tail call noalias noundef nonnull dereferenceable(4) ptr addrspace(200) @_Znwm(i64 noundef zeroext 4) #[[ATTR6:[0-9]+]]
// AGGRESSIVE-NEXT:    ret ptr addrspace(200) [[CALL]]
//
// CONSERVATIVE-LABEL: @_Z20test_foo_not_alignedv(
// CONSERVATIVE-NEXT:  entry:
// CONSERVATIVE-NEXT:    [[CALL:%.*]] = tail call noalias noundef nonnull dereferenceable(4) ptr addrspace(200) @_Znwm(i64 noundef zeroext 4) #[[ATTR4:[0-9]+]]
// CONSERVATIVE-NEXT:    ret ptr addrspace(200) [[CALL]]
//
Foo *test_foo_not_aligned() { return new Foo; }


// AGGRESSIVE-LABEL: @_Z26test_foo_not_aligned_arrayv(
// AGGRESSIVE-NEXT:  entry:
// AGGRESSIVE-NEXT:    [[CALL:%.*]] = tail call noalias noundef nonnull dereferenceable(8) ptr addrspace(200) @_Znam(i64 noundef zeroext 8) #[[ATTR6]]
// AGGRESSIVE-NEXT:    ret ptr addrspace(200) [[CALL]]
//
// CONSERVATIVE-LABEL: @_Z26test_foo_not_aligned_arrayv(
// CONSERVATIVE-NEXT:  entry:
// CONSERVATIVE-NEXT:    [[CALL:%.*]] = tail call noalias noundef nonnull dereferenceable(8) ptr addrspace(200) @_Znam(i64 noundef zeroext 8) #[[ATTR4]]
// CONSERVATIVE-NEXT:    ret ptr addrspace(200) [[CALL]]
//
Foo *test_foo_not_aligned_array() { return new Foo[2]; }


// AGGRESSIVE-LABEL: @_Z35test_foo_not_aligned_array_nonconsti(
// AGGRESSIVE-NEXT:  entry:
// AGGRESSIVE-NEXT:    [[TMP0:%.*]] = sext i32 [[N:%.*]] to i64
// AGGRESSIVE-NEXT:    [[TMP1:%.*]] = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 [[TMP0]], i64 4)
// AGGRESSIVE-NEXT:    [[TMP2:%.*]] = extractvalue { i64, i1 } [[TMP1]], 1
// AGGRESSIVE-NEXT:    [[TMP3:%.*]] = extractvalue { i64, i1 } [[TMP1]], 0
// AGGRESSIVE-NEXT:    [[TMP4:%.*]] = select i1 [[TMP2]], i64 -1, i64 [[TMP3]]
// AGGRESSIVE-NEXT:    [[CALL:%.*]] = tail call noalias noundef nonnull ptr addrspace(200) @_Znam(i64 noundef zeroext [[TMP4]]) #[[ATTR6]]
// AGGRESSIVE-NEXT:    ret ptr addrspace(200) [[CALL]]
//
// CONSERVATIVE-LABEL: @_Z35test_foo_not_aligned_array_nonconsti(
// CONSERVATIVE-NEXT:  entry:
// CONSERVATIVE-NEXT:    [[TMP0:%.*]] = sext i32 [[N:%.*]] to i64
// CONSERVATIVE-NEXT:    [[TMP1:%.*]] = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 [[TMP0]], i64 4)
// CONSERVATIVE-NEXT:    [[TMP2:%.*]] = extractvalue { i64, i1 } [[TMP1]], 1
// CONSERVATIVE-NEXT:    [[TMP3:%.*]] = extractvalue { i64, i1 } [[TMP1]], 0
// CONSERVATIVE-NEXT:    [[TMP4:%.*]] = select i1 [[TMP2]], i64 -1, i64 [[TMP3]]
// CONSERVATIVE-NEXT:    [[CALL:%.*]] = tail call noalias noundef nonnull ptr addrspace(200) @_Znam(i64 noundef zeroext [[TMP4]]) #[[ATTR4]]
// CONSERVATIVE-NEXT:    ret ptr addrspace(200) [[CALL]]
//
Foo *test_foo_not_aligned_array_nonconst(int n) { return new Foo[n]; }


// Now check that the compiler inserts csetbounds after placement new in aggressive mode


// AGGRESSIVE-LABEL: @_Z17test_foo_nonallocPv(
// AGGRESSIVE-NEXT:  entry:
// AGGRESSIVE-NEXT:    [[TMP0:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) [[BUFFER:%.*]], i64 4)
// AGGRESSIVE-NEXT:    ret ptr addrspace(200) [[TMP0]]
//
// CONSERVATIVE-LABEL: @_Z17test_foo_nonallocPv(
// CONSERVATIVE-NEXT:  entry:
// CONSERVATIVE-NEXT:    ret ptr addrspace(200) [[BUFFER:%.*]]
//
Foo *test_foo_nonalloc(void* buffer) { return new (buffer) Foo; }

// AGGRESSIVE-LABEL: @_Z23test_foo_nonalloc_arrayPv(
// AGGRESSIVE-NEXT:  entry:
// AGGRESSIVE-NEXT:    [[TMP0:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) [[BUFFER:%.*]], i64 8)
// AGGRESSIVE-NEXT:    ret ptr addrspace(200) [[TMP0]]
//
// CONSERVATIVE-LABEL: @_Z23test_foo_nonalloc_arrayPv(
// CONSERVATIVE-NEXT:  entry:
// CONSERVATIVE-NEXT:    ret ptr addrspace(200) [[BUFFER:%.*]]
//
Foo *test_foo_nonalloc_array(void* buffer) { return new (buffer) Foo[2]; }


// AGGRESSIVE-LABEL: @_Z32test_foo_nonalloc_array_nonconstPvi(
// AGGRESSIVE-NEXT:  entry:
// AGGRESSIVE-NEXT:    [[TMP0:%.*]] = sext i32 [[N:%.*]] to i64
// AGGRESSIVE-NEXT:    [[TMP1:%.*]] = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 [[TMP0]], i64 4)
// AGGRESSIVE-NEXT:    [[TMP2:%.*]] = extractvalue { i64, i1 } [[TMP1]], 1
// AGGRESSIVE-NEXT:    [[TMP3:%.*]] = extractvalue { i64, i1 } [[TMP1]], 0
// AGGRESSIVE-NEXT:    [[TMP4:%.*]] = select i1 [[TMP2]], i64 -1, i64 [[TMP3]]
// AGGRESSIVE-NEXT:    [[TMP5:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) [[BUFFER:%.*]], i64 [[TMP4]])
// AGGRESSIVE-NEXT:    ret ptr addrspace(200) [[TMP5]]
//
// CONSERVATIVE-LABEL: @_Z32test_foo_nonalloc_array_nonconstPvi(
// CONSERVATIVE-NEXT:  entry:
// CONSERVATIVE-NEXT:    ret ptr addrspace(200) [[BUFFER:%.*]]
//
Foo *test_foo_nonalloc_array_nonconst(void* buffer, int n) { return new (buffer) Foo[n]; }
