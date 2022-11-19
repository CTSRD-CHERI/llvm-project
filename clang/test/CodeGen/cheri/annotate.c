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
// CHECK-NEXT: @llvm.global.annotations = appending addrspace(200) global [2 x { i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i32, i8 addrspace(200)* }]
// CHECK-SAME: [{ i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i32, i8 addrspace(200)* }
// CHECK-SAME: { i8 addrspace(200)* bitcast (void () addrspace(200)* @annotated_function to i8 addrspace(200)*), i8 addrspace(200)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(200)* @.str, i32 0, i32 0), i8 addrspace(200)* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME: i32 [[ANNOTATED_FUNC_LINE:7]], i8 addrspace(200)* null },
// CHECK-SAME: { i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i32, i8 addrspace(200)* } { i8 addrspace(200)* bitcast (i32 addrspace(200)* @annotated_global to i8 addrspace(200)*), i8 addrspace(200)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(200)* @.str.2, i32 0, i32 0), i8 addrspace(200)* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME: i32 [[ANNOTATED_GLOBAL_LINE:9]], i8 addrspace(200)* null }], section "llvm.metadata"

// CHECK-LABEL: define {{[^@]+}}@var_annotation
// CHECK-SAME: () addrspace(200) [[ATTR0:#.*]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[B:%.*]] = alloca i32, align 4, addrspace(200)
// CHECK-NEXT:    [[B1:%.*]] = bitcast i32 addrspace(200)* [[B]] to i8 addrspace(200)*
// CHECK-NEXT:    call void @llvm.var.annotation.p200i8(i8 addrspace(200)* [[B1]], i8 addrspace(200)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(200)* @.str.3, i32 0, i32 0),
// CHECK-SAME:  i8 addrspace(200)* getelementptr inbounds ([[FILENAME_ARRAY]],
// CHECK-SAME:  [[FILENAME_ARRAY]] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME:  i32 [[@LINE+17]],
// CHECK-SAME:  i8 addrspace(200)* null)
// CHECK-NEXT:    ret void
//
// HYBRID-LABEL: define {{[^@]+}}@var_annotation
// HYBRID-SAME: () [[ATTR0:#.*]] {
// HYBRID-NEXT:  entry:
// HYBRID-NEXT:    [[B:%.*]] = alloca i32, align 4
// HYBRID-NEXT:    [[B1:%.*]] = bitcast i32* [[B]] to i8*
// HYBRID-NEXT:    call void @llvm.var.annotation.p0i8(i8* [[B1]], i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0),
// HYBRID-SAME:  i8* getelementptr inbounds ([[FILENAME_ARRAY:\[[0-9]+ x i8\]]],
// HYBRID-SAME:  [[FILENAME_ARRAY]]* @.str.1, i32 0, i32 0),
// HYBRID-SAME:  i32 [[@LINE+5]],
// HYBRID-SAME:  i8* null)
// HYBRID-NEXT:    ret void
//
void var_annotation(void) {
  __attribute__((annotate("foo"))) int b;
}

// Should be overloaded and args in AS200
// CHECK: declare void @llvm.var.annotation.p200i8(i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i32, i8 addrspace(200)*) addrspace(200)
// HYBRID: declare void @llvm.var.annotation.p0i8(i8*, i8*, i8*, i32, i8*)

