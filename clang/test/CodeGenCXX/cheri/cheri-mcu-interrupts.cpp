// RUN: %clang_cc1 %s -o - "-triple" "riscv32-unknown-unknown" "-emit-llvm" "-mframe-pointer=none" "-mcmodel=small" "-target-cpu" "cheriot" "-target-feature" "+xcheri" "-target-feature" "-64bit" "-target-feature" "-relax" "-target-feature" "-xcheri-rvc" "-target-feature" "-save-restore" "-target-abi" "cheriot" "-Oz" "-Werror" "-cheri-compartment=example" | FileCheck %s
int foo(void);
// CHECK: define dso_local i32 @_Z8disabledv() local_unnamed_addr addrspace(200) #[[DIS:[0-9]]]
[[cheri::interrupt_state(disabled)]]
int disabled(void)
{
	return foo();
}

// CHECK: define dso_local i32 @_Z7enabledv() local_unnamed_addr addrspace(200) #[[EN:[0-9]]]
[[cheri::interrupt_state(enabled)]]
int enabled(void)
{
	return foo();
}

// CHECK: define dso_local i32 @_Z7inheritv() local_unnamed_addr addrspace(200) #[[INH:[0-9]]]
[[cheri::interrupt_state(inherit)]]
int inherit(void)
{
	return foo();
}

// CHECK: attributes #[[DIS]]
// CHECK-SAME: "interrupt-state"="disabled"
// CHECK: attributes #[[EN]]
// CHECK-SAME: "interrupt-state"="enabled"
// CHECK: attributes #[[INH]]
// CHECK-SAME: "interrupt-state"="inherit"
