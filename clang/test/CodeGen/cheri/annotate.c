// RUN: %cheri_purecap_cc1 -o - -emit-llvm -O0 %s | FileCheck %s
// RUN: %cheri_cc1 -o - -emit-llvm -O0 %s | FileCheck %s -check-prefix HYBRID

// Ensure that the embedded source paths match across all build machines:
#line 6 "/some/dir/annotate.c"

void annotated_function(void) __attribute__((annotate("function"))) {}

int annotated_global __attribute__((annotate("global"))) = 1;

/// Check that globals annotations are emitted in AS200 for purecap:
// CHECK:      @.str = private unnamed_addr addrspace(200) constant [9 x i8] c"function\00", section "llvm.metadata"
// CHECK-NEXT: @.str.1 = private unnamed_addr addrspace(200) constant [[FILENAME_ARRAY:\[21 x i8\]]] c"/some/dir/annotate.c\00", section "llvm.metadata"
// CHECK-NEXT: @annotated_global = addrspace(200) global i32 1, align 4
// CHECK-NEXT: @.str.2 = private unnamed_addr addrspace(200) constant [7 x i8] c"global\00", section "llvm.metadata"
// CHECK-NEXT: @.str.3 = private unnamed_addr addrspace(200) constant [4 x i8] c"foo\00", section "llvm.metadata"
// CHECK-NEXT: @.str.4 = private unnamed_addr addrspace(200) constant [13 x i8] c"myannotation\00", section "llvm.metadata"
// CHECK-NEXT: @.str.5 = private unnamed_addr addrspace(200) constant [13 x i8] c"annotation_a\00", section "llvm.metadata"
// CHECK-NEXT: @llvm.global.annotations = appending addrspace(200) global [2 x { ptr addrspace(200), ptr addrspace(200), ptr addrspace(200), i32, ptr addrspace(200) }]
// CHECK-SAME: [{ ptr addrspace(200), ptr addrspace(200), ptr addrspace(200), i32, ptr addrspace(200) }
// CHECK-SAME: { ptr addrspace(200) @annotated_function, ptr addrspace(200) @.str, ptr addrspace(200) @.str.1
// CHECK-SAME: i32 [[ANNOTATED_FUNC_LINE:7]], ptr addrspace(200) null },
// CHECK-SAME: { ptr addrspace(200), ptr addrspace(200), ptr addrspace(200), i32, ptr addrspace(200) } { ptr addrspace(200) @annotated_global, ptr addrspace(200) @.str.2, ptr addrspace(200) @.str.1
// CHECK-SAME: i32 [[ANNOTATED_GLOBAL_LINE:9]], ptr addrspace(200) null }], section "llvm.metadata"

// CHECK-LABEL: define {{[^@]+}}@var_annotation
// CHECK-SAME: () addrspace(200)
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[B:%.*]] = alloca i32, align 4, addrspace(200)
// CHECK-NEXT:    call void @llvm.var.annotation.p200.p200(ptr addrspace(200) [[B]], ptr addrspace(200) @.str.3,
// CHECK-SAME:  ptr addrspace(200) @.str.1
// CHECK-SAME:  i32 [[@LINE+15]],
// CHECK-SAME:  ptr addrspace(200) null)
// CHECK-NEXT:    ret void
//
// HYBRID-LABEL: define {{[^@]+}}@var_annotation
// HYBRID-SAME: ()
// HYBRID-NEXT:  entry:
// HYBRID-NEXT:    [[B:%.*]] = alloca i32, align 4
// HYBRID-NEXT:    call void @llvm.var.annotation.p0.p0(ptr [[B]], ptr @.str.3,
// HYBRID-SAME:  ptr @.str.1
// HYBRID-SAME:  i32 [[@LINE+5]],
// HYBRID-SAME:  ptr null)
// HYBRID-NEXT:    ret void
//
void var_annotation(void) {
  __attribute__((annotate("foo"))) int b;
}

// Should be overloaded and args in AS200
// CHECK: declare void @llvm.var.annotation.p200.p200(ptr addrspace(200), ptr addrspace(200), ptr addrspace(200), i32, ptr addrspace(200)) addrspace(200)
// HYBRID: declare void @llvm.var.annotation.p0.p0(ptr, ptr, ptr, i32, ptr)

