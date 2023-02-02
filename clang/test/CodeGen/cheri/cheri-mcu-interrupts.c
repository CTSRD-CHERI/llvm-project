// RUN: %clang_cc1 %s -o - "-triple" "riscv32-unknown-unknown" "-emit-llvm" "-mframe-pointer=none" "-mcmodel=small" "-target-cpu" "cheriot" "-target-feature" "+xcheri" "-target-feature" "-64bit" "-target-feature" "-relax" "-target-feature" "-xcheri-rvc" "-target-feature" "-save-restore" "-target-abi" "cheriot" "-Oz" "-Werror" "-cheri-compartment=example" | FileCheck %s
int foo(void);
// CHECK: define dso_local i32 @disabled() local_unnamed_addr addrspace(200) #[[DIS:[0-9]]]
__attribute__((cheri_interrupt_state(disabled)))
int disabled(void)
{
	return foo();
}

// CHECK: define dso_local i32 @enabled() local_unnamed_addr addrspace(200) #[[EN:[0-9]]]
__attribute__((cheri_interrupt_state(enabled)))
int enabled(void)
{
	return foo();
}

// CHECK: define dso_local i32 @inherit() local_unnamed_addr addrspace(200) #[[INH:[0-9]]]
__attribute__((cheri_interrupt_state(inherit)))
int inherit(void)
{
	return foo();
}

// The default for exported functions should be interrupts enabled
//
// CHECK: define dso_local chericcallcce void @_Z21default_enable_calleev() local_unnamed_addr addrspace(200) #[[DEFEN:[0-9]]]
__attribute__((cheri_compartment("example")))
void default_enable_callee(void)
{
}

// CHECK: define dso_local chericcallcc void @default_enable_callback() local_unnamed_addr addrspace(200) #[[DEFEN]]
__attribute__((cheri_ccallback))
void default_enable_callback(void)
{
}

// Explicitly setting interrupt status should override the default

// CHECK: define dso_local chericcallcce void @_Z23explicit_disable_calleev() local_unnamed_addr addrspace(200) #[[EXPDIS:[0-9]]]
__attribute__((cheri_interrupt_state(disabled)))
__attribute__((cheri_compartment("example")))
void explicit_disable_callee(void)
{
}

// CHECK: define dso_local chericcallcc void @explicit_disable_callback() local_unnamed_addr addrspace(200) #[[EXPDIS]]
__attribute__((cheri_interrupt_state(disabled)))
__attribute__((cheri_ccallback))
void explicit_disable_callback(void)
{
}


// CHECK: attributes #[[DIS]]
// CHECK-SAME: "interrupt-state"="disabled"
// CHECK: attributes #[[EN]]
// CHECK-SAME: "interrupt-state"="enabled"
// CHECK: attributes #[[INH]]
// CHECK-SAME: "interrupt-state"="inherit"
// CHECK: attributes #[[DEFEN]]
// CHECK-SAME: "interrupt-state"="enabled"
// CHECK: attributes #[[EXPDIS]]
// CHECK-SAME: "interrupt-state"="disabled"
