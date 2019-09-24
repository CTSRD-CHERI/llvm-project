// RUN: %cheri_cc1 -emit-pch -o %t.1.ast %S/Inputs/var1.c
// RUN: %cheri_cc1 -emit-pch -o %t.2.ast %S/Inputs/var2.c
// RUN: not %cheri_cc1 -ast-merge %t.1.ast -ast-merge %t.2.ast -fsyntax-only -fdiagnostics-show-note-include-stack %s -ast-dump -Werror=odr 2>%t.err | FileCheck %s -check-prefix AST
// RUN: FileCheck %s -input-file=%t.err

// CHECK: var2.c:5:21: error: external variable 'should_not_match' declared with incompatible types in different translation units ('char * __capability' vs. 'char *')
// CHECK: var1.c:5:8: note: declared here with type 'char *'
// CHECK: var2.c:6:9: error: external variable 'wrong_type' declared with incompatible types in different translation units ('float *' vs. 'int *')
// CHECK: var1.c:6:7: note: declared here with type 'int *'
// CHECK: var2.c:9:20: error: external function 'fn_decl_cap_mismatch' declared with incompatible types in different translation units ('void * __capability (int * __capability)' vs. 'void *(int *)')
// CHECK: var1.c:9:7: note: declared here with type 'void *(int *)'
// CHECK: 3 errors


// AST: |-VarDecl [[NOTCAP_ADDR:0x.+]] <{{.+}}var1.c:1:1, col:6> col:6 notcap 'int *'
// AST: |-VarDecl [[CAP_ADDR:0x.+]] <line:2:1, col:20> col:20 cap 'int * __capability'
// AST: |-VarDecl [[CHARPTR_ADDR:0x.+]] <line:3:1, col:8> col:8 charptr 'char *'
// AST: |-VarDecl [[CHARCAP_ADDR:0x.+]] <line:4:1, col:21> col:21 charcap 'char * __capability'
// AST: |-VarDecl 0x{{.+}} <line:5:1, col:8> col:8 should_not_match 'char *'
// AST: |-VarDecl 0x{{.+}} <line:6:1, col:7> col:7 wrong_type 'int *'
// AST: |-VarDecl 0x{{.+}} <line:7:1, col:8> col:8 foo 'double'
// AST: |-FunctionDecl 0x{{.+}} <line:9:1, col:50> col:7 fn_decl_cap_mismatch 'void *(int *)'
// AST: |-VarDecl 0x{{.+}} prev [[NOTCAP_ADDR]] <{{.+}}var2.c:1:1, col:6> col:6 notcap 'int *'
// AST: |-VarDecl 0x{{.+}} prev [[CAP_ADDR]] <line:2:1, col:20> col:20 cap 'int * __capability'
// AST: |-VarDecl 0x{{.+}} prev [[CHARPTR_ADDR]] <line:3:1, col:8> col:8 charptr 'char *'
// AST: |-VarDecl 0x{{.+}} prev [[CHARCAP_ADDR]] <line:4:1, col:21> col:21 charcap 'char * __capability'
