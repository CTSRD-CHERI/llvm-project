// RUN: %cheri_cc1 -o - %s -emit-llvm | FileCheck %s

int write_only(__capability __output int *x);
int read_only(__capability __input int *x);

int caller(__capability int *x)
{
	// Ensure that the read and read capability flags are cleared.
	// CHECK: llvm.cheri.cap.perms.and(
	// CHECK: i64 65515)
	// CHECK: write_only(
	write_only(x);
	// Ensure that the write and write-capability flags are cleared.
	// CHECK: llvm.cheri.cap.perms.and(
	// CHECK: i64 65495)
	// CHECK: read_only(
	return read_only(x);
}

