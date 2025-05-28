// RUN: %check_clang_tidy %s cheri-FixCapAsArraySubscript %t
#ifdef __CHERI__
typedef __uintcap_t uintptr_t;
#else
typedef unsigned long uintptr_t;
#endif
struct driver {
  uintptr_t priv;
  unsigned long privu;
};

int ids[] = {
  1, 2, 3, -1
};

int getid1(struct driver *d) {
  return ids[(d->priv)];
// CHECK-MESSAGES: :[[@LINE-1]]:14: warning: CHERI: Array index expression has capability type. Wrap it in __c_ua() [cheri-FixCapAsArraySubscript]
}

int getid2(struct driver *d) {
  return ids[d->privu];
}

