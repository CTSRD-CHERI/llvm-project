# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/three.s -o %t/three.o
# RUN: ld.lld --compartment-policy=%t/compartments.json %t/one.o %t/two.o %t/three.o -o %t/acl_default_permissions.exe
# RUN: readelf -t --acls --cap-relocs %t/acl_default_permissions.exe | FileCheck %s

# CHECK-LABEL: Section Headers:
# CHECK:         [ 6] .got.one
# CHECK-NEXT:         PROGBITS        0000000000012470 000470 000030 00   0   0  16
# CHECK:         [10] .got.two
# CHECK-NEXT:         PROGBITS        0000000000015cc0 000cc0 000030 00   0   0  16

# CHECK-LABEL: ACLs
# CHECK-NEXT:    Subject: compartment one Permissions: r-- Object: symbol buf
# CHECK-NEXT:    Subject: compartment one Permissions: rw- Object: symbol buf2

# CHECK-LABEL: CHERI capability relocation section '__cap_relocs'
# CHECK-NEXT:      Offset             Info         Type        Value
# relocs for compartment one
# CHECK-NEXT:  0000000000012480  4000000000000000 RODATA  00000000000134a0 [00000000000134a0-00000000000138a0]
# CHECK-NEXT:  0000000000012490  0000000000000000 DATA    00000000000138a0 [00000000000138a0-0000000000013ca0]
# relocs for compartment two
# CHECK-NEXT:  0000000000015cd0  0000000000000000 DATA    00000000000134a0 [00000000000134a0-00000000000138a0]
# CHECK-NEXT:  0000000000015ce0  0000000000000000 DATA    00000000000138a0 [00000000000138a0-0000000000013ca0]

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	clgc	ct0, buf
	clgc	ct1, buf2
	ret
	.size	foo, . - foo

#--- two.s

	.text
	.global bar
	.type	bar, @function
bar:
	clgc	ct0, buf
	clgc	ct1, buf2
	ret
	.size	bar, . - bar

#--- three.s

	.data
	.global buf
	.type	buf, @object
buf:	.space	1024
	.size	buf, . - buf

	.global buf2
	.type	buf2, @object
buf2:	.space	1024
	.size	buf2, . - buf2

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] },
	"three": { "symbols": ["buf","buf2"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"r", "symbols":["buf"] },
	{ "subject":"one", "permissions":"rw", "symbols":["buf2"] }
    ]
}
