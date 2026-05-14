# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o %t/two.o -o %t/acl_dynrelocs_perms.so
# RUN: llvm-readelf -t -d -r %t/acl_dynrelocs_perms.so | FileCheck %s

# CHECK-LABEL: Section Headers:
# CHECK:         [ 5] .rela.dyn
# CHECK:         [ 6] .rela.dyn.flags
# CHECK-NEXT:         CHERI_RELFLAGS  [[#%x,FLAGS:]] 0005b0 000014 04   5   0  4
# CHECK:         [12] .got.one
# CHECK-NEXT:         PROGBITS        [[#%x,GOTONE:]]
# CHECK:         [18] .got.two
# CHECK-NEXT:         PROGBITS        [[#%x,GOTTWO:]]

# CHECK-LABEL: Dynamic section at offset
# CHECK:         0x0000000064348454 (CHERI_RELFLAGS) 0x[[#%x,FLAGS]]

# CHECK-LABEL: Relocation section '.rela.dyn'
# CHECK-NEXT:      Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
# CHECK-NEXT:  [[#%.16x,GOTONE+0x10]]  00000001000000c1 R_RISCV_CHERI_CAPABILITY 0000000000000000 baz + 0{{$}}
# CHECK-NEXT:  [[#%.16x,GOTONE+0x20]]  00000002000000c1 R_RISCV_CHERI_CAPABILITY 0000000000000000 buf + 0 (RO)
# CHECK-NEXT:  [[#%.16x,GOTTWO+0x10]]  00000002000000c1 R_RISCV_CHERI_CAPABILITY 0000000000000000 buf + 0{{$}}
# CHECK-NEXT:  [[#%.16x,GOTONE+0x30]]  00000003000000c1 R_RISCV_CHERI_CAPABILITY 0000000000000000 buf2 + 0{{$}}
# CHECK-NEXT:  [[#%.16x,GOTTWO+0x20]]  00000003000000c1 R_RISCV_CHERI_CAPABILITY 0000000000000000 buf2 + 0{{$}}

#--- one.s

	.type	baz, @function

	.text
	.global	foo
	.type	foo, @function
foo:
	clgc	ct0, buf
	clgc	ct1, buf2
	clgc	ct2, baz
	ret
	.size	foo, . - foo

#--- two.s

	.type	baz, @function

	.text
	.global bar
	.type	bar, @function
bar:
	clgc	ct0, buf
	clgc	ct1, buf2
	ccall	baz
	ret
	.size	bar, . - bar

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] },
	"two": { "symbols": ["bar"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"r", "symbols":["buf"] },
	{ "subject":"one", "permissions":"rw", "symbols":["buf2"] },
	{ "subject":"two", "permissions":"rw", "symbols":["buf","buf2"] },
	{ "subject":"*", "permissions":"x", "symbols":["baz"] }
    ]
}
