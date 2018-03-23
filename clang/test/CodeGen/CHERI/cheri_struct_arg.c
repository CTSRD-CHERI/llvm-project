// RUN: %cheri128_purecap_cc1 -emit-llvm -o - %s -O2 | FileCheck %s -check-prefixes CHECK,CHERI128
// RUN: %cheri256_purecap_cc1 -emit-llvm -o - %s -O2 | FileCheck %s -check-prefixes CHECK,CHERI256
struct thingy {
	unsigned long a;
	unsigned int b[5];
};
void *render_char(struct thingy);

// Make sure that capabilities are passed as pointers but structs containing
// multiple integers all have their values coerced to register-sized ints.
// CHECK: define i8 addrspace(200)* @f(i8 addrspace(200)* nocapture readnone %a, i64 inreg %b.coerce0, i64 inreg %b.coerce1, i64 inreg %b.coerce2, i64 inreg %b.coerce3)
void *f(void *a, struct thingy b)
{
// The magic numbers here are difficult to read in decimal, but are clearer in
// hex: they're the high 32 bits set to the first of a pair of 32-bit values
// and the low 32 bits set to the second value.
//
// CHECK: call i8 addrspace(200)* 
// CHECK: (i64 inreg 1, i64 inreg 8589934595, i64 inreg 17179869189, i64 inreg 25769803776)
	render_char((struct thingy){1,{2,3,4,5,6}});
	return render_char(b);
}

typedef struct heim_octet_string {
	long length;
	void *data;
} heim_octet_string;

// This is a bit ugly, as we're copying the struct padding in registers, but
// that's probably needed for stable memcmpy anyway.  This won't be as bad on
// CHERI128.
// CHERI256: declare void @h(i64 inreg, i64 inreg, i64 inreg, i64 inreg, i8 addrspace(200)* inreg)
// CHERI128: declare void @h(i64 inreg, i64 inreg, i8 addrspace(200)* inreg)
void h(heim_octet_string str);

void g(void)
{
	char buf[42];
	h((heim_octet_string){42, buf});
}

