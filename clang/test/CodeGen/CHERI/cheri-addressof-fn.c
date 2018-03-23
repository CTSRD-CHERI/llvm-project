// RUN: %cheri_cc1 %s "-target-abi" "purecap" -emit-llvm -cheri-linker  -o -
typedef void (*foo)(void);
void fn(void);

foo f = fn;
foo g = &fn;
// CHECK: __cxx_global_var_init
// CHECK: llvm.mips.pcc.get
// CHECK: store
// CHECK: @f, align 32
// CHECK: __cxx_global_var_init
// CHECK: llvm.mips.pcc.get
// CHECK: store
// CHECK: @g, align 32

