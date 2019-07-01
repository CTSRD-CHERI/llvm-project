// RUN: %cheri_purecap_cc1 -dM -E -x c++ /dev/null | %cheri_FileCheck %s
// CHECK: __STDCPP_DEFAULT_NEW_ALIGNMENT__ [[#CAP_SIZE]]UL
