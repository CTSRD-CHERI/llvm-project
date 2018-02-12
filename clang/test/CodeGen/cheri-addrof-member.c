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
	// CHECK: [[VAR:%.+]] = load %struct.astruct addrspace(200)*
	// CHECK: getelementptr inbounds %struct.astruct, %struct.astruct addrspace(200)* [[VAR]], i32 0, i32 1
	// CHECK: ret i32 addrspace(200)*

}

int * __capability
func2(int* __capability asp)
{
	// Check that we generate something sensible for this
	return &(asp[42]);
	// CHECK: [[ARRAY:%.+]] = load i32 addrspace(200)*,
	// CHECK: getelementptr inbounds i32, i32 addrspace(200)* [[ARRAY]], i64 42
	// CHECK: ret i32 addrspace(200)*

}
