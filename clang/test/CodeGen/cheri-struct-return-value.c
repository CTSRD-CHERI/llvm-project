// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -std=c11 -O2 -emit-llvm -S -o - %s | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -std=c11 -O2 -emit-llvm -S -mcpu=cheri128 -mllvm -cheri128-test-mode -o - %s | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -std=c11 -O2 -S -fomit-frame-pointer -o - %s | FileCheck -check-prefix=ASM %s
// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -std=c11 -O2 -S -fomit-frame-pointer -mcpu=cheri128 -mllvm -cheri128-test-mode -o - %s | FileCheck -check-prefix=ASM %s

int global;

unsigned long sizeof_cap(void) {
  return sizeof(void* __capability);
  // CHECK: define i64 @sizeof_cap() local_unnamed_addr #0 {
  // CHECK: ret i64 [[CAP_SIZE:16|32]]
  // ASM-LABEL: sizeof_cap
  // ASM: cjr     $c17
  // ASM: daddiu   $2, $zero, [[CAP_SIZE:16|32]]
}

typedef struct {
  __uintcap_t intptr;
} IntptrStruct;

// TODO: we should just return the struct in $c3 instead of using an output parameter
IntptrStruct set_int() {
  IntptrStruct p;
  p.intptr = 0;
  return p;
  // CHECK: define inreg { i8 addrspace(200)* } @set_int() local_unnamed_addr #1 {
  // CHECK: %0 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 0)
  // CHECK: %.fca.0.insert = insertvalue { i8 addrspace(200)* } undef, i8 addrspace(200)* %0, 0
  // CHECK: ret { i8 addrspace(200)* } %.fca.0.insert
  // ASM-LABEL: set_int
  // ASM:       cfromptr        $c1, $c0, $zero
  // ASM-NEXT:  cjr     $c17
  // ASM-NEXT:  csetoffset      $c3, $c1, $zero
}

IntptrStruct set_int2(IntptrStruct p) {
  return p;
  // CHECK: define inreg { i8 addrspace(200)* } @set_int2(i8 addrspace(200)* inreg %p.coerce) local_unnamed_addr #0 {
  // CHECK: %.fca.0.insert = insertvalue { i8 addrspace(200)* } undef, i8 addrspace(200)* %p.coerce, 0
  // CHECK: ret { i8 addrspace(200)* } %.fca.0.insert
  // ASM-LABEL: set_int2
  // ASM:       cjr     $c17
  // ASM-NEXT:  nop
}

__uintcap_t set_int3(IntptrStruct p) {
  return p.intptr;
  // CHECK: define i8 addrspace(200)* @set_int3(i8 addrspace(200)* inreg readnone returned %p.coerce) local_unnamed_addr #0 {
  // CHECK: ret i8 addrspace(200)* %p.coerce
  // ASM-LABEL: set_int3
  // ASM:       cjr     $c17
  // ASM-NEXT:  nop
}

typedef struct {
  __uintcap_t intptr;
  void* __capability ptr;
} TwoCapsStruct;

TwoCapsStruct two_caps_struct(TwoCapsStruct in) {
  TwoCapsStruct t;
  t.intptr = in.intptr + 1;
  t.ptr = in.ptr;
  return t;
  // argument is split up into two cap regs, but return value is indirect
  // CHECK: define void @two_caps_struct(%struct.TwoCapsStruct addrspace(200)* noalias nocapture sret %agg.result, i64, i8 addrspace(200)* inreg %in.coerce0, i8 addrspace(200)* inreg %in.coerce1) local_unnamed_addr #4 {
  // CHECK: %1 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %in.coerce0, i64 1)
  // CHECK: %t.sroa.0.0..sroa_idx = getelementptr inbounds %struct.TwoCapsStruct, %struct.TwoCapsStruct addrspace(200)* %agg.result, i64 0, i32 0
  // CHECK: store i8 addrspace(200)* %1, i8 addrspace(200)* addrspace(200)* %t.sroa.0.0..sroa_idx, align [[CAP_SIZE]]
  // CHECK: %t.sroa.4.0..sroa_idx4 = getelementptr inbounds %struct.TwoCapsStruct, %struct.TwoCapsStruct addrspace(200)* %agg.result, i64 0, i32 1
  // CHECK: store i8 addrspace(200)* %in.coerce1, i8 addrspace(200)* addrspace(200)* %t.sroa.4.0..sroa_idx4, align [[CAP_SIZE]]
  // CHECK: ret void
  // ASM-LABEL: two_caps_struct
  // ASM:       daddiu  $1, $zero, 1
  // ASM-NEXT:  cincoffset      $c1, $c4, $1
  // ASM-NEXT:  csc     $c1, $zero, 0($c3)
  // ASM-NEXT:  cjr     $c17
  // ASM-NEXT:  csc     $c5, $zero, [[CAP_SIZE]]($c3)

}

typedef union {
  __uintcap_t intptr;
  void * __capability ptr;
  long longvalue;
} IntCapSizeUnion;
_Static_assert(sizeof(IntCapSizeUnion) == sizeof(void*), "");

