// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy %s -fno-rtti -std=c++11 -o - -emit-llvm | %cheri_FileCheck %s "-implicit-check-not=alloca { i64, i64 }" -check-prefixes CHECK,LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel %s -fno-rtti -std=c++11 -o - -emit-llvm | %cheri_FileCheck %s "-implicit-check-not=alloca { i64, i64 }" -check-prefixes CHECK,CAPTABLE
// RUN: %cheri_cc1 %s -target-abi n64 -fno-rtti -std=c++11 -o - -emit-llvm -O2 | %cheri_FileCheck %s -check-prefix N64
// RUN: %cheri_cc1 %s -target-abi n64 -fno-rtti -std=c++11 -o - -S -O2 | %cheri_FileCheck %s -check-prefix N64-ASM

class A {
public:
  int x = 3;
  int y = 4;
  int foo() { return 1; }
  virtual int foo_virtual() { return 2; }
  int bar() { return 1; }
  virtual int bar_virtual() { return 2; }
};

// compare IR with simulated function pointers:
struct mem_fn_ptr {
  void* ptr;
  long offset;
};
void func(void) {}
mem_fn_ptr virt = { (void*)32, 1 };
mem_fn_ptr nonvirt = { (void*)&func, 1 };
// CHECK: @virt = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 32 to i8 addrspace(200)*), i64 1 }, align [[#CAP_SIZE]]
// CHECK: @nonvirt = addrspace(200) global { i8 addrspace(200)*, i64 } {
// LEGACY-SAME: i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @_Z4funcv to i8*) to i8 addrspace(200)*), i64 1 }, align [[#CAP_SIZE]]
// CAPTABLE-SAME: i8 addrspace(200)* bitcast (void () addrspace(200)* @_Z4funcv to i8 addrspace(200)*), i64 1 }, align [[#CAP_SIZE]]

// now the real thing
typedef int (A::* AMemberFuncPtr)();

AMemberFuncPtr global_null_func_ptr = nullptr;
int A::* global_data_ptr = &A::y;
AMemberFuncPtr global_nonvirt_func_ptr = &A::bar;
AMemberFuncPtr global_virt_func_ptr = &A::bar_virtual;
// CHECK: @global_null_func_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } zeroinitializer, align [[#CAP_SIZE]]
// Offet is 20 for 128 and 36 for 256:
// CHECK: @global_data_ptr = addrspace(200) global i64 [[$A_Y_OFFSET:20|36]], align 8
// CHECK: @global_nonvirt_func_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } {
// LEGACY-SAME: i8 addrspace(200)* addrspacecast (i8* bitcast (i32 (%class.A addrspace(200)*)* @_ZN1A3barEv to i8*) to i8 addrspace(200)*), i64 0 }, align [[#CAP_SIZE]]
// CAPTABLE-SAME: i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A3barEv to i8 addrspace(200)*), i64 0 }, align [[#CAP_SIZE]]
// CHECK: @global_virt_func_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 [[#CAP_SIZE]] to i8 addrspace(200)*), i64 1 }, align [[#CAP_SIZE]]


