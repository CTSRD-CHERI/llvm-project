// RUN: %cheri_purecap_cc1 %s -O2 -msoft-float -emit-llvm -o - | FileCheck %s
union u1 {
	int x;
	int *field;
};
union u2 {
	int x;
	struct {
		int a,b,c,d;
		int *z;
	} y;
};

void *x;
int *y;

// Check that unions containing capabilities are passed as capabilities.
// CHECK: foo(i8 addrspace(200)*
int *foo(union u1 m)
{
	if (m.field == x)
	{
		return y;
	}
	return m.field;
}

// CHECK: bar(i8 addrspace(200)*
int *bar(union u2 m)
{
	if (m.y.z == x)
	{
		return y;
	}
	return m.y.z;
}
