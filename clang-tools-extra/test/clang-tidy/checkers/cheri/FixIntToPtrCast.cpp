// RUN: %check_clang_tidy %s cheri-FixIntToPtrCast %t

void *f1(int x)
{
  return (((void*)(((x)))));
// CHECK-MESSAGES: :[[@LINE-1]]:19: warning: CHERI: Intermediate cast to 'uintptr_t' required [cheri-FixIntToPtrCast]
// CHECK-MESSAGES: :[[@LINE-2]]:19: warning: Here: [cheri-FixIntToPtrCast]
}

void *f2(int x)
{
  return (void*)x;
// CHECK-MESSAGES: :[[@LINE-1]]:17: warning: CHERI: Intermediate cast to 'uintptr_t' required [cheri-FixIntToPtrCast]
// CHECK-MESSAGES: :[[@LINE-2]]:17: warning: Here: [cheri-FixIntToPtrCast]
}

void *g1(void)
{
  return (((void*)(((17UL)))));
}

void *g2(void)
{
  return (void*)17U;
}

void *h1(int x)
{
  return (((void *)(((unsigned long)x))));
// CHECK-MESSAGES: :[[@LINE-1]]:22: warning: CHERI: Intermediate cast to 'uintptr_t' required [cheri-FixIntToPtrCast]
// CHECK-MESSAGES: :[[@LINE-2]]:22: warning: Here: [cheri-FixIntToPtrCast]
}

void *h2(int x)
{
  return (void *)(unsigned long)x;
// CHECK-MESSAGES: :[[@LINE-1]]:18: warning: CHERI: Intermediate cast to 'uintptr_t' required [cheri-FixIntToPtrCast]
// CHECK-MESSAGES: :[[@LINE-2]]:18: warning: Here: [cheri-FixIntToPtrCast]
}
