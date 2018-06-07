// RUN: %cheri_cc1 -o - %s -fsyntax-only

int move_clobber(void * __capability x)
{
	__asm__ __volatile__ (
		"cmove $c16, %0" :: "C"(x) : "memory", "$c16", "$idc", "$cnull", "$ddc"
	);
	return 0;
}
