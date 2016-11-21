// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -o - %s | FileCheck %s

class time_point {
  public:
    // CHECK: call void @_ZN10time_pointC1Ev(%class.time_point addrspace(200)* @t1)
    time_point() { }
};

time_point t1;
