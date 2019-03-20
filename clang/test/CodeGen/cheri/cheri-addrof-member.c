// RUN: %cheri_cc1 -S  %s  -emit-llvm -o - | FileCheck %s

struct astruct {
        int     member1;
        int     member2;
};


int * __capability
func(struct astruct * __capability asp)
{
	// Check that we generate something sensible for this
	return &(asp->member2);
	// CHECK-LABEL @func(
	// CHECK: [[VAR:%.+]] = load %struct.astruct addrspace(200)*
	// CHECK: getelementptr inbounds %struct.astruct, %struct.astruct addrspace(200)* [[VAR]], i32 0, i32 1
	// CHECK: ret i32 addrspace(200)*

}

int * __capability
func2(int* __capability asp)
{
	// Check that we generate something sensible for this
	return &(asp[42]);
	// CHECK-LABEL @func2(
	// CHECK: [[ARRAY:%.+]] = load i32 addrspace(200)*,
	// CHECK: getelementptr inbounds i32, i32 addrspace(200)* [[ARRAY]], i64 42
	// CHECK: ret i32 addrspace(200)*

}

// This started crashing in CodeGen after recursively resolving memberexprs
void func3(void * __capability b) {
  typeof(&b) c = &b;
  // CHECK-LABEL @func3(
  // Check that we don't create a capability here:
  // CHECK: [[C_ALLOCA:%.+]] = alloca i8 addrspace(200)**, align 8
  // CHECK: store i8 addrspace(200)** {{%.+}}, i8 addrspace(200)*** [[C_ALLOCA]], align 8
}