// CHECK-LABEL: define {{[^@]+}}@ptr_annotation
// CHECK-SAME: () addrspace(200) [[ATTR0]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[VAR:%.*]] = alloca [[STRUCT_ANON:%.*]], align 4, addrspace(200)
// CHECK-NEXT:    [[U:%.*]] = getelementptr inbounds [[STRUCT_ANON]], [[STRUCT_ANON]] addrspace(200)* [[VAR]], i32 0, i32 0
// CHECK-NEXT:    store i32 0, i32 addrspace(200)* [[U]], align 4
// CHECK-NEXT:    [[V:%.*]] = getelementptr inbounds [[STRUCT_ANON]], [[STRUCT_ANON]] addrspace(200)* [[VAR]], i32 0, i32 1
// CHECK-NEXT:    [[TMP0:%.*]] = bitcast i32 addrspace(200)* [[V]] to i8 addrspace(200)*
// CHECK-NEXT:    [[TMP1:%.*]] = call i8 addrspace(200)* @llvm.ptr.annotation.p200i8.p200i8(i8 addrspace(200)* [[TMP0]],
// CHECK-SAME:      i8 addrspace(200)* getelementptr inbounds ([13 x i8], [13 x i8] addrspace(200)* @.str.4, i32 0, i32 0),
// CHECK-SAME:      i8 addrspace(200)* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME:      i32 [[@LINE+24]], i8 addrspace(200)* null)
// CHECK-NEXT:    [[TMP2:%.*]] = bitcast i8 addrspace(200)* [[TMP1]] to i32 addrspace(200)*
// CHECK-NEXT:    store i32 0, i32 addrspace(200)* [[TMP2]], align 4
// CHECK-NEXT:    ret i32 0
//
// HYBRID-LABEL: define {{[^@]+}}@ptr_annotation
// HYBRID-SAME: () [[ATTR0]] {
// HYBRID-NEXT:  entry:
// HYBRID-NEXT:    [[VAR:%.*]] = alloca [[STRUCT_ANON:%.*]], align 4
// HYBRID-NEXT:    [[U:%.*]] = getelementptr inbounds [[STRUCT_ANON]], %struct.anon* [[VAR]], i32 0, i32 0
// HYBRID-NEXT:    store i32 0, i32* [[U]], align 4
// HYBRID-NEXT:    [[V:%.*]] = getelementptr inbounds [[STRUCT_ANON]], %struct.anon* [[VAR]], i32 0, i32 1
// HYBRID-NEXT:    [[TMP0:%.*]] = bitcast i32* [[V]] to i8*
// HYBRID-NEXT:    [[TMP1:%.*]] = call i8* @llvm.ptr.annotation.p0i8.p0i8(i8* [[TMP0]],
// HYBRID-SAME:      i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.4, i32 0, i32 0),
// HYBRID-SAME:      i8* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]]* @.str.1, i32 0, i32 0),
// HYBRID-SAME:      i32 [[@LINE+8]], i8* null)
// HYBRID-NEXT:    [[TMP2:%.*]] = bitcast i8* [[TMP1]] to i32*
// HYBRID-NEXT:    store i32 0, i32* [[TMP2]], align 4
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
// CHECK: declare i8 addrspace(200)* @llvm.ptr.annotation.p200i8.p200i8(i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i32, i8 addrspace(200)*) addrspace(200)
// HYBRID: declare i8* @llvm.ptr.annotation.p0i8.p0i8(i8*, i8*, i8*, i32, i8*)

