// RUN: %cheri_cc1 -o - %s -emit-llvm | %cheri_FileCheck %s
#define CHERI_CCALL(suffix, cls) \
	__attribute__((cheri_ccall))\
	__attribute__((cheri_method_suffix(suffix)))\
	__attribute__((cheri_method_class(cls)))

struct cheri_object
{
  void * __capability a;
  void * __capability b;
};

struct cheri_object cls;
struct cheri_object other;


CHERI_CCALL("_cap", cls)
void foo(int, int);

// CHECK: @__cheri_method.cls.foo = linkonce_odr global i64 0, section ".CHERI_CALLER"
// CHECK: = private unnamed_addr constant [4 x i8] c"cls\00"
// CHECK: = private unnamed_addr constant [4 x i8] c"foo\00"
// CHECK: @.sandbox_required_method.cls.foo = global { i64, ptr, ptr, ptr, i64 } { i64 0, ptr
// CHECK: , ptr @1, ptr @__cheri_method.cls.foo, i64 0 }, section "__cheri_sandbox_required_methods", comdat
// CHECK: @cls = global %struct.cheri_object zeroinitializer, align [[#CAP_SIZE]]
// CHECK: @__cheri_callee_method.cls.fish = global ptr @fish, section ".CHERI_CALLEE"
// CHECK: = private unnamed_addr constant [5 x i8] c"fish\00"
// CHECK: @.sandbox_provided_method.cls.fish = global { i64, ptr, ptr, ptr } { i64 0, ptr @2
// CHECK: , ptr @3
// CHECK: , ptr @__cheri_callee_method.cls.fish }, section "__cheri_sandbox_provided_methods", comdat
// CHECK: @__cheri_method.cls.fish = linkonce_odr global i64 0, section ".CHERI_CALLER"

void bar(int a, int b)
{
  // CHECK-LABEL: define dso_local void @bar(i32
  // CHECK: load i64, ptr @__cheri_method.cls.foo, align 8, [[$INVARIANT_LOAD:!invariant.load ![0-9]+]]
	// CHECK: call chericcallcc void @cheri_invoke(ptr addrspace(200) inreg %{{.*}}, ptr addrspace(200) inreg %{{.*}}, i64 noundef zeroext %{{.*}}, i32 noundef signext %{{.*}}, i32 noundef signext %{{.*}})
	foo_cap(other, a, b);
	// CHECK: call chericcallcc void @cheri_invoke(ptr addrspace(200) noundef %{{.*}}, ptr addrspace(200) noundef %{{.*}}, i64 noundef zeroext %{{.*}}, i32 noundef signext %{{.*}}, i32 noundef signext %{{.*}})
	foo(a,b);
}

__attribute__((cheri_ccallee))
__attribute__((cheri_method_class(cls)))
__attribute__((cheri_method_suffix("_cap")))
void fish(void)
{
  // CHECK-LABEL: define dso_local chericcallcce void @fish()
}

__attribute__((cheri_method_suffix("_cap")))
__attribute__((cheri_method_class(cls)))
void flibble(void);

// CHECK-LABEL: define dso_local void @call()
void call(void)
{
  // CHECK: call chericcallcce void @fish()
	fish();
	// Check that we get a ccall to cheri_invoke with the correct method number
	// CHECK: load i64, ptr @__cheri_method.cls.fish, align 8, [[$INVARIANT_LOAD]]
	// CHECK: call chericcallcc void
	// CHECK: @cheri_invoke
	fish_cap(other);
	// CHECK: call void @flibble()
	flibble();
	// CHECK: load i64, ptr @__cheri_method.cls.flibble, align 8, [[$INVARIANT_LOAD]]
	// CHECK: call chericcallcc void
	// CHECK: @cheri_invoke
	flibble_cap(other);
}
