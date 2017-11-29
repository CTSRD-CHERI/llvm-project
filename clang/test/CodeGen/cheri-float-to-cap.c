// RUN:  %cheri_purecap_cc1 -o - -O0 -emit-llvm %s | FileCheck %s -enable-var-scope
// RUN:  %cheri_purecap_cc1 -o - -O2 -emit-llvm %s | FileCheck %s -check-prefix OPT

double test_long(void) {
  double d = 1234.5678;
  long l = d;
  double d2 = l;
  return d2;
  // CHECK-LABEL: define double @test_long()
  // CHECK: store double 0x40934A456D5CFAAD, double addrspace(200)* [[D:%.+]], align 8
  // CHECK: [[TMP:%.+]] = load double, double addrspace(200)* [[D]], align 8
  // CHECK: [[CONV:%.+]] = fptosi double [[TMP]] to i64
  // CHECK: store i64 [[CONV]], i64 addrspace(200)* [[L:%.+]], align 8
  // CHECK: [[TMP2:%.+]] = load i64, i64 addrspace(200)* [[L]], align 8
  // CHECK: [[CONV2:%.+]] = sitofp i64 [[TMP2]] to double


  // OPT-LABEL: define double @test_long()
  // OPT: ret double 1.234000e+03
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
  // CHECK: [[TMP_CAP:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 [[CONV]])
  // CHECK: store i8 addrspace(200)* [[TMP_CAP]], i8 addrspace(200)* addrspace(200)* [[CAP:%.+]], align [[$CAP_ALIGN:16|32]]
  // CHECK: [[TMP_CAP2:%.+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[CAP]], align [[$CAP_ALIGN]]
  // CHECK: [[CAP_OFFSET:%.+]] = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[TMP_CAP2]])
  // CHECK: [[CONV2:%.+]] = sitofp i64 [[CAP_OFFSET]] to double


  // OPT-LABEL: define double @test_intcap()
  // OPT: ret double 1.234000e+03
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
  // CHECK: store i64 [[CONV]], i64 addrspace(200)* [[L:%.+]], align 8
  // CHECK: [[TMP2:%.+]] = load i64, i64 addrspace(200)* [[L]], align 8
  // CHECK: [[CONV2:%.+]] = uitofp i64 [[TMP2]] to double

  // OPT-LABEL: define double @test_ulong()
  // OPT: ret double 1.234000e+03
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
  // CHECK: [[TMP_CAP:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 [[CONV]])
  // CHECK: store i8 addrspace(200)* [[TMP_CAP]], i8 addrspace(200)* addrspace(200)* [[CAP:%.+]], align [[$CAP_ALIGN]]
  // CHECK: [[TMP_CAP2:%.+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[CAP]], align [[$CAP_ALIGN]]
  // CHECK: [[CAP_OFFSET:%.+]] = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[TMP_CAP2]])
  // CHECK: [[CONV2:%.+]] = uitofp i64 [[CAP_OFFSET]] to double

  // OPT-LABEL: define double @test_uintcap()
  // OPT: ret double 1.234000e+03
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
