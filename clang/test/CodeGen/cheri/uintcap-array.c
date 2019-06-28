// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -emit-llvm %s -O2 -o - | %cheri_FileCheck %s
// RUN: %cheri_purecap_cc1 -S %s -O2 -o - | %cheri_FileCheck %s -check-prefix ASM

// Check that we emit sensible values for __intcap_t arrays (this broke Qt qhooks.cpp)

int x;
long longvalue = (long)&x;
__uintcap_t uintcapvalue = (__uintcap_t)&x;

// CHECK: @x = common addrspace(200) global i32 0, align 4
// CHECK: @longvalue = local_unnamed_addr addrspace(200) global i64 ptrtoint (i32 addrspace(200)* @x to i64), align 8
// CHECK: @uintcapvalue = local_unnamed_addr addrspace(200) global i8 addrspace(200)* bitcast (i32 addrspace(200)* @x to i8 addrspace(200)*), align [[#CAP_SIZE]]

__uintcap_t uintptr_constant_int = 1;
// CHECK: @uintptr_constant_int = addrspace(200) global i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*), align [[#CAP_SIZE]]
__uintcap_t uintptr_constant_ptr = (__uintcap_t)&uintptr_constant_int;
// CHECK: @uintptr_constant_ptr = local_unnamed_addr addrspace(200) global i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @uintptr_constant_int to i8 addrspace(200)*), align [[#CAP_SIZE]]

// We were previously not emitting capability sized elements for this array:
// All integer constant were emitted as i64 and only the pointer casted to uintcap ended up as addrspace(200) pointers
// @uintptr_array = addrspace(200) global <{ i64, i64, i8 addrspace(200)*, i64 }> <{ i64 3, i64 5246977, i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @one_uintptr to i8 addrspace(200)*), i64 0 }>, align 16
// This bug resulted in qhooks.cpp being miscompiled (and I'm sure lots of other stuff too)

__uintcap_t uintptr_array[] = {
// CHECK: @uintptr_array = local_unnamed_addr addrspace(200) global [4 x i8 addrspace(200)*]
    3,
// CHECK-SAME: [i8 addrspace(200)* inttoptr (i64 3 to i8 addrspace(200)*),
    0x501001,
// CHECK-SAME:  i8 addrspace(200)* inttoptr (i64 5246977 to i8 addrspace(200)*),
    (__uintcap_t)&uintptr_constant_int,
// CHECK-SAME:  i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @uintptr_constant_int to i8 addrspace(200)*),
    0
// CHECK-SAME: i8 addrspace(200)* null]
};

// An array with a filler was also broken: @uintptr_array_2 = global <{ i64, i128, i128 }> <{ i64 1, i128 0, i128 0 }>, align 16
__uintcap_t uintptr_array_2[3] = { 1 };
// CHECK: @uintptr_array_2 = local_unnamed_addr addrspace(200) global [3 x i8 addrspace(200)*]
// CHECK-SAME: [i8 addrspace(200)* inttoptr (i64 1 to i8 addrspace(200)*),
// CHECK-SAME: i8 addrspace(200)* null, i8 addrspace(200)* null]

// Arrays of pointers should also just emit 4 .chericap directives
void* ptr_array[] = {
  (void*)3,
  (void*)0x501001,
  &uintptr_constant_int,
  (void*)0
};
// CHECK: @ptr_array = local_unnamed_addr addrspace(200) global [4 x i8 addrspace(200)*] [i8 addrspace(200)* inttoptr (i64 3 to i8 addrspace(200)*), i8 addrspace(200)* inttoptr (i64 5246977 to i8 addrspace(200)*), i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @uintptr_constant_int to i8 addrspace(200)*), i8 addrspace(200)* null]

// Using a struct with a single uintptr_t value happened to work correctly for ASM output
// but the IR was quite broken:
// @uintptr_struct_array = addrspace(200) global <{ { i64, [8 x i8] }, { i64, [8 x i8] }, %struct.uintptr_struct, { i64, [8 x i8] } }> <{ { i64, [8 x i8] } { i64 3, [8 x i8] undef }, { i64, [8 x i8] } { i64 5246977, [8 x i8] undef }, %struct.uintptr_struct { i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @one_uintptr to i8 addrspace(200)*) }, { i64, [8 x i8] } { i64 0, [8 x i8] undef } }>, align 16

struct uintptr_struct {
  __uintcap_t value;
};

struct uintptr_struct one_uintptr_struct = { .value = 123 };
// CHECK: @one_uintptr_struct = local_unnamed_addr addrspace(200) global %struct.uintptr_struct { i8 addrspace(200)* inttoptr (i64 123 to i8 addrspace(200)*) }

struct uintptr_struct uintptr_struct_array[] = {
// CHECK: @uintptr_struct_array = local_unnamed_addr addrspace(200) global [4 x %struct.uintptr_struct]
    {3},
// CHECK-SAME: [%struct.uintptr_struct { i8 addrspace(200)* inttoptr (i64 3 to i8 addrspace(200)*) },
    {.value = 0x501001},
// CHECK-SAME:  %struct.uintptr_struct { i8 addrspace(200)* inttoptr (i64 5246977 to i8 addrspace(200)*) },
    {(__uintcap_t)&uintptr_constant_int},
// CHECK-SAME:  %struct.uintptr_struct { i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @uintptr_constant_int to i8 addrspace(200)*) },
    {0},
// CHECK-SAME:  %struct.uintptr_struct zeroinitializer]
};

// Now check local arrays in functions:

void use_array(void* value);

