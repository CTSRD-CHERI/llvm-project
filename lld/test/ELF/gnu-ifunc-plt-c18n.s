// REQUIRES: x86
// RUN: split-file %s %t

/// Test references to non-preemptable ifuncs from both the same
/// compartment and a foreign compartment.

// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %t/one.s -o %t/one.o
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %t/oneb.s -o %t/oneb.o
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %t/two.s -o %t/two.o

// RUN: ld.lld --compartment-policy=%t/one.json %t/one.o %t/oneb.o -o %t/only_one.so
// RUN: ld.lld --compartment-policy=%t/one.json --compartment-policy=%t/two.json %t/one.o %t/oneb.o %t/two.o -o %t/both.so
// RUN: ld.lld --compartment-policy=%t/two.json --compartment-policy=%t/one.json %t/two.o %t/one.o %t/oneb.o -o %t/both_reverse.so
// RUN: ld.lld --compartment-policy=%t/one.json --compartment-policy=%t/two.json %t/one.o %t/two.o -o %t/only_two.so

// RUN: llvm-objdump -h -t -d %t/only_one.so | FileCheck --check-prefixes=CHECK,ONE %s
// RUN: llvm-objdump -h -t -d %t/only_two.so | FileCheck --check-prefixes=CHECK,TWO %s
// RUN: llvm-objdump -h -t -d %t/both.so | FileCheck --check-prefixes=CHECK,ONE,TWO %s
// RUN: llvm-objdump -h -t -d %t/both_reverse.so | FileCheck --check-prefixes=CHECK,ONE %s
// RUN: llvm-objdump -h -t -d %t/both_reverse.so | FileCheck --check-prefixes=CHECK,TWO %s

// CHECK-LABEL: Sections:
// CHECK-DAG: {{[0-9]+}} .iplt.one {{[0-9a-f]+}} [[#%.16x,FOO_ADDR:]] TEXT
// TWO-DAG:   {{[0-9]+}} .iplt.two {{[0-9a-f]+}} [[#%.16x,IPLT_TWO_ADDR:]] TEXT

// CHECK-LABEL: SYMBOL TABLE:
// CHECK:     [[#%.16x,FOO_ADDR]] l     F .iplt.one      0000000000000000 .hidden foo

// ONE-LABEL: Disassembly of section .text.one:
// ONE: <one_call>:
// ONE-NEXT: callq   0x[[#%x,FOO_ADDR]] <foo>
// ONE-NEXT: retq

// ONE: <one_value>:
// ONE-NEXT: leaq    0x[[#%x,FOO_ADDR]], %rax
// ONE-NEXT: retq

// TWO-LABEL: Disassembly of section .text.two:
// TWO: <two_call>:
// TWO-NEXT: callq   0x[[#%x,IPLT_TWO_ADDR]]
// TWO-NEXT: retq

// TWO: <two_value>:
// TWO-NEXT: leaq    0x[[#%x,FOO_ADDR]], %rax
// TWO-NEXT: retq

//--- one.s

	.text
	.global foo
	.type	foo, STT_GNU_IFUNC
	.hidden	foo
foo:
	ret
	.size	foo, . - foo

//--- oneb.s

	.text
	.global one_call
	.type	one_call, @function
one_call:
	call	foo
	ret
	.size	one_call, . - one_call

	.text
	.global	one_value
	.type	one_value, @function
one_value:
	lea	foo, %rax
	ret
	.size	one_value, . - one_value

//--- two.s

	.text
	.global two_call
	.type	two_call, @function
two_call:
	call	foo
	ret
	.size	two_call, . - two_call

	.text
	.global	two_value
	.type	two_value, @function
two_value:
	lea	foo, %rax
	ret
	.size	two_value, . - two_value

//--- one.json

{
    "compartments": {
	"one": { "files": ["*/one*.o"] }
    }
}

//--- two.json

{
    "compartments": {
	"two": { "files": ["*/two.o"] }
    }
}
