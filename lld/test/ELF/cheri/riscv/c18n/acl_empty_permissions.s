# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/two.s -o %t/two.o
# RUN: ld.lld --compartment-policy=%t/compartments.json %t/one.o -o %t/acl_empty_permissions.exe
# RUN: readelf --acls %t/acl_empty_permissions.exe | FileCheck --check-prefix=ONE %s
# RUN: not ld.lld --shared --compartment-policy=%t/compartments.json %t/two.o -o %t/acl_empty_permissions.so |& FileCheck --check-prefix=TWO %s

# ONE-LABEL: ACLs
# ONE-NEXT:    Subject: compartment one Permissions: --- Object: symbol *

# TWO: policy forbids accessing symbol bar from compartment one

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

#--- two.s

	.text
	.global	foo
	.type	foo, @function
foo:
	clgc	ct0, bar
	ret
	.size	foo, . - foo

#--- compartments.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"", "symbols":["*"] }
    ]
}
