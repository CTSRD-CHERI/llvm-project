# REQUIRES: mips
# RUN: llvm-mc -filetype=obj -triple=mips64-none-freebsd %s -o %t.o
# RUN: ld.lld -shared %t.o -o %t.so
# RUN: llvm-objdump -d %t.so | FileCheck %s
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
	.set	noreorder
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

## Check that the R_MIPS_JALR relocations did not change jalr to bal:
# CHECK-LABEL: Disassembly of section .text:
# CHECK-EMPTY:
# CHECK-NEXT: test:
# CHECK-NEXT:    103d0: df 99 80 20                  	ld	$25, -32736($gp)
# CHECK-NEXT:    103d4: 03 20 f8 09                  	jalr	$25
# CHECK-NEXT:    103d8: 00 00 00 00                  	nop
# CHECK-NEXT:    103dc: 3c 01 00 00                  	lui	$1, 0
# CHECK-NEXT:    103e0: 00 22 08 2d                  	daddu	$1, $1, $2
# CHECK-NEXT:    103e4: dc 39 80 00                  	ld	$25, -32768($1)
# CHECK-NEXT:    103e8: 03 20 00 08                  	jr	$25
# CHECK-NEXT:    103ec: 00 00 00 00                  	nop

