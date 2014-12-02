// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only

int move_clobber(__capability void * x)
{
	__asm__ __volatile__ (
		"cmove $c16, %0" :: "C"(x) : "memory", "$c16", "$idc", "$c0"
	);
}
