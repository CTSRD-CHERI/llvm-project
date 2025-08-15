# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/one.s -o %t/one.o
# RUN: not ld.lld --shared --compartment-policy=%t/no_subject.json %t/one.o -o %t/acl_invalid.so 2>&1 | FileCheck --check-prefix=NO_SUBJECT %s
# RUN: not ld.lld --shared --compartment-policy=%t/no_permissions.json %t/one.o -o %t/acl_invalid.so 2>&1 | FileCheck --check-prefix=NO_PERMISSIONS %s
# RUN: not ld.lld --shared --compartment-policy=%t/no_object.json %t/one.o -o %t/acl_invalid.so 2>&1 | FileCheck --check-prefix=NO_OBJECT %s
# RUN: not ld.lld --shared --compartment-policy=%t/both_objects.json %t/one.o -o %t/acl_invalid.so 2>&1 | FileCheck --check-prefix=BOTH_OBJECTS %s
# RUN: not ld.lld --shared --compartment-policy=%t/bad_old_subject.json %t/one.o -o %t/acl_invalid.so 2>&1 | FileCheck --check-prefix=BAD_OLD_SUBJECT %s
# RUN: not ld.lld --shared --compartment-policy=%t/empty_permissions.json %t/one.o -o %t/acl_invalid.so 2>&1 | FileCheck --check-prefix=INVALID_PERMISSIONS %s
# RUN: not ld.lld --shared --compartment-policy=%t/invalid_permissions.json %t/one.o -o %t/acl_invalid.so 2>&1 | FileCheck --check-prefix=INVALID_PERMISSIONS %s

# NO_SUBJECT: missing value at (root).acls[0].subject
# NO_PERMISSIONS: missing value at (root).acls[0].permissions
# NO_OBJECT: no rule objects specified at (root).acls[0]
# BOTH_OBJECTS: cannot specify both "object" and "symbols" and/or "compartments" at (root).acls[0]
# BAD_OLD_SUBJECT: missing value at (root).acls[0].subject.name
# INVALID_PERMISSIONS: invalid permissions at (root).acls[0].permissions

#--- one.s

	.text
	.global	foo
	.type	foo, @function
foo:
	ret
	.size	foo, . - foo

#--- no_subject.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "permissions":"rwx", "symbols":["*"] }
    ]
}

#--- no_permissions.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "symbols":["*"] }
    ]
}

#--- no_object.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"rwx" }
    ]
}

#--- both_objects.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"rwx", "symbols":["*"],
	  "object":{"type" :"symbol", "name":"*" } }
    ]
}

#--- bad_old_subject.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":{}, "permissions":"rwx", "symbols":["*"] }
    ]
}

#--- empty_permissions.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"", "symbols":["*"] }
    ]
}

#--- invalid_permissions.json

{
    "compartments": {
	"one": { "symbols": ["foo"] }
    },
    "acls": [
	{ "subject":"one", "permissions":"abc", "symbols":["*"] }
    ]
}
