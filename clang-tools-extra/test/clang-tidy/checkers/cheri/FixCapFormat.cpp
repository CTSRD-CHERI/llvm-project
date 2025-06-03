// RUN: %check_clang_tidy %s cheri-FixCapFormat %t

#ifdef __CHERI__
typedef __uintcap_t uintptr_t;
#else
typedef unsigned long uintptr_t;
#endif
extern unsigned long __c_ua(uintptr_t);
extern int __dbg(int level, const char *fmt, ...);
extern void other(const char *, const char *, unsigned long);
struct printer {
  int (*dump)(int level, const char *fmt, ...);
  uintptr_t context_foo_bar_baz;
};

#define DBG_ERR(S,...) __dbg(10, S,##__VA_ARGS__)

void f(struct printer *p, uintptr_t arg_x)
{
  DBG_ERR("foobar %s %lu", "test", 10, arg_x);
// CHECK-MESSAGES: :[[@LINE-1]]:40: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
  DBG_ERR("foobar %s %lu", "test", 10, arg_x, 11);
// CHECK-MESSAGES: :[[@LINE-1]]:40: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
  p->dump(11, "foobar %s %lu", "test", 10, arg_x);
// CHECK-MESSAGES: :[[@LINE-1]]:44: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
  other("foobar %s %lu", "test", arg_x);
  __dbg(1, "bar %s %lu %lu", "test", 4711, arg_x);
// CHECK-MESSAGES: :[[@LINE-1]]:44: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
  __dbg(1, "bar %s %lu %lu");
  __dbg(arg_x, "bar %s %lu %lu", 17);
  p->dump(11, "foobar %s %lu", "test", 10, arg_x + 10);
// CHECK-MESSAGES: :[[@LINE-1]]:50: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
  DBG_ERR("foobar %s %lu", "test", 10, p->context_foo_bar_baz);
// CHECK-MESSAGES: :[[@LINE-1]]:40: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
  DBG_ERR("foobar %s %lu", "test", 10, p->context_foo_bar_baz, 10);
// CHECK-MESSAGES: :[[@LINE-1]]:40: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
  __dbg(1, "foobar %s %lu", "test", 10, p->context_foo_bar_baz);
// CHECK-MESSAGES: :[[@LINE-1]]:41: warning: CHERI: Integer capability passed to a variadic function. Need cast to 'unsigned long' [cheri-FixCapFormat]
}