IntCapSizeUnion intcap_size_union() {
  IntCapSizeUnion i;
  i.ptr = &global;
  return i;
  // CHECK: define inreg i8 addrspace(200)* @intcap_size_union() local_unnamed_addr #0 {
  // CHECK: ret i8 addrspace(200)* bitcast (i32 addrspace(200)* @global to i8 addrspace(200)*)
  // ASM-LABEL: intcap_size_union
  // ASM:       ld      $2, %got_disp(.size.global)($1)
  // ASM-NEXT:  ld      $1, %got_disp(global)($1)
  // ASM-NEXT:  ld      $2, 0($2)
  // ASM-NEXT:  cfromptr        $c1, $c0, $1
  // ASM-NEXT:  cjr     $c17
  // ASM-NEXT:  csetbounds      $c3, $c1, $2
}

// Check that a union with size > intcap_t is not returned as a value
typedef union {
  __uintcap_t intptr;
  void* __capability ptr;
  long longvalue;
  char buffer[sizeof(__uintcap_t) + 1];
} GreaterThanIntCapSizeUnion;
_Static_assert(sizeof(GreaterThanIntCapSizeUnion) > sizeof(void*), "");

GreaterThanIntCapSizeUnion greater_than_intcap_size_union() {
  GreaterThanIntCapSizeUnion g;
  g.ptr = &global;
  return g;
  // CHECK: define void @greater_than_intcap_size_union(%union.GreaterThanIntCapSizeUnion addrspace(200)* noalias nocapture sret %agg.result) local_unnamed_addr #4 {
  // CHECK: %g.sroa.0.0..sroa_idx = getelementptr inbounds %union.GreaterThanIntCapSizeUnion, %union.GreaterThanIntCapSizeUnion addrspace(200)* %agg.result, i64 0, i32 0
  // CHECK: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @global to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* %g.sroa.0.0..sroa_idx, align [[CAP_SIZE]]
  // CHECK: ret void
  // ASM-LABEL: greater_than_intcap_size_union
  // ASM:       ld      $2, %got_disp(.size.global)($1)
  // ASM-NEXT:  ld      $1, %got_disp(global)($1)
  // ASM-NEXT:  ld      $2, 0($2)
  // ASM-NEXT:  cfromptr        $c1, $c0, $1
  // ASM-NEXT:  csetbounds      $c1, $c1, $2
  // ASM-NEXT:  cjr     $c17
  // ASM-NEXT:  csc     $c1, $zero, 0($c3)
}

// Check that we didn't break the normal case of returning small structs in integer registers
typedef struct {
  long l1;
} OneLong;

OneLong one_long() {
  OneLong o = { 1 };
  return o;
  // CHECK: define inreg { i64 } @one_long() local_unnamed_addr #0 {
  // CHECK: ret { i64 } { i64 1 }
  // ASM-LABEL: one_long
  // ASM:       cjr     $c17
  // ASM-NEXT:  daddiu  $2, $zero, 1
}
typedef struct {
  long l1;
  long l2;
} TwoLongs;

TwoLongs two_longs() {
  TwoLongs t = { 1, 2 };
  return t;
  // CHECK: define inreg { i64, i64 } @two_longs() local_unnamed_addr #0 {
  // CHECK:   ret { i64, i64 } { i64 1, i64 2 }
  // ASM-LABEL: two_longs
  // ASM:       daddiu  $2, $zero, 1
  // ASM-NEXT:  cjr     $c17
  // ASM-NEXT:  daddiu  $3, $zero, 2
}

typedef struct {
  long l1;
  long l2;
  long l3;
} ThreeLongs;

ThreeLongs three_longs() {
  ThreeLongs t = { 1, 2, 3 };
  return t;
  // CHECK: define void @three_longs(%struct.ThreeLongs addrspace(200)* noalias nocapture sret %agg.result) local_unnamed_addr #4 {
  // CHECK: tail call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %0, i8 addrspace(200)* bitcast (%struct.ThreeLongs addrspace(200)* @three_longs.t to i8 addrspace(200)*), i64 24, i32 8, i1 false), !tbaa.struct !2
  // ASM-LABEL: three_longs
  // ASM: ld      $1, %call16(memcpy_c)($gp)
}

typedef struct {
  int l1;
  long l2;
} IntAndLong;

IntAndLong int_and_long() {
  // FIXME: this seems very wrong, we are returning registers so it should not assume this is an in-memory big-endian representation
  // CHECK: define inreg { i64, i64 } @int_and_long() local_unnamed_addr #0 {
  // TODO-CHECK-NOT: ret { i64, i64 } { i64 8589934592, i64 3 }
  // TODO-CHECK:     ret { i32, i64 } { i32 2, i64 3 }
  // ASM-LABEL: int_and_long
  // TODO-ASM-NOT: daddiu  $1, $zero, 1
  // TODO-ASM-NOT: dsll    $2, $1, 33
  // TODO-ASM: daddiu  $2, $zero, 3
  // ASM:            cjr     $c17
  // ASM-NEXT:       daddiu  $3, $zero, 3
  IntAndLong t = { 2, 3 };
  return t;
}