// CHECK-LABEL: define {{[^@]+}}@ptr_annotation
// CHECK-SAME: () addrspace(200)
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[VAR:%.*]] = alloca [[STRUCT_ANON:%.*]], align 4, addrspace(200)
// CHECK-NEXT:    [[U:%.*]] = getelementptr inbounds [[STRUCT_ANON]], ptr addrspace(200) [[VAR]], i32 0, i32 0
// CHECK-NEXT:    store i32 0, ptr addrspace(200) [[U]], align 4
// CHECK-NEXT:    [[V:%.*]] = getelementptr inbounds [[STRUCT_ANON]], ptr addrspace(200) [[VAR]], i32 0, i32 1
// CHECK-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.ptr.annotation.p200.p200(ptr addrspace(200) [[V]],
// CHECK-SAME:      ptr addrspace(200) @.str.4,
// CHECK-SAME:      ptr addrspace(200) @.str.1,
// CHECK-SAME:      i32 [[#@LINE+21]], ptr addrspace(200) null)
// CHECK-NEXT:    store i32 0, ptr addrspace(200) [[TMP1]], align 4
// CHECK-NEXT:    ret i32 0
//
// HYBRID-LABEL: define {{[^@]+}}@ptr_annotation
// HYBRID-SAME: ()
// HYBRID-NEXT:  entry:
// HYBRID-NEXT:    [[VAR:%.*]] = alloca [[STRUCT_ANON:%.*]], align 4
// HYBRID-NEXT:    [[U:%.*]] = getelementptr inbounds [[STRUCT_ANON]], ptr [[VAR]], i32 0, i32 0
// HYBRID-NEXT:    store i32 0, ptr [[U]], align 4
// HYBRID-NEXT:    [[V:%.*]] = getelementptr inbounds [[STRUCT_ANON]], ptr [[VAR]], i32 0, i32 1
// HYBRID-NEXT:    [[TMP1:%.*]] = call ptr @llvm.ptr.annotation.p0.p0(ptr [[V]],
// HYBRID-SAME:      ptr @.str.4,
// HYBRID-SAME:      ptr @.str.1,
// HYBRID-SAME:      i32 [[@LINE+7]], ptr null)
// HYBRID-NEXT:    store i32 0, ptr [[TMP1]], align 4
// HYBRID-NEXT:    ret i32 0
//
int ptr_annotation(void) {
  struct {
    int u;
    __attribute__((annotate("myannotation"))) int v;
  } var;
  var.u = 0;
  var.v = 0;
  return (0);
}

// Should be overloaded and args in AS200
// CHECK: declare ptr addrspace(200) @llvm.ptr.annotation.p200.p200(ptr addrspace(200), ptr addrspace(200), ptr addrspace(200), i32, ptr addrspace(200)) addrspace(200)
// HYBRID: declare ptr @llvm.ptr.annotation.p0.p0(ptr, ptr, ptr, i32, ptr)

