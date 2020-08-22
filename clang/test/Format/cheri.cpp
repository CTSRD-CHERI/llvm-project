// RUN: grep -Ev "// *[A-Z-]+:" %s | clang-format -style=LLVM \
// RUN:   | FileCheck -strict-whitespace %s

int x;
// CHECK: {{^void\ \*\ __capability\ p\ =\ \(void\ \*\ __capability\)&x;$}}
 void  *__capability  p = (void*  __capability)& x;
// CHECK: {{^uintcap_t\ a\ =\ \(__cheri_tocap\ uintcap_t\)&x;$}}
 uintcap_t  a = (__cheri_tocap uintcap_t)& x;
// CHECK: {{^vaddr_t\ a\ =\ \(__cheri_fromcap\ vaddr_t\)&x;$}}
 vaddr_t  a = (__cheri_fromcap vaddr_t)& x;
// CHECK: {{^vaddr_t\ a\ =\ \(__cheri_offset\ vaddr_t\)&x;$}}
 vaddr_t  a = (__cheri_offset vaddr_t)& x;
// CHECK: {{^vaddr_t\ a\ =\ \(__cheri_addr\ vaddr_t\)&x;$}}
 vaddr_t  a = (__cheri_addr vaddr_t)& x;
// CHECK: {{^vaddr_t\ a\ =\ \(__cheri_cast\ vaddr_t\)&x;$}}
 vaddr_t  a = (__cheri_cast vaddr_t)& x;
