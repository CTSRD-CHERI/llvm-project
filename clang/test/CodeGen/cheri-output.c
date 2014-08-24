// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -emit-llvm | FileCheck %s

int write_only(__capability __output int *x);

int caller(__capability int *x)
{
	// Ensure that the read and read capability flags are cleared.gq
	// CHECK: llvm.mips.and.cap.perms(
	// CHECK: i64 65515)
	return write_only(x);
}