// CHECK-LABEL: define {{[^@]+}}@builtin_annotation
// CHECK-SAME: (i64 signext [[X:%.*]]) addrspace(200) [[ATTR0]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[X_ADDR:%.*]] = alloca i64, align 8, addrspace(200)
// CHECK-NEXT:    [[Y:%.*]] = alloca i64, align 8, addrspace(200)
// CHECK-NEXT:    store i64 [[X:%.*]], i64 addrspace(200)* [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP0:%.*]] = load i64, i64 addrspace(200)* [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP1:%.*]] = call i64 @llvm.annotation.i64.p200i8(i64 [[TMP0]], i8 addrspace(200)* getelementptr inbounds ([13 x i8], [13 x i8] addrspace(200)* @.str.5, i32 0, i32 0),
// CHECK-SAME:      i8 addrspace(200)* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME:      i32 [[@LINE+26]])
// CHECK-NEXT:    store i64 [[TMP1]], i64 addrspace(200)* [[Y]], align 8
// CHECK-NEXT:    [[TMP2:%.*]] = load i64, i64 addrspace(200)* [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP3:%.*]] = load i64, i64 addrspace(200)* [[Y]], align 8
// CHECK-NEXT:    [[ADD:%.*]] = add nsw i64 [[TMP2]], [[TMP3]]
// CHECK-NEXT:    [[CONV:%.*]] = trunc i64 [[ADD]] to i32
// CHECK-NEXT:    ret i32 [[CONV]]
//
// HYBRID-LABEL: define {{[^@]+}}@builtin_annotation
// HYBRID-SAME: (i64 signext [[X:%.*]]) [[ATTR0]] {
// HYBRID-NEXT:  entry:
// HYBRID-NEXT:    [[X_ADDR:%.*]] = alloca i64, align 8
// HYBRID-NEXT:    [[Y:%.*]] = alloca i64, align 8
// HYBRID-NEXT:    store i64 [[X]], i64* [[X_ADDR]], align 8
// HYBRID-NEXT:    [[TMP0:%.*]] = load i64, i64* [[X_ADDR]], align 8
// HYBRID-NEXT:    [[TMP1:%.*]] = call i64 @llvm.annotation.i64.p0i8(i64 [[TMP0]], i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.5, i32 0, i32 0),
// HYBRID-SAME:      i8* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]]* @.str.1, i32 0, i32 0),
// HYBRID-SAME:      i32 [[@LINE+9]])
// HYBRID-NEXT:    store i64 [[TMP1]], i64* [[Y]], align 8
// HYBRID-NEXT:    [[TMP2:%.*]] = load i64, i64* [[X_ADDR]], align 8
// HYBRID-NEXT:    [[TMP3:%.*]] = load i64, i64* [[Y]], align 8
// HYBRID-NEXT:    [[ADD:%.*]] = add nsw i64 [[TMP2]], [[TMP3]]
// HYBRID-NEXT:    [[CONV:%.*]] = trunc i64 [[ADD]] to i32
// HYBRID-NEXT:    ret i32 [[CONV]]
//
int builtin_annotation(long x) {
  long y = __builtin_annotation(x, "annotation_a");
  return x + y;
}

// Should be overloaded and args in AS200
// CHECK: declare i64 @llvm.annotation.i64.p200i8(i64, i8 addrspace(200)*, i8 addrspace(200)*, i32) addrspace(200)
// HYBRID: declare i64 @llvm.annotation.i64.p0i8(i64, i8*, i8*, i32)

// https://github.com/CTSRD-CHERI/llvm-project/issues/327
void issue327(void) {
  struct {
    __attribute__((annotate("myannotation"))) int u;
  } var, *__capability x;
  x = &var;
  x->u = x->u;
}

// CHECK: define {{[^@]+}}@issue327() addrspace(200)
// CHECK: call i8 addrspace(200)* @llvm.ptr.annotation.p200i8.p200i8(i8 addrspace(200)* %{{.+}}, i8 addrspace(200)* getelementptr inbounds ([13 x i8], [13 x i8] addrspace(200)* @.str.4, i32 0, i32 0),
// CHECK-SAME: i8 addrspace(200)* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME: i32 [[MYANNOTATION_LINE:150]],
// CHECK-SAME: i8 addrspace(200)* null)
// CHECK: call i8 addrspace(200)* @llvm.ptr.annotation.p200i8.p200i8(i8 addrspace(200)* %{{.+}}, i8 addrspace(200)* getelementptr inbounds ([13 x i8], [13 x i8] addrspace(200)* @.str.4, i32 0, i32 0),
// CHECK-SAME: i8 addrspace(200)* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME: i32 [[MYANNOTATION_LINE]],
// CHECK-SAME: i8 addrspace(200)* null)

// HYBRID: define {{[^@]+}} @issue327()
// HYBRID: call i8 addrspace(200)* @llvm.ptr.annotation.p200i8.p0i8(i8 addrspace(200)* %{{.+}}, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.4, i32 0, i32 0),
// HYBRID-SAME: i8* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]]* @.str.1, i32 0, i32 0),
// HYBRID-SAME: i32 [[MYANNOTATION_LINE:150]],
// HYBRID-SAME: i8* null)
// HYBRID: call i8 addrspace(200)* @llvm.ptr.annotation.p200i8.p0i8(i8 addrspace(200)* %{{.+}}, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.4, i32 0, i32 0),
// HYBRID-SAME: i8* getelementptr inbounds ([[FILENAME_ARRAY]], [[FILENAME_ARRAY]]* @.str.1, i32 0, i32 0),
// HYBRID-SAME: i32 [[MYANNOTATION_LINE]],
// HYBRID-SAME: i8* null)
