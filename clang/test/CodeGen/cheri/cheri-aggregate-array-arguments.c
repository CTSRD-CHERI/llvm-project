// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm | FileCheck %s
struct foo
{
	void *a;
	void *refs[4];
	void *b;
	void *c;
};
struct bar
{
	struct x {
		void *a;
		void *b;
	} y[4];
};

// CHECK: declare void @insert(ptr addrspace(200) inreg, ptr addrspace(200) inreg, ptr addrspace(200) inreg, ptr addrspace(200) inreg, ptr addrspace(200) inreg, ptr addrspace(200) inreg, ptr addrspace(200) inreg)
// CHECK: declare void @insertb({ ptr addrspace(200), ptr addrspace(200) } inreg, { ptr addrspace(200), ptr addrspace(200) } inreg, { ptr addrspace(200), ptr addrspace(200) } inreg, { ptr addrspace(200), ptr addrspace(200) } inreg)

void insert(struct foo b);
void insertb(struct bar b);

int main(int argc, char **argv)
{
	struct foo b;
	b.refs[0]=argv;
	insert(b);
	struct bar c;
	c.y[0].b=argv;
	insertb(c);
}
