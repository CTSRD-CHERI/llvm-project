// RUN: %riscv32_cheri_purecap_cc1 -cheri-bounds=subobject-safe-exact -o - -emit-llvm %s | FileCheck %s --check-prefixes=CHECK-IL32PC64
// RUN: %riscv64_cheri_purecap_cc1 -cheri-bounds=subobject-safe-exact -o - -emit-llvm %s | FileCheck %s --check-prefixes=CHECK-L64PC128

// CHECK-IL32PC64-LABEL: %struct.NeedPadBase = type { i32, [508 x i8], [4096 x i8] }
// CHECK-L64PC128-LABEL: %struct.NeedPadBase = type { i32, [4 x i8], [4096 x i8] }
struct NeedPadBase {
  int field;
  char unrepresentable[0x1000] __attribute__((cheri_pad_representable));
} s_base;

// CHECK-IL32PC64-LABEL: %struct.NeedPadTop = type { [4097 x i8], [511 x i8], i32, [508 x i8] }
// CHECK-L64PC128-LABEL: %struct.NeedPadTop = type { [4097 x i8], [7 x i8], i32, [4 x i8] }
struct NeedPadTop {
  char unrepresentable[0x1001] __attribute__((cheri_pad_representable));
  int field;
} s_top;

// CHECK-IL32PC64-LABEL: %struct.NeedPadBoth = type { i32, [508 x i8], [4097 x i8], [511 x i8], i32, [508 x i8] }
// CHECK-L64PC128-LABEL: %struct.NeedPadBoth = type { i32, [4 x i8], [4097 x i8], [7 x i8], i32, [4 x i8] }
struct NeedPadBoth {
  int pre;
  char unrepresentable[0x1001] __attribute__((cheri_pad_representable));
  int post;
} s_both;

// CHECK-IL32PC64-LABEL: @small_global = addrspace(200) global [1024 x i8] zeroinitializer, align 1
// CHECK-L64PC128-LABEL: @small_global = addrspace(200) global [1024 x i8] zeroinitializer, align 1
char small_global[1024];

// CHECK-IL32PC64-LABEL: @large_global = addrspace(200) global [4097 x i8] zeroinitializer, align 1
// CHECK-L64PC128-LABEL: @large_global = addrspace(200) global [4097 x i8] zeroinitializer, align 1
char large_global[0x1001];

char small_global_pad[1024] __attribute__((cheri_pad_representable));
char large_global_pad[0x1001] __attribute__((cheri_pad_representable));


// CHECK-IL32PC64-LABEL: @get_s_base
// CHECK-IL32PC64-NEXT:  entry:
// CHECK-IL32PC64-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i32(ptr addrspace(200) getelementptr inbounds ([[STRUCT_NEEDPADBASE:%.*]] @s_base, i32 0, i32 2), i32 4096)
//
// CHECK-L64PC128-LABEL: @get_s_base
// CHECK-L64PC128-NEXT:  entry:
// CHECK-L64PC128-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) getelementptr inbounds ([[STRUCT_NEEDPADBASE:%.*]] @s_base, i32 0, i32 2), i64 4096)
char *get_s_base() {
  return s_base.unrepresentable;
}

// CHECK-IL32PC64-LABEL: @get_s_top
// CHECK-IL32PC64-NEXT:  entry:
// CHECK-IL32PC64-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i32(ptr addrspace(200) @s_top, i32 4097)
//
// CHECK-L64PC128-LABEL: @get_s_top
// CHECK-L64PC128-NEXT:  entry:
// CHECK-L64PC128-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) @s_top, i64 4097)
char *get_s_top() {
  return s_top.unrepresentable;
}

// CHECK-IL32PC64-LABEL: @get_s_both
// CHECK-IL32PC64-NEXT:  entry:
// CHECK-IL32PC64-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i32(ptr addrspace(200) getelementptr inbounds ([[STRUCT_NEEDPADBOTH:%.*]] @s_both, i32 0, i32 2), i32 4097)
//
// CHECK-L64PC128-LABEL: @get_s_both
// CHECK-L64PC128-NEXT:  entry:
// CHECK-L64PC128-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) getelementptr inbounds ([[STRUCT_NEEDPADBOTH:%.*]] @s_both, i32 0, i32 2), i64 4097)
char *get_s_both() {
  return s_both.unrepresentable;
}
