// RUN: %check_clang_tidy %s cheri-FixPtrToUlongCast %t

#define __user [[clang::annotate_type("user")]]
#ifdef __CHERI__
typedef __uintcap_t uintptr_t;
#else
typedef unsigned long uintptr_t;
#endif
typedef unsigned long kernel_ulong_t;
typedef unsigned long __ptraddr_t;

unsigned long f1(void *p)
{
  return (kernel_ulong_t)p;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:10: warning: Here: [cheri-FixPtrToUlongCast]
}

unsigned long f2(void __user *p)
{
  return (unsigned long)p;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'user_uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:10: warning: Here: [cheri-FixPtrToUlongCast]
}

unsigned long f3(void *p)
{
  return (uintptr_t)p;
}

void vars(void *p)
{
  unsigned long p1 = (unsigned long)p;
// CHECK-MESSAGES: :[[@LINE-1]]:22: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:22: warning: CHERI: Variable type should be 'uintptr_t' [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-3]]:22: warning: Here: [cheri-FixPtrToUlongCast]
  unsigned long p2 = (kernel_ulong_t)p;
// CHECK-MESSAGES: :[[@LINE-1]]:22: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:22: warning: CHERI: Variable type should be 'uintptr_t' [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-3]]:22: warning: Here: [cheri-FixPtrToUlongCast]
  unsigned long p3 = (uintptr_t)p;
// CHECK-MESSAGES: :[[@LINE-1]]:22: warning: CHERI: Variable type should be 'uintptr_t' [cheri-FixPtrToUlongCast]
  kernel_ulong_t p4 = (unsigned long)p;
// CHECK-MESSAGES: :[[@LINE-1]]:23: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:23: warning: CHERI: Variable type should be 'uintptr_t' [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-3]]:23: warning: Here: [cheri-FixPtrToUlongCast]
  kernel_ulong_t p5 = (kernel_ulong_t)p;
// CHECK-MESSAGES: :[[@LINE-1]]:23: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:23: warning: CHERI: Variable type should be 'uintptr_t' [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-3]]:23: warning: Here: [cheri-FixPtrToUlongCast]
  kernel_ulong_t p6 = (uintptr_t)p;
// CHECK-MESSAGES: :[[@LINE-1]]:23: warning: CHERI: Variable type should be 'uintptr_t' [cheri-FixPtrToUlongCast]
  uintptr_t p7 = (unsigned long)p;
// CHECK-MESSAGES: :[[@LINE-1]]:18: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:18: warning: Here: [cheri-FixPtrToUlongCast]
  uintptr_t p8 = (kernel_ulong_t)p;
// CHECK-MESSAGES: :[[@LINE-1]]:18: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:18: warning: Here: [cheri-FixPtrToUlongCast]
  uintptr_t p9 = (uintptr_t)p;
}

extern void doit(unsigned long arg);
extern void varargs(unsigned long arg, ...);
extern void addr(__ptraddr_t arg);
void check(void *ptr, void *ptr2)
{
  doit((unsigned long)ptr);
// CHECK-MESSAGES: :[[@LINE-1]]:8: warning: CHERI: Explicit cast from pointer to 'unsigned long'. Cast to 'uintptr_t' instead [cheri-FixPtrToUlongCast]
// CHECK-MESSAGES: :[[@LINE-2]]:8: warning: Here: [cheri-FixPtrToUlongCast]
  varargs((unsigned long)ptr);
  addr((unsigned long)ptr);
  doit((unsigned long)ptr & 0x17);
  doit((uintptr_t)ptr & ~0x17UL);
  doit((kernel_ulong_t)ptr - (unsigned long)ptr2);
}
