// RUN: %cheri_purecap_cc1 %s -emit-llvm -std=c++2a -cheri-bounds=aggressive -o - -std=c++11 -O2 | FileCheck -check-prefixes CHECK,AGGRESSIVE %s
// RUN: %cheri_purecap_cc1 %s -emit-llvm -std=c++2a -cheri-bounds=conservative -o - -std=c++11 -O2 | FileCheck -check-prefixes CHECK,CONSERVATIVE %s
namespace std {
typedef decltype(sizeof(0)) size_t;
}

void* operator new(std::size_t count, void* p);
void* operator new[](std::size_t count, void* p);
void operator delete(void* ptr, void* place) {}
void operator delete[](void* ptr, void* place) {}

struct Foo {
  int x;
};

extern "C" Foo *test_foo_not_aligned() { return new Foo; }
// CHECK-LABEL: @test_foo_not_aligned(
// CHECK: call {{.+}} @_Znwm(i64 noundef zeroext 4)
// CHECK-NOT: bounds.set


extern "C" Foo *test_foo_not_aligned_array() { return new Foo[2]; }
// CHECK-LABEL: @test_foo_not_aligned_array(
// CHECK: call {{.+}} @_Znam(i64 noundef zeroext 8)
// CHECK-NOT: bounds.set


extern "C" Foo *test_foo_not_aligned_array_nonconst(int n) { return new Foo[n]; }
// CHECK-LABEL: @test_foo_not_aligned_array_nonconst(
// CHECK: call {{.+}} @_Znam(i64 noundef zeroext %{{.+}})
// CHECK-NOT: bounds.set


// Now check that the compiler inserts csetbounds after placement new in aggressive mode


extern "C" Foo *test_foo_nonalloc(void* buffer) { return new (buffer) Foo; }
// CHECK-LABEL: @test_foo_nonalloc(
// CHECK-NEXT:  entry:
// AGGRESSIVE-NEXT: [[TMP0:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* [[BUFFER:%.*]], i64 4)
// CHECK-NEXT:      [[RESULT:%.*]] = bitcast i8 addrspace(200)* %{{.+}} to [[STRUCT_FOO:%.*]] addrspace(200)*
// CHECK-NEXT:      ret %struct.Foo addrspace(200)* [[RESULT]]

extern "C" Foo *test_foo_nonalloc_array(void* buffer) { return new (buffer) Foo[2]; }
// CHECK-LABEL: @test_foo_nonalloc_array(
// CHECK-NEXT:  entry:
// AGGRESSIVE-NEXT: [[TMP0:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* [[BUFFER:%.*]], i64 8)
// CHECK-NEXT:      [[RESULT:%.*]] = bitcast i8 addrspace(200)* %{{.+}} to [[STRUCT_FOO:%.*]] addrspace(200)*
// CHECK-NEXT:      ret %struct.Foo addrspace(200)* [[RESULT]]


extern "C" Foo *test_foo_nonalloc_array_nonconst(void* buffer, int n) { return new (buffer) Foo[n]; }
// AGGRESSIVE-LABEL: @test_foo_nonalloc_array_nonconst(
// AGGRESSIVE-NEXT:  entry:
// AGGRESSIVE-NEXT:    [[TMP0:%.*]] = sext i32 [[N:%.*]] to i64
// AGGRESSIVE-NEXT:    [[TMP1:%.*]] = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 [[TMP0]], i64 4)
// AGGRESSIVE-NEXT:    [[TMP2:%.*]] = extractvalue { i64, i1 } [[TMP1]], 1
// AGGRESSIVE-NEXT:    [[TMP3:%.*]] = extractvalue { i64, i1 } [[TMP1]], 0
// AGGRESSIVE-NEXT:    [[TMP4:%.*]] = select i1 [[TMP2]], i64 -1, i64 [[TMP3]]
// AGGRESSIVE-NEXT:    [[TMP5:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* [[BUFFER:%.*]], i64 [[TMP4]])
// AGGRESSIVE-NEXT:    [[TMP6:%.*]] = bitcast i8 addrspace(200)* [[TMP5]] to [[STRUCT_FOO:%.*]] addrspace(200)*
// AGGRESSIVE-NEXT:    ret %struct.Foo addrspace(200)* [[TMP6]]
//
// CONSERVATIVE-LABEL: @test_foo_nonalloc_array_nonconst(
// CONSERVATIVE-NEXT:  entry:
// CONSERVATIVE-NEXT:    [[TMP0:%.*]] = bitcast i8 addrspace(200)* [[BUFFER:%.*]] to [[STRUCT_FOO:%.*]] addrspace(200)*
// CONSERVATIVE-NEXT:    ret %struct.Foo addrspace(200)* [[TMP0]]
//