void func(void) {
  // This seems to emit a memcpy from a global so it had the same issues:
  // CHECK: @__const.func.uintptr_struct_array = private unnamed_addr addrspace(200) constant [2 x %struct.uintptr_struct]
  // CHECK-SAME: [%struct.uintptr_struct { i8 addrspace(200)* inttoptr (i64 5246977 to i8 addrspace(200)*) },
  // CHECK-SAME:  %struct.uintptr_struct { i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @uintptr_constant_int to i8 addrspace(200)*) }]
  struct uintptr_struct uintptr_struct_array[] = {
    {.value = 0x501001},
    {(__uintcap_t)&uintptr_constant_int},
  };
  use_array(&uintptr_struct_array);

  // CHECK: @__const.func.uintptr_array = private unnamed_addr addrspace(200) constant [2 x i8 addrspace(200)*]
  // CHECK-SAME: [i8 addrspace(200)* inttoptr (i64 3 to i8 addrspace(200)*),
  // CHECK-SAME:  i8 addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* @uintptr_constant_int to i8 addrspace(200)*)]
  __uintcap_t uintptr_array[] = {
    3,
    (__uintcap_t)&uintptr_constant_int,
  };
  use_array(&uintptr_array);
}

// Check uninitialized elements:
__uintcap_t uintptr_bss;
__uintcap_t uintptr_array_bss[5];
struct uintptr_struct uintptr_struct_bss;
// CHECK: @uintptr_bss = common local_unnamed_addr addrspace(200) global i8 addrspace(200)* null, align [[#CAP_SIZE]]
// CHECK: @uintptr_array_bss = common local_unnamed_addr addrspace(200) global [5 x i8 addrspace(200)*] zeroinitializer, align [[#CAP_SIZE]]
// CHECK: @uintptr_struct_bss = common local_unnamed_addr addrspace(200) global %struct.uintptr_struct zeroinitializer, align [[#CAP_SIZE]]



// CHECK-LABEL: define void @func()
// test that the correct values are used
// CHECK: alloca [2 x %struct.uintptr_struct], align [[#CAP_SIZE]], addrspace(200)
// CHECK: alloca [2 x i8 addrspace(200)*], align [[#CAP_SIZE]], addrspace(200)



// TODO: all the ASM checks should move to LLVM, but checking it here is easier
// ASM-LABEL: longvalue:
// ASM-NEXT:	.8byte	x
// ASM-NEXT:	.size	longvalue, 8
// ASM-LABEL: uintcapvalue:
// ASM-NEXT: 	.chericap	x
// ASM-NEXT: 	.size	uintcapvalue, [[#CAP_SIZE]]
// ASM-LABEL: uintptr_constant_int:
// ASM-NEXT: 	.chericap	1
// ASM-NEXT: 	.size	uintptr_constant_int, [[#CAP_SIZE]]
// ASM-LABEL: uintptr_constant_ptr:
// ASM-NEXT: 	.chericap	uintptr_constant_int
// ASM-NEXT: 	.size	uintptr_constant_ptr, [[#CAP_SIZE]]

// ASM-LABEL: uintptr_array:
// ASM-NEXT: 	.chericap	3
// ASM-NEXT: 	.chericap	5246977
// ASM-NEXT: 	.chericap	uintptr_constant_int
// ASM-NEXT: 	.chericap	0
// ASM-NEXT: 	.size	uintptr_array, [[#CAP_SIZE * 4]]

// ASM-LABEL: uintptr_array_2:
// ASM-NEXT: 	.chericap	1
// ASM-NEXT: 	.chericap	0
// ASM-NEXT: 	.chericap	0
// ASM-NEXT: 	.size	uintptr_array_2, [[#CAP_SIZE * 3]]

// ASM-LABEL: ptr_array:
// ASM-NEXT: 	.chericap	3
// ASM-NEXT: 	.chericap	5246977
// ASM-NEXT: 	.chericap	uintptr_constant_int
// ASM-NEXT: 	.chericap	0
// ASM-NEXT: 	.size	ptr_array, [[#CAP_SIZE * 4]]

// ASM-LABEL: one_uintptr_struct:
// ASM-NEXT: 	.chericap	123
// ASM-NEXT: 	.size	one_uintptr_struct, [[#CAP_SIZE]]

// ASM-LABEL: uintptr_struct_array:
// ASM-NEXT: 	.chericap	3
// ASM-NEXT: 	.chericap	5246977
// ASM-NEXT: 	.chericap	uintptr_constant_int
// ASM-NEXT: 	.space	[[#CAP_SIZE]]
// ASM-NEXT: 	.size	uintptr_struct_array, [[#CAP_SIZE * 4]]

// ASM-LABEL: .L__const.func.uintptr_struct_array:
// ASM-NEXT: 	.chericap	5246977
// ASM-NEXT: 	.chericap	uintptr_constant_int
// ASM-NEXT: 	.size	.L__const.func.uintptr_struct_array, [[#CAP_SIZE * 2]]

// ASM-LABEL: .L__const.func.uintptr_array:
// ASM-NEXT: 	.chericap	3
// ASM-NEXT: 	.chericap	uintptr_constant_int
// ASM-NEXT: 	.size	.L__const.func.uintptr_array, [[#CAP_SIZE * 2]]

// ASM-LABEL: .type	uintptr_bss,@object
// ASM-NEXT: .comm	uintptr_bss,[[#CAP_SIZE]],[[#CAP_SIZE]]
// ASM-NEXT: .type	uintptr_array_bss,@object
// ASM-NEXT: .comm	uintptr_array_bss,[[#CAP_SIZE * 5]],[[#CAP_SIZE]]
// ASM-NEXT: .type	uintptr_struct_bss,@object
// ASM-NEXT: .comm	uintptr_struct_bss,[[#CAP_SIZE]],[[#CAP_SIZE]]
