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
	// CHECK: getelementptr inbounds %struct.astruct, %struct.astruct addrspace(200)* %0
	// CHECK: ret i32 addrspace(200)*

}

int * __capability
func2(int* __capability asp)
{
	// Check that we generate something sensible for this
	return &(asp[42]); 
	// CHECK: getelementptr inbounds %struct.astruct, %struct.astruct addrspace(200)* %0
	// CHECK: ret i32 addrspace(200)*

}