// CHECK-LABEL: define {{[^@]+}}@builtin_annotation
// CHECK-SAME: (i64 noundef signext [[X:%.*]]) addrspace(200)
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[X_ADDR:%.*]] = alloca i64, align 8, addrspace(200)
// CHECK-NEXT:    [[Y:%.*]] = alloca i64, align 8, addrspace(200)
// CHECK-NEXT:    store i64 [[X:%.*]], ptr addrspace(200) [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP0:%.*]] = load i64, ptr addrspace(200) [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP1:%.*]] = call i64 @llvm.annotation.i64.p200(i64 [[TMP0]], ptr addrspace(200) @.str.5,
// CHECK-SAME:      ptr addrspace(200) @.str.1,
// CHECK-SAME:      i32 [[@LINE+26]])
// CHECK-NEXT:    store i64 [[TMP1]], ptr addrspace(200) [[Y]], align 8
// CHECK-NEXT:    [[TMP2:%.*]] = load i64, ptr addrspace(200) [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP3:%.*]] = load i64, ptr addrspace(200) [[Y]], align 8
// CHECK-NEXT:    [[ADD:%.*]] = add nsw i64 [[TMP2]], [[TMP3]]
// CHECK-NEXT:    [[CONV:%.*]] = trunc i64 [[ADD]] to i32
// CHECK-NEXT:    ret i32 [[CONV]]
//
// HYBRID-LABEL: define {{[^@]+}}@builtin_annotation
// HYBRID-SAME: (i64 noundef signext [[X:%.*]])
// HYBRID-NEXT:  entry:
// HYBRID-NEXT:    [[X_ADDR:%.*]] = alloca i64, align 8
// HYBRID-NEXT:    [[Y:%.*]] = alloca i64, align 8
// HYBRID-NEXT:    store i64 [[X]], ptr [[X_ADDR]], align 8
// HYBRID-NEXT:    [[TMP0:%.*]] = load i64, ptr [[X_ADDR]], align 8
// HYBRID-NEXT:    [[TMP1:%.*]] = call i64 @llvm.annotation.i64.p0(i64 [[TMP0]], ptr @.str.5,
// HYBRID-SAME:      ptr @.str.1,
// HYBRID-SAME:      i32 [[@LINE+9]])
// HYBRID-NEXT:    store i64 [[TMP1]], ptr [[Y]], align 8
// HYBRID-NEXT:    [[TMP2:%.*]] = load i64, ptr [[X_ADDR]], align 8
// HYBRID-NEXT:    [[TMP3:%.*]] = load i64, ptr [[Y]], align 8
// HYBRID-NEXT:    [[ADD:%.*]] = add nsw i64 [[TMP2]], [[TMP3]]
// HYBRID-NEXT:    [[CONV:%.*]] = trunc i64 [[ADD]] to i32
// HYBRID-NEXT:    ret i32 [[CONV]]
//
int builtin_annotation(long x) {
  long y = __builtin_annotation(x, "annotation_a");
  return x + y;
}

// Should be overloaded and args in AS200
// CHECK: declare i64 @llvm.annotation.i64.p200(i64, ptr addrspace(200), ptr addrspace(200), i32) addrspace(200)
// HYBRID: declare i64 @llvm.annotation.i64.p0(i64, ptr, ptr, i32)

// https://github.com/CTSRD-CHERI/llvm-project/issues/327
void issue327(void) {
  struct {
    __attribute__((annotate("myannotation"))) int u;
  } var, *__capability x;
  x = &var;
  x->u = x->u;
}

// CHECK: define {{[^@]+}}@issue327() addrspace(200)
// CHECK: call ptr addrspace(200) @llvm.ptr.annotation.p200.p200(ptr addrspace(200) %{{.+}}, ptr addrspace(200) @.str.4,
// CHECK-SAME: ptr addrspace(200) @.str.1
// CHECK-SAME: i32 [[#MYANNOTATION_LINE:142]],
// CHECK-SAME: ptr addrspace(200) null)
// CHECK: call ptr addrspace(200) @llvm.ptr.annotation.p200.p200(ptr addrspace(200) %{{.+}}, ptr addrspace(200) @.str.4,
// CHECK-SAME: ptr addrspace(200) @.str.1
// CHECK-SAME: i32 [[#MYANNOTATION_LINE]],
// CHECK-SAME: ptr addrspace(200) null)

// HYBRID: define {{[^@]+}} @issue327()
// HYBRID: call ptr addrspace(200) @llvm.ptr.annotation.p200.p0(ptr addrspace(200) %{{.+}}, ptr @.str.4,
// HYBRID-SAME: ptr @.str.1,
// HYBRID-SAME: i32 [[MYANNOTATION_LINE:142]],
// HYBRID-SAME: ptr null)
// HYBRID: call ptr addrspace(200) @llvm.ptr.annotation.p200.p0(ptr addrspace(200) %{{.+}}, ptr @.str.4,
// HYBRID-SAME: ptr @.str.1,
// HYBRID-SAME: i32 [[MYANNOTATION_LINE]],
// HYBRID-SAME: ptr null)
