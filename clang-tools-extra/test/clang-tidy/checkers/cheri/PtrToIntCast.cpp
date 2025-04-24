// RUN: %check_clang_tidy %s cheri-PtrToIntCast %t --extra-arg=-target --extra-arg=riscv64-codasip-linux-musl --extra-arg=-march=rv64imafdc_zcherihybrid_zihintpause_zicbom_zcherilevels --extra-arg=-mabi=l64pc128d

#define __force
int foo1(__uintcap_t x) { return x; }

unsigned long foo2(__uintcap_t x) {
  return x;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: CHERI: Invalid capability to integer cast [cheri-PtrToIntCast]
}

int bar1(void *foo) { return !(((unsigned long)foo)); }

int bar2(void *foo) { return (unsigned long)foo; }

int bar3(__uintcap_t foo) { return (unsigned long)foo; }

int bar4(__uintcap_t foo) { return !(unsigned long)foo; }

int lt(__uintcap_t foo, void *bar) { return foo < (unsigned long)bar; }

unsigned long mod(__uintcap_t foo, void *bar) {
  return (unsigned long)bar % (unsigned long)foo;
}

unsigned long force(__uintcap_t foo, void *bar) {
  unsigned long foou = (unsigned long __force)foo;
  unsigned long baru = (unsigned long __force)bar;

  return foou + baru;
}

unsigned long bitand1(__uintcap_t foo, void *bar) {
  return (unsigned long)bar & (__force unsigned long)foo;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: CHERI: Invalid capability to integer cast [cheri-PtrToIntCast]
}

unsigned long bitand2a(__uintcap_t foo) { return (unsigned long)foo & 0xfffUL; }

unsigned long bitand2b(__uintcap_t foo) { return foo & 0xfffUL; }

unsigned long bitand3a(__uintcap_t foo) {
  return (unsigned long)foo & ~0xfffUL;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}}: warning: CHERI: Invalid capability to integer cast [cheri-PtrToIntCast]
}

unsigned long bitand3b(__uintcap_t foo) {
  return foo & ~0xfffUL;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}}: warning: CHERI: Invalid capability to integer cast [cheri-PtrToIntCast]
}

unsigned long bitand4a(__uintcap_t foo) {
  return (unsigned long)foo & (7 * 128);
}

unsigned long bitand4b(__uintcap_t foo) { return foo & (7 * 128); }

unsigned long foo3(__uintcap_t x) { return (unsigned __force long)x; }

int long c(void *ptr) {
  return !(((unsigned long)ptr + sizeof(void *) - 1) & (1024 - sizeof(void *)));
}

void *up(void) { return (void *)3UL; }

__uintcap_t do_or(__uintcap_t pval, unsigned long other) {
  return pval | other;
}

void *toptr(unsigned long arg) {
  return (void *)(__uintcap_t)arg;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}}: warning: CHERI: Invalid integer to capability cast [cheri-PtrToIntCast]
}

void *toptr2(unsigned long arg) { return (void *)(__uintcap_t __force)arg; }

void *toptr3(unsigned long arg) {
  return (void *)arg;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: CHERI: Invalid integer to capability cast [cheri-PtrToIntCast]
}

__uintcap_t tocap(unsigned long arg) {
  return arg;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: CHERI: Invalid integer to capability cast [cheri-PtrToIntCast]
}
__uintcap_t tocap2(unsigned long arg) {
  return arg;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: CHERI: Invalid integer to capability cast [cheri-PtrToIntCast]
}

#define NULL ((void *)0)
#define ONE ((void *)1)

unsigned long null(void) { return (unsigned long)NULL; }

unsigned long one(void) { return (unsigned long)ONE; }
