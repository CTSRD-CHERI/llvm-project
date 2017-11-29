// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s

class time_point {
  public:
    // CHECK: call void @_ZN10time_pointC1Ev(%class.time_point addrspace(200)* @t1)
    time_point() { }
};

time_point t1;
