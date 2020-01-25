// RUN: %cheri_cc1 -o - %s -emit-llvm | FileCheck %s

int write_only(__capability __cheri_output int *x);
int read_only(__capability __cheri_input int *x);

int caller(__capability int *x)
{
	// Ensure that the read and read capability flags are cleared.
	// CHECK: llvm.cheri.cap.perms.and.i64(
	// CHECK: i64 65515)
	// CHECK: write_only(
	write_only(x);
	// Ensure that the write and write-capability flags are cleared.
	// CHECK: llvm.cheri.cap.perms.and.i64(
	// CHECK: i64 65495)
	// CHECK: read_only(
	return read_only(x);
}

