# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two_hidden.s -o %t/two_hidden.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/three.s -o %t/three.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/three_hidden.s -o %t/three_hidden.o

# Only imply 'buf'
# RUN: ld.lld --shared --verbose-c18n --compartment-policy=%t/one.json %t/one.o -o %t/implicit_buf.so.0 2>&1 | FileCheck --check-prefixes=ONE,BUF %s

# 'baz' can't be implied because it is exported
# RUN: ld.lld --shared --verbose-c18n --compartment-policy=%t/one.json %t/one.o %t/two.o -o %t/implicit_baz_exported.so.0 2>&1 | FileCheck --check-prefixes=ONE,BUF,NOBAZ %s

# Hidden 'baz' can be implied
# RUN: ld.lld --shared --verbose-c18n --compartment-policy=%t/one.json %t/one.o %t/two_hidden.o -o %t/implicit_baz_hidden.so.0 2>&1 | FileCheck --check-prefixes=ONE,BUF,BAZ %s

# Neither 'bar' nor 'baz' can be implied because they are exported
# RUN: ld.lld --shared --verbose-c18n --compartment-policy=%t/one.json %t/one.o %t/two.o %t/three.o -o %t/implicit_bar_baz_exported.so.0 2>&1 | FileCheck --check-prefixes=ONE,BUF,NOBAZ,NOBAR %s

# 'bar' can't be implied because it is exported
# This also prevents implying the DSU, so 'baz' is silently not implied either
# RUN: ld.lld --shared --verbose-c18n --compartment-policy=%t/one.json %t/one.o %t/two_hidden.o %t/three.o -o %t/implicit_bar_exported.so.0 2>&1 | FileCheck --check-prefixes=ONE,BUF,NOBAR %s

# Hidden 'bar' and 'baz' can be implied
# RUN: ld.lld --shared --verbose-c18n --compartment-policy=%t/one.json %t/one.o %t/two_hidden.o %t/three_hidden.o -o %t/implicit_bar_baz.so.0 2>&1 | FileCheck --check-prefixes=ONE,BUF,BAR %s

# Exported 'bar' in separate compartment blocks implicit 'baz'
# RUN: ld.lld --shared --verbose-c18n --compartment-policy=%t/two.json %t/one.o %t/two_hidden.o %t/three.o -o %t/implicit_two.so.0 2>&1 | FileCheck --check-prefixes=ONE,TWO,BUF %s

# ONE: one.o:.text assigned to compartment one
# TWO: three.o:.text assigned to compartment two
# BUF: one.o:.data assigned to implied compartment one due to direct access of symbol buf
# NOBAZ: two.o:.data not assigned to implied compartment one due to exported symbol baz
# BAZ: info: assigning disjoint set to compartment one:
# BAZ-NEXT: two_hidden.o:.data
# NOBAR: three.o:.text not assigned to implied compartment one due to exported symbol bar
# BAR: info: assigning disjoint set to compartment one:
# BAR-NEXT: two_hidden.o:.data
# BAR-NEXT: three_hidden.o:.text
# TWO: info: disjoint set spans multiple compartments:
# TWO-NEXT: two_hidden.o:.data from the default compartment
# TWO-NEXT: three.o:.text from compartment two
# TWO-NEXT: one.o:.text from compartment one

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	cllc	ct0, buf
	clgc	ct1, baz
	ret
	.size	foo, . - foo

	.data
	.type	buf, @object
buf:
	.space 1024
	.size	buf, . - buf

#--- two.s

	.data
	.global	baz
	.type	baz, @object
baz:
	.space 1024
	.size	baz, . - baz

#--- two_hidden.s

	.data
	.global	baz
	.hidden baz
	.type	baz, @object
baz:
	.space 1024
	.size	baz, . - baz

#--- three.s

	.text
	.global	bar
	.type	bar, @function
bar:
	clgc	ct0, baz
	ret
	.size	bar, . - bar

#--- three_hidden.s

	.text
	.global	bar
	.hidden	bar
	.type	bar, @function
bar:
	clgc	ct0, baz
	ret
	.size	bar, . - bar

#--- one.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    }
}

#--- two.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] }
    }
}
