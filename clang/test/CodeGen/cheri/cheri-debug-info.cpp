// XFAIL: *
// For now this test will always fail because dereferencing a capability does not
// give a capability reference

// RUN: %cheri128_cc1 --std=c++11 -emit-llvm -debug-info-kind=limited -o - %s \
// RUN:   | FileCheck %s --check-prefix=CHECK-HYBRID --check-prefix=CHECK-128
// RUN: %cheri256_cc1 --std=c++11 -emit-llvm -debug-info-kind=limited -o - %s \
// RUN:   | FileCheck %s --check-prefix=CHECK-HYBRID --check-prefix=CHECK-256
// RUN: %cheri128_cc1 --std=c++11 -emit-llvm -debug-info-kind=limited \
// RUN:   -target-abi purecap -o - %s \
// RUN:   | FileCheck %s --check-prefix=CHECK-PUREABI --check-prefix=CHECK-128
// RUN: %cheri256_cc1 --std=c++11 -emit-llvm -debug-info-kind=limited \
// RUN:   -target-abi purecap -o - %s \
// RUN:   | FileCheck %s --check-prefix=CHECK-PUREABI --check-prefix=CHECK-256

// Check that CHERI capability pointer/references have the right size in debug
// information.

char *pointer;
char &reference = *pointer;
char &&rvalue_reference = 0;
char *__capability always_capability_pointer;
char &__capability always_capability_reference = *always_capability_pointer;
char &&__capability always_capability_rvalue_reference = 0;

// CHECK-HYBRID-DAG: ![[DWARF_SIMPLE_POINTER:[0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !{{[0-9]+}}, size: 64)
// CHECK-HYBRID-DAG: ![[DWARF_SIMPLE_REFERENCE:[0-9]+]] = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !{{[0-9]+}}, size: 64)
// CHECK-HYBRID-DAG: ![[DWARF_SIMPLE_RVALUE_REFERENCE:[0-9]+]] = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !{{[0-9]+}}, size: 64)
// CHECK-128-DAG: ![[DWARF_CAPABILITY_POINTER:[0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !{{[0-9]+}}, size: 128)
// CHECK-128-DAG: ![[DWARF_CAPABILITY_REFERENCE:[0-9]+]] = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !{{[0-9]+}}, size: 128)
// CHECK-128-DAG: ![[DWARF_CAPABILITY_RVALUE_REFERENCE:[0-9]+]] = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !{{[0-9]+}}, size: 128)
// CHECK-256-DAG: ![[DWARF_CAPABILITY_POINTER:[0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !{{[0-9]+}}, size: 256)
// CHECK-256-DAG: ![[DWARF_CAPABILITY_REFERENCE:[0-9]+]] = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !{{[0-9]+}}, size: 256)
// CHECK-256-DAG: ![[DWARF_CAPABILITY_RVALUE_REFERENCE:[0-9]+]] = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !{{[0-9]+}}, size: 256)

// CHECK-HYBRID-DAG: ![[POINTER_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "pointer", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_SIMPLE_POINTER]], isLocal: false, isDefinition: true)
// CHECK-HYBRID-DAG: ![[REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_SIMPLE_REFERENCE]], isLocal: false, isDefinition: true)
// CHECK-HYBRID-DAG: ![[RVALUE_REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "rvalue_reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_SIMPLE_RVALUE_REFERENCE]], isLocal: false, isDefinition: true)
// CHECK-HYBRID-DAG: ![[ALWAYS_CAPABILITY_POINTER_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "always_capability_pointer", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_POINTER]], isLocal: false, isDefinition: true)
// CHECK-HYBRID-DAG: ![[ALWAYS_CAPABILITY_REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "always_capability_reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_REFERENCE]], isLocal: false, isDefinition: true)
// CHECK-HYBRID-DAG: ![[ALWAYS_CAPABILITY_RVALUE_REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "always_capability_rvalue_reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_RVALUE_REFERENCE]], isLocal: false, isDefinition: true)

// CHECK-PUREABI-DAG: ![[POINTER_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "pointer", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_POINTER]], isLocal: false, isDefinition: true)
// CHECK-PUREABI-DAG: ![[REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_REFERENCE]], isLocal: false, isDefinition: true)
// CHECK-PUREABI-DAG: ![[RVALUE_REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "rvalue_reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_RVALUE_REFERENCE]], isLocal: false, isDefinition: true)
// CHECK-PUREABI-DAG: ![[ALWAYS_CAPABILITY_POINTER_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "always_capability_pointer", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_POINTER]], isLocal: false, isDefinition: true)
// CHECK-PUREABI-DAG: ![[ALWAYS_CAPABILITY_REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "always_capability_reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_REFERENCE]], isLocal: false, isDefinition: true)
// CHECK-PUREABI-DAG: ![[ALWAYS_CAPABILITY_RVALUE_REFERENCE_VAR:[0-9]+]] = distinct !DIGlobalVariable(name: "always_capability_rvalue_reference", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, type: ![[DWARF_CAPABILITY_RVALUE_REFERENCE]], isLocal: false, isDefinition: true)
