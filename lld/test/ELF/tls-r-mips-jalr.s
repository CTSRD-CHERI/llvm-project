# REQUIRES: mips
# RUN: llvm-mc -filetype=obj -triple=mips64-none-freebsd %s -o %t.o
# RUN: ld.lld -shared %t.o -o %t.so
## Minimized test case create from the following C source code
## typedef int (*callback_f)(void);
## static __thread callback_f callback;
## void set_callback(callback_f f) {
##   callback = f;
## }
## int test(void) {
##   return callback();
## }

	.set	noat
	.globl	test                    # -- Begin function test
	.p2align	3
	.type	test,@function
	.ent	test
test:                                   # @test
# %bb.0:                                # %entry
	ld	$t9, %call16(__tls_get_addr)($gp)
	.reloc .Ltmp1, R_MIPS_JALR, __tls_get_addr
.Ltmp1:
	jalr	$t9
	nop
	lui	$1, %dtprel_hi(callback)
	daddu	$1, $1, $2
	ld	$t9, %dtprel_lo(callback)($1)
## The following line (creating a R_MIPS_JALR against a TLS symbol)
## previously caused a linker error:
## tls-r-mips-jalr.s.tmp.o has an STT_TLS symbol but doesn't have an SHF_TLS section
	.reloc .Ltmp2, R_MIPS_JALR, callback
.Ltmp2:
	jr	$t9
	nop
	.end	test
.Lfunc_end1:
	.size	test, .Lfunc_end1-test


	.type	callback,@object        # @callback
	.section	.tbss,"awT",@nobits
	.p2align	3
callback:
	.8byte	0
	.size	callback, 8

