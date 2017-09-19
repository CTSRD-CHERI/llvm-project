// RUN: %cheri_cc1 -target-abi n64 -emit-llvm -o - %s -DDATA_SIZE=64 | FileCheck %s -check-prefix N64-EXPANDED
// RUN: %cheri_cc1 -target-abi n64 -emit-llvm -o - %s -DDATA_SIZE=72 | FileCheck %s -check-prefix N64-MEMCPY
// CHERI 256 expands up to 2048 bytes!
// RUN: %cheri256_cc1 -target-abi purecap -emit-llvm -o - %s -DDATA_SIZE=2048 | FileCheck %s -check-prefix EXPANDED
// RUN: %cheri256_cc1 -target-abi purecap -emit-llvm -o - %s -DDATA_SIZE=2056 | FileCheck %s -check-prefix MEMCPY
// 128 only expands up to 1024 bytes:
// RUN: %cheri128_cc1 -target-abi purecap -emit-llvm -o - %s -DDATA_SIZE=1024 | FileCheck %s -check-prefix EXPANDED
// RUN: %cheri128_cc1 -target-abi purecap -emit-llvm -o - %s -DDATA_SIZE=1032 | FileCheck %s -check-prefix MEMCPY

// The code to pass arguments indirectly was reverted in 32ad8fc799f90f5f15b2f2be29b3e328deb1e7dc
// XFAIL: *


struct big {
  char data[DATA_SIZE]; // FIXME: even with 2048 we get 256 i64 inreg arguments, is that correct? N64 starts using memcpy at 72
};

#if 0
// FIXME: this expands to a lot of code for DATA_SIZE=1024, etc...
int do_stuff(struct big arg) {
  int sum = 0;
  for (int i = 0; i < sizeof(arg.data); i++)
    sum += arg.data[i];
  return sum;
};
#endif

extern void do_stuff_extern(struct big arg);
extern struct big global_struct;

void foo(void) {
  struct big b;
  __builtin_memset(&b, 0xff, sizeof(b));
  do_stuff_extern(global_struct);
}

// MEMCPY-LABEL: define void @foo()
// MEMCPY:       call void @llvm.memcpy.p200i8.p200i8.i64
// MEMCPY-NEXT:  call void @do_stuff_extern(%struct.big addrspace(200)* byval align 8 {{%.+}})
// MEMCPY-LABEL: declare void @do_stuff_extern(%struct.big addrspace(200)* byval align 8)


// EXPANDED-LABEL: define void @foo()
// EXPANDED:       call void @do_stuff_extern(i64 inreg
// EXPANDED-LABEL: declare void @do_stuff_extern(i64 inreg, i64 inreg,


// N64-EXPANDED-LABEL: define void @foo()
// N64-EXPANDED: call void @do_stuff_extern(i64 inreg
// N64-EXPANDED-LABEL: declare void @do_stuff_extern(i64 inreg, i64 inreg,

// N64-MEMCPY-LABEL: define void @foo()
// N64-MEMCPY: call void @llvm.memcpy.p0i8.p0i8.i64
// N64-MEMCPY: call void @do_stuff_extern(%struct.big* byval align 8 {{%.+}})
// N64-MEMCPY-LABEL: declare void @do_stuff_extern(%struct.big* byval align 8)
