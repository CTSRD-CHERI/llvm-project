// RUN: %check_clang_tidy %s cheri-FixUserPtrToAddr %t

#define __user [[clang::annotate_type("user")]]

extern unsigned long __c_pa(const volatile void *);
extern unsigned long __c_pa_u(const volatile void __user *);

int
f(void *kp, void __user *up)
{
  __c_pa(kp);
  __c_pa_u(kp);
// CHECK-MESSAGES: :[[@LINE-1]]:3: warning: CHERI: __c_pa_u() on kernel pointer. Should be __c_pa() [cheri-FixUserPtrToAddr]
  __c_pa(up);
// CHECK-MESSAGES: :[[@LINE-1]]:3: warning: CHERI: __c_pa() on user pointer. Should be __c_pa_u() [cheri-FixUserPtrToAddr]
  __c_pa_u(up);
}