int main() {
  // CHECK: [[NULL_DATA_PTR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: [[DATA_PTR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: [[DATA_PTR_2:%.+]] = alloca i64, align 8
  // CHECK-NEXT: [[NULL_FUNC_PTR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[FUNC_PTR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[MEMPTR_TMP:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[FUNC_PTR_2:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[MEMPTR_TMP1:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VIRTUAL_FUNC_PTR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VIRTUAL_FUNC_PTR_2:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  A a;
  // FIXME: alignment is wrong
  int A::* null_data_ptr = nullptr;
  // CHECK: store i64 -1, i64 addrspace(200)* [[NULL_DATA_PTR]], align 8
  int A::* data_ptr = &A::x;
  // CHECK: store i64 [[$A_X_OFFSET:16|32]], i64 addrspace(200)* [[DATA_PTR]], align 8
  int A::* data_ptr_2 = &A::y;
  // CHECK: store i64 [[$A_Y_OFFSET]], i64 addrspace(200)* [[DATA_PTR_2]], align 8

  AMemberFuncPtr null_func_ptr = nullptr;
  // CHECK:   store { i8 addrspace(200)*, i64 } zeroinitializer, { i8 addrspace(200)*, i64 } addrspace(200)* [[NULL_FUNC_PTR]], align [[#CAP_SIZE]]

  AMemberFuncPtr func_ptr = &A::foo;
  // This IR is pretty horrible, maybe we can create something nicer
  // LEGACY: [[PCC:%.*]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // LEGACY: [[NONVIRT_PTR:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* [[PCC]], i64 ptrtoint (i32 (%class.A addrspace(200)*)* @_ZN1A3fooEv to i64))
  // CHECK: [[TMP:%.*]] = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[MEMPTR_TMP]], i32 0, i32 0
  // LEGACY: store i8 addrspace(200)* [[NONVIRT_PTR]], i8 addrspace(200)* addrspace(200)* [[TMP]], align [[#CAP_SIZE]]
  // CAPTABLE: store i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A3fooEv to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* [[TMP]], align [[#CAP_SIZE]]
  // CHECK: [[TMP:%.*]] = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[MEMPTR_TMP]], i32 0, i32 1
  // CHECK: store i64 0, i64 addrspace(200)* [[TMP]], align [[#CAP_SIZE]]
  // CHECK: [[TMP:%.*]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[MEMPTR_TMP]], align [[#CAP_SIZE]]
  // CHECK: store { i8 addrspace(200)*, i64 } [[TMP]], { i8 addrspace(200)*, i64 } addrspace(200)* [[FUNC_PTR]], align [[#CAP_SIZE]]

  AMemberFuncPtr func_ptr_2 = &A::bar;
  // LEGACY: [[PCC:%.*]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // LEGACY: [[NONVIRT_PTR:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* [[PCC]], i64 ptrtoint (i32 (%class.A addrspace(200)*)* @_ZN1A3barEv to i64))
  // CHECK: [[TMP:%.*]] = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[MEMPTR_TMP1]], i32 0, i32 0
  // LEGACY: store i8 addrspace(200)* [[NONVIRT_PTR]], i8 addrspace(200)* addrspace(200)* [[TMP]], align [[#CAP_SIZE]]
  // CAPTABLE: store i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A3barEv to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* [[TMP]], align [[#CAP_SIZE]]
  // CHECK: [[TMP:%.*]] = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[MEMPTR_TMP1]], i32 0, i32 1
  // CHECK: store i64 0, i64 addrspace(200)* [[TMP]], align [[#CAP_SIZE]]
  // CHECK: [[TMP:%.*]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[MEMPTR_TMP1]], align [[#CAP_SIZE]]
  // CHECK: store { i8 addrspace(200)*, i64 } [[TMP]], { i8 addrspace(200)*, i64 } addrspace(200)* [[FUNC_PTR_2]], align [[#CAP_SIZE]]


  AMemberFuncPtr virtual_func_ptr = &A::foo_virtual;
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* null, i64 1 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[VIRTUAL_FUNC_PTR]], align [[#CAP_SIZE]]
   AMemberFuncPtr virtual_func_ptr_2 = &A::bar_virtual;
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 [[#CAP_SIZE]] to i8 addrspace(200)*), i64 1 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[VIRTUAL_FUNC_PTR_2]], align [[#CAP_SIZE]]

  // return a.*data_ptr + (a.*func_ptr)() + (a.*virtual_func_ptr)();
  // return null_func_ptr == nullptr;
  return a.*data_ptr;
}

bool data_ptr_is_nonnull(int A::* ptr) {
  return static_cast<bool>(ptr);
  // CHECK-LABEL: define zeroext i1 @_Z19data_ptr_is_nonnullM1Ai(i64{{.*}})
  // CHECK: [[PTR_ADDR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: store i64 [[PTR:%.+]], i64 addrspace(200)* [[PTR_ADDR]], align 8
  // CHECK-NEXT: [[VAR0:%.+]] = load i64, i64 addrspace(200)* [[PTR_ADDR]], align 8
  // CHECK-NEXT: [[MEMPTR_TOBOOL:%.+]] = icmp ne i64 [[VAR0]], -1
  // CHECK-NEXT: ret i1 [[MEMPTR_TOBOOL]]
}

bool data_ptr_is_null(int A::* ptr) {
  // CHECK-LABEL: define zeroext i1 @_Z16data_ptr_is_nullM1Ai(i64{{.*}})
  // CHECK: [[PTR_ADDR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: store i64 [[PTR:%.+]], i64 addrspace(200)* [[PTR_ADDR]], align 8
  // CHECK-NEXT: [[VAR0:%.+]] = load i64, i64 addrspace(200)* [[PTR_ADDR]], align 8
  // CHECK-NEXT: [[MEMPTR_TOBOOL:%.+]] = icmp ne i64 [[VAR0]], -1
  // CHECK-NEXT: [[LNOT:%.+]] = xor i1 [[MEMPTR_TOBOOL]], true
  // CHECK-NEXT: ret i1 [[LNOT]]
  return !ptr;
}

bool data_ptr_equal(int A::* ptr1, int A::* ptr2) {
  return ptr1 == ptr2;
  // CHECK-LABEL: define zeroext i1 @_Z14data_ptr_equalM1AiS0_(i64{{.*}}, i64{{.*}})
  // CHECK: [[PTR1_ADDR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: [[PTR2_ADDR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: store i64 [[PTR1:%.+]], i64 addrspace(200)* [[PTR1_ADDR]], align 8
  // CHECK-NEXT: store i64 [[PTR2:%.+]], i64 addrspace(200)* [[PTR2_ADDR]], align 8
  // CHECK-NEXT: [[VAR0:%.+]] = load i64, i64 addrspace(200)* [[PTR1_ADDR]], align 8
  // CHECK-NEXT: [[VAR1:%.+]] = load i64, i64 addrspace(200)* [[PTR2_ADDR]], align 8
  // CHECK-NEXT: [[VAR2:%.+]] = icmp eq i64 [[VAR0]], [[VAR1]]
  // CHECK-NEXT: ret i1 [[VAR2]]
}

bool data_ptr_not_equal(int A::* ptr1, int A::* ptr2) {
  return ptr1 != ptr2;
  // CHECK-LABEL: define zeroext i1 @_Z18data_ptr_not_equalM1AiS0_(i64{{.*}}, i64{{.*}})
  // CHECK: [[PTR1_ADDR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: [[PTR2_ADDR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: store i64 [[PTR1:%.+]], i64 addrspace(200)* [[PTR1_ADDR]], align 8
  // CHECK-NEXT: store i64 [[PTR2:%.+]], i64 addrspace(200)* [[PTR2_ADDR]], align 8
  // CHECK-NEXT: [[VAR0:%.+]] = load i64, i64 addrspace(200)* [[PTR1_ADDR]], align 8
  // CHECK-NEXT: [[VAR1:%.+]] = load i64, i64 addrspace(200)* [[PTR2_ADDR]], align 8
  // CHECK-NEXT: [[VAR2:%.+]] = icmp ne i64 [[VAR0]], [[VAR1]]
  // CHECK-NEXT: ret i1 [[VAR2]]
}

int data_ptr_dereferece(A* a, int A::* ptr) {
  return a->*ptr;
  // CHECK-LABEL: define signext i32 @_Z19data_ptr_derefereceU3capP1AMS_i(%class.A addrspace(200)*{{.*}}, i64{{.*}})
  // CHECK: [[A_ADDR:%.+]] = alloca %class.A addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR_ADDR:%.+]] = alloca i64, align 8
  // CHECK-NEXT: store %class.A addrspace(200)* [[A:%.+]], %class.A addrspace(200)* addrspace(200)* [[A_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: store i64 [[PTR:%.+]], i64 addrspace(200)* [[PTR_ADDR]], align 8
  // CHECK-NEXT: [[VAR0:%.+]] = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* [[A_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR1:%.+]] = load i64, i64 addrspace(200)* [[PTR_ADDR]], align 8
  // CHECK-NEXT: [[VAR2:%.+]] = bitcast %class.A addrspace(200)* [[VAR0]] to i8 addrspace(200)*
  // CHECK-NEXT: [[MEMPTR_OFFSET:%.+]] = getelementptr inbounds i8, i8 addrspace(200)* [[VAR2]], i64 [[VAR1]]
  // CHECK-NEXT: [[VAR3:%.+]] = bitcast i8 addrspace(200)* [[MEMPTR_OFFSET]] to i32 addrspace(200)*
  // CHECK-NEXT: [[VAR4:%.+]] = load i32, i32 addrspace(200)* [[VAR3]], align 4
  // CHECK-NEXT: ret i32 [[VAR4]]
}

// TODO: this could be simplified to test the tag bit of the address instead
// of checking the low bit of the adjustment

bool func_ptr_is_nonnull(AMemberFuncPtr ptr) {
  return static_cast<bool>(ptr);
  // CHECK-LABEL: zeroext i1 @_Z19func_ptr_is_nonnullM1AFivE(i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}})
  // CHECK: [[PTR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]

  // CHECK: [[VAR2:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[MEMPTR_PTR:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR2]], 0
  // CHECK-NEXT: [[MEMPTR_TOBOOL:%.+]] = icmp ne i8 addrspace(200)* [[MEMPTR_PTR]], null
  // CHECK-NEXT: [[MEMPTR_ADJ:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR2]], 1
  // CHECK-NEXT: [[MEMPTR_VIRTUALBIT:%.+]] = and i64 [[MEMPTR_ADJ]], 1
  // CHECK-NEXT: [[MEMPTR_ISVIRTUAL:%.+]] = icmp ne i64 [[MEMPTR_VIRTUALBIT]], 0
  // CHECK-NEXT: [[MEMPTR_ISNONNULL:%.+]] = or i1 [[MEMPTR_TOBOOL]], [[MEMPTR_ISVIRTUAL]]
  // CHECK-NEXT: ret i1 [[MEMPTR_ISNONNULL]]
}

bool func_ptr_is_null(AMemberFuncPtr ptr) {
  return !ptr;
  // CHECK-LABEL: define zeroext i1 @_Z16func_ptr_is_nullM1AFivE(i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}})
  // CHECK: [[PTR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]

  // CHECK: [[VAR2:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[MEMPTR_PTR:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR2]], 0
  // CHECK-NEXT: [[MEMPTR_TOBOOL:%.+]] = icmp ne i8 addrspace(200)* [[MEMPTR_PTR]], null
  // CHECK-NEXT: [[MEMPTR_ADJ:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR2]], 1
  // CHECK-NEXT: [[MEMPTR_VIRTUALBIT:%.+]] = and i64 [[MEMPTR_ADJ]], 1
  // CHECK-NEXT: [[MEMPTR_ISVIRTUAL:%.+]] = icmp ne i64 [[MEMPTR_VIRTUALBIT]], 0
  // CHECK-NEXT: [[MEMPTR_ISNONNULL:%.+]] = or i1 [[MEMPTR_TOBOOL]], [[MEMPTR_ISVIRTUAL]]
  // CHECK-NEXT: [[LNOT:%.+]] = xor i1 [[MEMPTR_ISNONNULL]], true
  // CHECK-NEXT: ret i1 [[LNOT]]
}

bool func_ptr_equal(AMemberFuncPtr ptr1, AMemberFuncPtr ptr2) {
  return ptr1 == ptr2;
  // CHECK-LABEL: define zeroext i1 @_Z14func_ptr_equalM1AFivES1_(i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}}, i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}})
  // CHECK: [[PTR1:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR2:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR1_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR2_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]

  // CHECK: [[VAR4:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR1_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR5:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR2_ADDR]], align [[#CAP_SIZE]]
  // CHECK: [[LHS_MEMPTR_PTR:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR4]], 0
  // CHECK: [[RHS_MEMPTR_PTR:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR5]], 0
  // CHECK: [[CMP_PTR:%.+]] = icmp eq i8 addrspace(200)* [[LHS_MEMPTR_PTR]], [[RHS_MEMPTR_PTR]]
  // CHECK: [[CMP_PTR_NULL:%.+]] = icmp eq i8 addrspace(200)* [[LHS_MEMPTR_PTR]], null
  // CHECK: [[LHS_MEMPTR_ADJ:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR4]], 1
  // CHECK: [[RHS_MEMPTR_ADJ:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR5]], 1
  // CHECK: [[CMP_ADJ:%.+]] = icmp eq i64 [[LHS_MEMPTR_ADJ]], [[RHS_MEMPTR_ADJ]]
  // CHECK: [[OR_ADJ:%.+]] = or i64 [[LHS_MEMPTR_ADJ]], [[RHS_MEMPTR_ADJ]]
  // CHECK: [[VAR6:%.+]] = and i64 [[OR_ADJ]], 1
  // CHECK: [[CMP_OR_ADJ:%.+]] = icmp eq i64 [[VAR6]], 0
  // CHECK: [[VAR7:%.+]] = and i1 [[CMP_PTR_NULL]], [[CMP_OR_ADJ]]
  // CHECK: [[VAR8:%.+]] = or i1 [[VAR7]], [[CMP_ADJ]]
  // CHECK: [[MEMPTR_EQ:%.+]] = and i1 [[CMP_PTR]], [[VAR8]]
  // CHECK: ret i1 [[MEMPTR_EQ]]
}

bool func_ptr_not_equal(AMemberFuncPtr ptr1, AMemberFuncPtr ptr2) {
  return ptr1 != ptr2;
  // CHECK-LABEL: define zeroext i1 @_Z18func_ptr_not_equalM1AFivES1_(i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}}, i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}})
  // CHECK: [[PTR1:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR2:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR1_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR2_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]

  // CHECK: [[VAR4:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR1_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR5:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR2_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[LHS_MEMPTR_PTR:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR4]], 0
  // CHECK-NEXT: [[RHS_MEMPTR_PTR:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR5]], 0
  // CHECK-NEXT: [[CMP_PTR:%.+]] = icmp ne i8 addrspace(200)* [[LHS_MEMPTR_PTR]], [[RHS_MEMPTR_PTR]]
  // CHECK-NEXT: [[CMP_PTR_NULL:%.+]] = icmp ne i8 addrspace(200)* [[LHS_MEMPTR_PTR]], null
  // CHECK-NEXT: [[LHS_MEMPTR_ADJ:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR4]], 1
  // CHECK-NEXT: [[RHS_MEMPTR_ADJ:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR5]], 1
  // CHECK-NEXT: [[CMP_ADJ:%.+]] = icmp ne i64 [[LHS_MEMPTR_ADJ]], [[RHS_MEMPTR_ADJ]]
  // CHECK-NEXT: [[OR_ADJ:%.+]] = or i64 [[LHS_MEMPTR_ADJ]], [[RHS_MEMPTR_ADJ]]
  // CHECK-NEXT: [[VAR6:%.+]] = and i64 [[OR_ADJ]], 1
  // CHECK-NEXT: [[CMP_OR_ADJ:%.+]] = icmp ne i64 [[VAR6]], 0
  // CHECK-NEXT: [[VAR7:%.+]] = or i1 [[CMP_PTR_NULL]], [[CMP_OR_ADJ]]
  // CHECK-NEXT: [[VAR8:%.+]] = and i1 [[VAR7]], [[CMP_ADJ]]
  // CHECK-NEXT: [[MEMPTR_NE:%.+]] = or i1 [[CMP_PTR]], [[VAR8]]
  // CHECK-NEXT: ret i1 [[MEMPTR_NE]]
}

int func_ptr_dereference(A* a, AMemberFuncPtr ptr) {
  return (a->*ptr)();
  // CHECK-LABEL: define signext i32 @_Z20func_ptr_dereferenceU3capP1AMS_FivE(%class.A addrspace(200)*{{.*}}, i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}})
  // CHECK: [[PTR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[A_ADDR:%.+]] = alloca %class.A addrspace(200)*, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK: [[VAR2:%.+]] = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* [[A_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR3:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[MEMPTR_ADJ:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR3]], 1
  // CHECK-NEXT: [[MEMPTR_ADJ_SHIFTED:%.+]] = ashr i64 [[MEMPTR_ADJ]], 1
  // CHECK-NEXT: [[THIS_NOT_ADJUSTED:%.+]] = bitcast %class.A addrspace(200)* [[VAR2]] to i8 addrspace(200)*
  // CHECK-NEXT: [[MEMPTR_VTABLE_ADDR:%.+]] = getelementptr inbounds i8, i8 addrspace(200)* [[THIS_NOT_ADJUSTED]], i64 [[MEMPTR_ADJ_SHIFTED]]
  // CHECK-NEXT: [[THIS_ADJUSTED:%.+]] = bitcast i8 addrspace(200)* [[MEMPTR_VTABLE_ADDR]] to %class.A addrspace(200)*
  // CHECK-NEXT: [[MEMPTR_PTR:%.+]] = extractvalue { i8 addrspace(200)*, i64 } [[VAR3]], 0
  // CHECK-NEXT: [[VAR4:%.+]] = and i64 [[MEMPTR_ADJ]], 1
  // CHECK-NEXT: [[MEMPTR_ISVIRTUAL:%.+]] = icmp ne i64 [[VAR4]], 0
  // CHECK-NEXT: br i1 [[MEMPTR_ISVIRTUAL]], label %[[MEMPTR_VIRTUAL_LABEL:.+]], label %[[MEMPTR_NONVIRTUAL_LABEL:.+]]

  // CHECK: [[MEMPTR_VIRTUAL_LABEL]]:
  // CHECK-NEXT: [[VAR5:%.+]] = bitcast i8 addrspace(200)* [[MEMPTR_VTABLE_ADDR]] to i8 addrspace(200)* addrspace(200)*
  // CHECK-NEXT: [[VTABLE:%.+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* [[VAR5]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[MEMPTR_VTABLE_OFFSET:%.+]] = ptrtoint i8 addrspace(200)* [[MEMPTR_PTR]] to i64
  // CHECK-NEXT: [[VAR6:%.+]] = getelementptr i8, i8 addrspace(200)* [[VTABLE]], i64 [[MEMPTR_VTABLE_OFFSET]]
  // CHECK-NEXT: [[VAR7:%.+]] = bitcast i8 addrspace(200)* [[VAR6]] to i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)*
  // CHECK-NEXT: [[MEMPTR_VIRTUALFN:%.+]] = load i32 (%class.A addrspace(200)*) addrspace(200)*, i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)* [[VAR7]], align [[#CAP_SIZE]]
  // CHECK-NEXT: br label %[[MEMPTR_END_LABEL:.+]]

  // CHECK: [[MEMPTR_NONVIRTUAL_LABEL]]
  // CHECK-NEXT: [[MEMPTR_NONVIRTUALFN:%.+]] = bitcast i8 addrspace(200)* [[MEMPTR_PTR]] to i32 (%class.A addrspace(200)*) addrspace(200)*
  // CHECK-NEXT: br label %[[MEMPTR_END_LABEL]]

  // CHECK: [[MEMPTR_END_LABEL]]:                                       ; preds = %[[MEMPTR_NONVIRTUAL_LABEL]], %[[MEMPTR_VIRTUAL_LABEL]]
  // CHECK-NEXT: [[VAR8:%.+]] = phi i32 (%class.A addrspace(200)*) addrspace(200)* [ [[MEMPTR_VIRTUALFN]], %[[MEMPTR_VIRTUAL_LABEL]] ], [ [[MEMPTR_NONVIRTUALFN]], %[[MEMPTR_NONVIRTUAL_LABEL]] ]
  // CHECK-NEXT: [[CALL:%.+]] = call signext i32 [[VAR8]](%class.A addrspace(200)* [[THIS_ADJUSTED]])
  // CHECK-NEXT: ret i32 [[CALL]]
  // N64: [[MEMPTR_ADJ_SHIFTED:%.+]] = ashr i64 [[PTR_COERCE1:%.+]], 1
  // N64-NEXT: [[THIS_NOT_ADJUSTED:%.+]] = bitcast %class.A* [[A:%.+]] to i8*
  // N64-NEXT: [[MEMPTR_VTABLE_ADDR:%.+]] = getelementptr inbounds i8, i8* [[THIS_NOT_ADJUSTED]], i64 [[MEMPTR_ADJ_SHIFTED]]
  // N64-NEXT: [[THIS_ADJUSTED:%.+]] = bitcast i8* [[MEMPTR_VTABLE_ADDR]] to %class.A*
  // N64-NEXT: [[VAR0:%.+]] = and i64 [[PTR_COERCE1]], 1
  // N64-NEXT: [[MEMPTR_ISVIRTUAL:%.+]] = icmp eq i64 [[VAR0]], 0
  // N64-NEXT: br i1 [[MEMPTR_ISVIRTUAL]], label %[[MEMPTR_NONVIRTUAL_LABEL:.+]], label %[[MEMPTR_VIRTUAL_LABEL:.+]]

  // N64: [[MEMPTR_VIRTUAL_LABEL]]:
  // N64-NEXT: [[VAR1:%.+]] = bitcast i8* [[MEMPTR_VTABLE_ADDR]] to i8**
  // N64-NEXT: [[VTABLE:%.+]] = load i8*, i8** [[VAR1]], align 8,
  // N64-NEXT: [[VAR2:%.+]] = getelementptr i8, i8* [[VTABLE]], i64 [[PTR_COERCE0:%.+]]
  // N64-NEXT: [[VAR3:%.+]] = bitcast i8* [[VAR2]] to i32 (%class.A*)**
  // N64-NEXT: [[MEMPTR_VIRTUALFN:%.+]] = load i32 (%class.A*)*, i32 (%class.A*)** [[VAR3]], align 8
  // N64-NEXT: br label %[[MEMPTR_END_LABEL:.+]]

  // N64: [[MEMPTR_NONVIRTUAL_LABEL]]:
  // N64-NEXT: [[MEMPTR_NONVIRTUALFN:%.+]] = inttoptr i64 [[PTR_COERCE0]] to i32 (%class.A*)*
  // N64-NEXT: br label %[[MEMPTR_END_LABEL]]

  // N64: [[MEMPTR_END_LABEL]]:
  // N64-NEXT: [[VAR4:%.+]] = phi i32 (%class.A*)* [ [[MEMPTR_VIRTUALFN]], %[[MEMPTR_VIRTUAL_LABEL]] ], [ [[MEMPTR_NONVIRTUALFN]], %[[MEMPTR_NONVIRTUAL_LABEL]] ]
  // N64-NEXT: [[CALL:%.+]] = tail call signext i32 [[VAR4]](%class.A* [[THIS_ADJUSTED]])
  // N64-NEXT: ret i32 [[CALL]]

  // N64 ASM on entry: $4 = A* a, $5 = memptr.ptr, $6 = memptr.adj
  // shift right by 1 to load adj from memptr.adj into $1
  // N64-ASM: dsra    [[ADJ:\$1]], $6, 1
  // $2 = isvirtual
  // N64-ASM: andi    $2, $6, 1
  // N64-ASM: beqz    $2, .LBB12_2
  // branch delay: add adj to $4 to get this.adjusted
  // N64-ASM: daddu   [[THIS_ADJUSTED:\$4]], $4, [[ADJ]]
  // load vtable into $1:
  // N64-ASM: ld      [[VTABLE:\$1]], 0([[THIS_ADJUSTED]])
  // add memptr.ptr (cast to int) to the vtable to get the index:
  // N64-ASM: daddu   [[VTABLE]], [[VTABLE]], $5
  // load the function pointer into $5 (which would already contain it in the non-virtual case)
  // N64-ASM: ld      $5, 0([[VTABLE]])
  // N64-ASM: move    $25, $5
  // N64-ASM: jalr    $25

}

// Check using Member pointers as return values an parameters
AMemberFuncPtr return_func_ptr() {
  // CHECK-LABEL: define void @_Z15return_func_ptrv({ i8 addrspace(200)*, i64 } addrspace(200)* noalias sret %agg.result)
  // CHECK: store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 [[#CAP_SIZE]] to i8 addrspace(200)*), i64 1 }, { i8 addrspace(200)*, i64 } addrspace(200)* %agg.result, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR0:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %agg.result, align [[#CAP_SIZE]]
  // CHECK-NEXT: store { i8 addrspace(200)*, i64 } [[VAR0]], { i8 addrspace(200)*, i64 } addrspace(200)* %agg.result, align [[#CAP_SIZE]]
  // CHECK-NEXT: ret void
  return &A::bar_virtual;
}

void take_func_ptr(AMemberFuncPtr ptr) {
  // CHECK-LABEL: define void @_Z13take_func_ptrM1AFivE(i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}})
  // CHECK: ret void
}

AMemberFuncPtr passthrough_func_ptr(AMemberFuncPtr ptr) {
  // CHECK-LABEL: define void @_Z20passthrough_func_ptrM1AFivE({ i8 addrspace(200)*, i64 } addrspace(200)* noalias sret %agg.result, {{.*}}, i8 addrspace(200)* inreg{{.*}}, i64 inreg{{.*}})
  // CHECK: [[PTR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR_ADDR:%.+]] = alloca { i8 addrspace(200)*, i64 }, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR1:%.+]] = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR]], i32 0, i32 0
  // CHECK-NEXT: store i8 addrspace(200)* [[PTR_COERCE0:%.+]], i8 addrspace(200)* addrspace(200)* [[VAR1]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR2:%.+]] = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR]], i32 0, i32 1
  // CHECK-NEXT: store i64 [[PTR_COERCE1:%.+]], i64 addrspace(200)* [[VAR2]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[PTR1:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: store { i8 addrspace(200)*, i64 } [[PTR1]], { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR3:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* [[PTR_ADDR]], align [[#CAP_SIZE]]
  // CHECK-NEXT: store { i8 addrspace(200)*, i64 } [[VAR3]], { i8 addrspace(200)*, i64 } addrspace(200)* %agg.result, align [[#CAP_SIZE]]
  // CHECK-NEXT: [[VAR4:%.+]] = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %agg.result, align [[#CAP_SIZE]]
  // CHECK-NEXT: store { i8 addrspace(200)*, i64 } [[VAR4]], { i8 addrspace(200)*, i64 } addrspace(200)* %agg.result, align [[#CAP_SIZE]]
  // CHECK-NEXT: ret void
  return ptr;
}

// taken from temporaries.cpp
namespace PR7556 {
  struct A { ~A(); };
  struct B { int i; ~B(); };
  struct C { int C::*pm; ~C(); };
  // CHECK-LABEL: define void @_ZN6PR75563fooEv()
  void foo() {
    // CHECK: [[STRUCT_A:%.+]] = alloca %"struct.PR7556::A", align 1
    // CHECK-NEXT: [[STRUCT_B:%.+]] = alloca %"struct.PR7556::B", align 4
    // CHECK-NEXT: [[STRUCT_C:%.+]] = alloca %"struct.PR7556::C", align 8
    // CHECK-NEXT: call void @_ZN6PR75561AD1Ev(%"struct.PR7556::A" addrspace(200)* [[STRUCT_A]])
    A();

    // B() is initialized using memset:
    // CHECK-NEXT: [[B_AS_I8:%.+]] = bitcast %"struct.PR7556::B" addrspace(200)* [[STRUCT_B]] to i8 addrspace(200)*
    // CHECK-NEXT: call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 4 [[B_AS_I8]], i8 0, i64 4, i1 false)
    // CHECK-NEXT: call void @_ZN6PR75561BD1Ev(%"struct.PR7556::B" addrspace(200)* [[STRUCT_B]])
    B();

    // C can't be zero-initialized due to pointer to data member:
    // CHECK-NEXT: [[C_AS_I8:%.+]] = bitcast %"struct.PR7556::C" addrspace(200)* [[STRUCT_C]] to i8 addrspace(200)*
    // CHECK-NEXT: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 8 [[C_AS_I8]],
    // LEGACY-SAME: i8 addrspace(200)* align 8 addrspacecast (i8* bitcast (%"struct.PR7556::C"* @0 to i8*) to i8 addrspace(200)*), i64 8, i1 false)
    // CAPTABLE-SAME: i8 addrspace(200)* align 8 bitcast (%"struct.PR7556::C" addrspace(200)* @0 to i8 addrspace(200)*), i64 8, i1 false)
    // CHECK-NEXT: call void @_ZN6PR75561CD1Ev(%"struct.PR7556::C" addrspace(200)* [[STRUCT_C]])
    C();
    // CHECK: ret void
  }
}
