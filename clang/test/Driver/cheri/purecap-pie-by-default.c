
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s -pie -shared 2>&1 | FileCheck -check-prefix PIE-AND-SHARED %s
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s -shared -pie 2>&1 | FileCheck -check-prefix PIE-AND-SHARED %s
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s -pie -r 2>&1 | FileCheck -check-prefix PIE-AND-RELOCATABLE %s
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s -r -pie 2>&1 | FileCheck -check-prefix PIE-AND-RELOCATABLE %s

// PIE-AND-SHARED: error: invalid argument '-pie' not allowed with '-shared'
// PIE-AND-RELOCATABLE: error: invalid argument '-pie' not allowed with '-r'

// RUN: %cheri_clang -### -no-canonical-prefixes %s 2>&1 | FileCheck -check-prefix HYBRID %s
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 | FileCheck -check-prefix PURECAP %s
// HYBRID-NOT: "-pie"
// PURECAP: "{{.+}}/ld" "-pie"
// PURECAP: "-o" "a.out" "{{[^"]*}}Scrt1.o"

// But for -static, -r, and -pie we don't want to add -pie
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -static | FileCheck -check-prefix PURECAP-STATIC %s
// PURECAP-STATIC: "{{.+}}/ld"
// PURECAP-STATIC-NOT: "-pie"
// PURECAP-STATIC:     "-Bstatic"
// PURECAP-STATIC-NOT: "-pie"
// PURECAP-STATIC: "-o" "a.out" "{{[^"]*}}crt1.o"
// PURECAP-STATIC-NOT: "-pie"

// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -shared | FileCheck -check-prefix PURECAP-SHARED %s
// PURECAP-SHARED: "{{.+}}/ld"
// PURECAP-SHARED-NOT: "-pie"
// PURECAP-SHARED: "-Bshareable"
// PURECAP-SHARED-NOT: "-pie"
// PURECAP-SHARED: "-o" "a.out" "{{[^"]*}}crtbeginS.o"
// PURECAP-SHARED-NOT: "-pie"

// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -r | FileCheck -check-prefix PURECAP-RELOCATABLE %s
// PURECAP-RELOCATABLE: "{{.+}}/ld"
// PURECAP-RELOCATABLE-NOT: "-pie"
// PURECAP-RELOCATABLE: "-o" "a.out"
// PURECAP-RELOCATABLE-NOT: "-pie"
// PURECAP-RELOCATABLE: "-r"
// PURECAP-RELOCATABLE-NOT: "-pie"

// -no-pie and -nopie should both disable pie:
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -nopie | FileCheck -check-prefix PURECAP-NOPIE %s
// RUN: %cheri_purecap_clang -### -no-canonical-prefixes %s 2>&1 -no-pie | FileCheck -check-prefix PURECAP-NOPIE %s
// PURECAP-NOPIE: "{{.+}}/ld"
// PURECAP-NOPIE-NOT: "-pie"
// PURECAP-NOPIE: "-o" "a.out" "{{[^"]*}}crt1.o"
// PURECAP-NOPIE-NOT: "-pie"
