// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -emit-llvm | FileCheck %s
// XFAIL: *

int move_clobber(__capability void * x)
{
	__asm__ __volatile__ (
		"cmove $c16, %0" :: "C"(x) : "memory", "c16"
	);
}
