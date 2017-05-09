// RUN:  %clang_cc1 "-triple" "cheri-unknown-freebsd" "-target-abi" "purecap" -o - -O0 -emit-llvm %s | FileCheck %s -enable-var-scope

double test_long(void) {
  double d = 1234.5678;
  long l = d;
  double d2 = l;
  return d2;
  // CHECK-LABEL: define double @test_long()
  // CHECK: store double 0x40934A456D5CFAAD, double addrspace(200)* [[D:%.+]], align 8
  // CHECK: [[TMP:%.+]] = load double, double addrspace(200)* [[D]], align 8
  // CHECK: [[CONV:%.+]] = fptosi double [[TMP]] to i64

}

double test_intcap(void) {
  double d = 1234.5678;
  __intcap_t cap = d;
  double d2 = cap;
  return d2;
  // CHECK-LABEL: define double @test_intcap()
  // CHECK: store double 0x40934A456D5CFAAD, double addrspace(200)* [[D:%.+]], align 8
  // CHECK: [[TMP:%.+]] = load double, double addrspace(200)* [[D]], align 8
  // CHECK: [[CONV:%.+]] = fptosi double [[TMP]] to i64
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 [[CONV]])
}

double test_ulong(void) {
  double d = 1234.5678;
  unsigned long l = d;
  double d2 = l;
  return d2;
  // CHECK-LABEL: define double @test_ulong()
  // CHECK: store double 0x40934A456D5CFAAD, double addrspace(200)* [[D:%.+]], align 8
  // CHECK: [[TMP:%.+]] = load double, double addrspace(200)* [[D]], align 8
  // CHECK: [[CONV:%.+]] = fptoui double [[TMP]] to i64
}

double test_uintcap(void) {
  double d = 1234.5678;
  __uintcap_t cap = d;
  double d2 = cap;
  return d2;
  // CHECK-LABEL: define double @test_uintcap()
  // CHECK: store double 0x40934A456D5CFAAD, double addrspace(200)* [[D:%.+]], align 8
  // CHECK: [[TMP:%.+]] = load double, double addrspace(200)* [[D]], align 8
  // CHECK: [[CONV:%.+]] = fptoui double [[TMP]] to i64
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 [[CONV]])
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
