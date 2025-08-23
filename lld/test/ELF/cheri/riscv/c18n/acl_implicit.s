# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: ld.lld --shared --compartment-policy=%t/compartments.json %t/one.o -o %t/acl_implicit.so
# RUN: readelf -t -d -r --acls %t/acl_implicit.so | FileCheck %s

# `bar` symbol implicitly assigned to compartment one can be accessed
# without an explicit ACL.

# CHECK-LABEL: Section Headers:
# CHECK:         [ 5] .c18nstrtab
# CHECK-NEXT:         STRTAB          0000000000000364 000364 00000c 00   0   0  1
# CHECK-NEXT:         [0000000000000002]: ALLOC
# CHECK-NEXT:    [ 6] .c18nacl
# CHECK-NEXT:         C18N_ACL        0000000000000370 000370 00000c 00   5   0  4
# CHECK-NEXT:         [0000000000000002]: ALLOC

# CHECK-LABEL: Dynamic section
# CHECK:         0x0000000064331382 (C18N_ACL)                  0x370
# CHECK-NEXT:    0x0000000064331383 (C18N_ACLSZ)                12 (bytes)


# CHECK-LABEL: Relocation section '.rela.plt.one'
# CHECK:       0000000000004590  0000000300000005 R_RISCV_JUMP_SLOT      0000000000003524 bar + 0
# CHECK:       00000000000045a0  0000000100000005 R_RISCV_JUMP_SLOT      0000000000000000 printf + 0

# CHECK-LABEL: ACLs
# CHECK-NEXT:    Subject: compartment one Permissions: --x Object: symbol printf
# CHECK-NEXT:  ]

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ccall	bar
	ccall	printf@plt
	ret
	.size	foo, . - foo

	.text
	.global bar
	.type	bar, @function
bar:
	ret
	.size	bar, . - bar

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"x", "symbols":["printf"] }
    ]
}
