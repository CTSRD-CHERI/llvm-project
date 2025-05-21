// RUN: %check_clang_tidy %s cheri-FixDriverData %t

#ifdef __CHERI__
typedef __uintcap_t uintptr_t;
#else
typedef unsigned long uintptr_t;
#endif
typedef unsigned long __ptraddr_t;
typedef unsigned long __ptraddr64_t;
typedef unsigned long kernel_ulong_t;

struct s1 {
  unsigned long driver_data;
// CHECK-MESSAGES: :[[@LINE-1]]:17: warning: CHERI: driver data field should have type `uintptr_t` [cheri-FixDriverData]
  unsigned long driver_info;
// CHECK-MESSAGES: :[[@LINE-1]]:17: warning: CHERI: driver data field should have type `uintptr_t` [cheri-FixDriverData]
  unsigned long other;
};

struct s2 {
  uintptr_t driver_data;
  uintptr_t driver_info;
  uintptr_t other;
};

struct s3 {
  kernel_ulong_t driver_data;
// CHECK-MESSAGES: :[[@LINE-1]]:18: warning: CHERI: driver data field should have type `uintptr_t` [cheri-FixDriverData]
  kernel_ulong_t driver_info;
// CHECK-MESSAGES: :[[@LINE-1]]:18: warning: CHERI: driver data field should have type `uintptr_t` [cheri-FixDriverData]
  kernel_ulong_t other;
};

struct s4 {
  __ptraddr_t driver_data;
  __ptraddr_t driver_info;
  __ptraddr_t other;
};

struct s5 {
  __ptraddr64_t driver_data;
  __ptraddr64_t driver_info;
  __ptraddr64_t other;
};

struct s6 {
  unsigned int driver_data;
  unsigned int driver_info;
  unsigned int other;
};

struct s7 {
  unsigned int driver_data[10];
  unsigned int driver_info[10];
  unsigned int other[10];
};
