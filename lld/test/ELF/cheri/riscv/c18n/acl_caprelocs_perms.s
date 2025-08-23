# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/three.s -o %t/three.o
# RUN: ld.lld --compartment-policy=%t/compartments.json %t/one.o %t/two.o %t/three.o -o %t/acl_caprelocs_perms.exe
# RUN: readelf -t --acls --cap-relocs %t/acl_caprelocs_perms.exe | FileCheck %s

# CHECK-LABEL: Section Headers:
# CHECK:         [ 6] .got.one
# CHECK-NEXT:         PROGBITS        0000000000012480 000480 000030 00   0   0  16
# CHECK:         [10] .got.two
# CHECK-NEXT:         PROGBITS        0000000000015cd0 000cd0 000030 00   0   0  16

# CHECK-LABEL: ACLs
# CHECK-NEXT:    Subject: compartment one Permissions: r-- Object: symbol buf
# CHECK-NEXT:    Subject: compartment one Permissions: rw- Object: symbol buf2
# CHECK-NEXT:    Subject: compartment two Permissions: rw- Object: compartment three

# CHECK-LABEL: CHERI __cap_relocs
# relocs for compartment one
# CHECK-NEXT:     0x012490 Base: 0x134b0 (buf) Length: 1024 Perms: Constant
# CHECK-NEXT:     0x0124a0 Base: 0x138b0 (buf2) Length: 1024 Perms: Object
# relocs for compartment two
# CHECK-NEXT:     0x015ce0 Base: 0x134b0 (buf) Length: 1024 Perms: Object
# CHECK-NEXT:     0x015cf0 Base: 0x138b0 (buf2) Length: 1024 Perms: Object

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
	{ "subject":"one", "permissions":"rw", "symbols":["buf2"] },
	{ "subject":"two", "permissions":"rw", "compartments":["three"] }
    ]
}
