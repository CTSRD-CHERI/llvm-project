// RUN:  %clang_cc1 "-triple" "cheri-unknown-freebsd" "-target-abi" "purecap" -o - -O0 -emit-llvm %s
// XFAIL: *

void test(void) {
  double d = 1234.5678;
  // error: void* __capability v = d;
  __intcap_t cap = d;
}

// original minimized test case:
#if 0
typedef __intcap_t __intptr_t;
struct kevent {
  __intptr_t data;
};
int getopt(int, char *[], const char *);

struct {
  double pa_duration;
} args;
struct kevent main_kev;

int main(int argc, char **argv) {
  if (getopt(argc, argv, "CDEFGMNOPRSTWacdefgklmnopqrstvwz")) {
    struct kevent *kevp = &main_kev;
    kevp->data = args.pa_duration;
  }
  return 0;
}
#endif
