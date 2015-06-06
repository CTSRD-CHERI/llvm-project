// RUN: %clang_cc1 "-cc1" "-triple" "cheri-unknown-freebsd" "-emit-llvm" "-target-abi" "sandbox" "-o" "-" %s -O2 | FileCheck %s
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
// CHECK: call i8 addrspace(200)* @render_char(i64 inreg 1, i64 inreg 8589934595, i64 inreg 17179869189, i64 inreg 25769803776)
	render_char((struct thingy){1,{2,3,4,5,6}});
	return render_char(b);
}
