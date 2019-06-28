// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -std=c11 -O2 -emit-llvm -o - %s | %cheri_FileCheck %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c11 -O2 -S -o - %s | %cheri_FileCheck -check-prefixes=ASM,%cheri_type-ASM %s
int global;

unsigned long sizeof_cap(void) {
  return sizeof(void* __capability);
  // CHECK-LABEL: define i64 @sizeof_cap() local_unnamed_addr
  // CHECK: ret i64 [[#CAP_SIZE]]
  // ASM-LABEL: sizeof_cap
  // ASM: cjr     $c17
  // ASM: daddiu   $2, $zero, [[#CAP_SIZE]]
}

typedef struct {
  __uintcap_t intptr;
} IntptrStruct;

// TODO: we should just return the struct in $c3 instead of using an output parameter
IntptrStruct set_int() {
  IntptrStruct p;
  p.intptr = 0;
  return p;
  // CHECK-LABEL: define inreg { i8 addrspace(200)* } @set_int() local_unnamed_addr
  // CHECK: ret { i8 addrspace(200)* } zeroinitializer
  // ASM-LABEL: set_int
  // ASM:  cjr     $c17
  // ASM-NEXT:  cgetnull $c3
}

IntptrStruct set_int2(IntptrStruct p) {
  return p;
  // CHECK-LABEL: define inreg { i8 addrspace(200)* } @set_int2(i8 addrspace(200)* inreg %p.coerce) local_unnamed_addr
  // CHECK: %.fca.0.insert = insertvalue { i8 addrspace(200)* } undef, i8 addrspace(200)* %p.coerce, 0
  // CHECK: ret { i8 addrspace(200)* } %.fca.0.insert
  // ASM-LABEL: set_int2
  // ASM:       cjr     $c17
  // ASM-NEXT:  nop
}

__uintcap_t set_int3(IntptrStruct p) {
  return p.intptr;
  // CHECK-LABEL: define i8 addrspace(200)* @set_int3(i8 addrspace(200)* inreg readnone returned %p.coerce) local_unnamed_addr
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
  // CHECK-LABEL: define void @two_caps_struct(%struct.TwoCapsStruct addrspace(200)* noalias nocapture sret %agg.result, {{.*}}, i8 addrspace(200)* inreg %in.coerce0, i8 addrspace(200)* inreg %in.coerce1) local_unnamed_addr
  // CHECK: [[VAR1:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* %in.coerce0, i64 1)
  // CHECK: [[INTPTR_MEMBER:%.+]] = getelementptr inbounds %struct.TwoCapsStruct, %struct.TwoCapsStruct addrspace(200)* %agg.result, i64 0, i32 0
  // CHECK: store i8 addrspace(200)* [[VAR1]], i8 addrspace(200)* addrspace(200)* [[INTPTR_MEMBER]], align [[#CAP_SIZE]]
  // CHECK: [[CAP_MEMBER:%.+]] = getelementptr inbounds %struct.TwoCapsStruct, %struct.TwoCapsStruct addrspace(200)* %agg.result, i64 0, i32 1
  // CHECK: store i8 addrspace(200)* %in.coerce1, i8 addrspace(200)* addrspace(200)* [[CAP_MEMBER]], align [[#CAP_SIZE]]
  // CHECK: ret void
  // ASM-LABEL: two_caps_struct
  // ASM:       cincoffset      $c1, $c4, 1
  // ASM-NEXT:  csc     $c1, $zero, 0($c3)
  // ASM-NEXT:  cjr     $c17
  // ASM-NEXT:  csc     $c5, $zero, [[#CAP_SIZE]]($c3)

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
  // CHECK-LABEL: define inreg i8 addrspace(200)* @intcap_size_union() local_unnamed_addr
  // CHECK: ret i8 addrspace(200)* bitcast (i32 addrspace(200)* @global to i8 addrspace(200)*)
  // ASM-LABEL: intcap_size_union
  // ASM: clcbi $c3, %captab20(global)($c{{.+}})
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
  // CHECK-LABEL: define void @greater_than_intcap_size_union(%union.GreaterThanIntCapSizeUnion addrspace(200)* noalias nocapture sret %agg.result) local_unnamed_addr
  // CHECK: [[CAP_MEMBER:%.+]] = getelementptr inbounds %union.GreaterThanIntCapSizeUnion, %union.GreaterThanIntCapSizeUnion addrspace(200)* %agg.result, i64 0, i32 0
  // CHECK: store i8 addrspace(200)* bitcast (i32 addrspace(200)* @global to i8 addrspace(200)*), i8 addrspace(200)* addrspace(200)* [[CAP_MEMBER]], align [[#CAP_SIZE]]
  // CHECK: ret void
  // ASM-LABEL: greater_than_intcap_size_union
  // ASM:       clcbi $c1, %captab20(global)($c{{.+}})
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
  // CHECK-LABEL: define inreg { i64 } @one_long() local_unnamed_addr
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
  // CHECK-LABEL: define inreg { i64, i64 } @two_longs() local_unnamed_addr
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
  // CHECK-LABEL: define void @three_longs(%struct.ThreeLongs addrspace(200)* noalias nocapture sret %agg.result) local_unnamed_addr
  // ASM-LABEL: three_longs
  // Clang now uses a memcpy from a global for cheri128
  // CHERI128-ASM: clcbi $c4, %captab20(.L__const.three_longs.t)($c{{.+}})
  // CHERI128-ASM: clcbi   $c12, %capcall20(memcpy)($c{{.+}})
  // For cheri256 clang will inline the memcpy from a global (since it is smaller than 1 cap)
  // CHERI256-ASM:      clcbi $c1, %captab20(.L__const.three_longs.t)($c{{.+}})
  // CHERI256-ASM-NEXT: cld	$1, $zero, 16($c1)
  // CHERI256-ASM-NEXT: cld	$2, $zero, 8($c1)
  // CHERI256-ASM-NEXT: cld	$3, $zero, 0($c1)
  // CHERI256-ASM-NEXT: csd	$1, $zero, 16($c3)
  // CHERI256-ASM-NEXT: csd	$2, $zero, 8($c3)
  // CHERI256-ASM-NEXT: cjr	$c17
  // CHERI256-ASM-NEXT: csd	$3, $zero, 0($c3)
}

typedef struct {
  int l1;
  long l2;
} IntAndLong;

IntAndLong int_and_long() {
  // Note: this looks wrong, but we actually have to use the in-memory big-endian representation for the registers!
  // See: https://github.com/CTSRD-CHERI/llvm-project/issues/310#issuecomment-497094466
  // Structs, unions, or other composite types are treated as a sequence of doublewords,
  // and are passed in integer or floating point registers as though they were simple
  // scalar parameters to the extent that they fit, with any excess on the stack packed
  // according to the normal memory layout of the object.
  // More specifically:
  //   – Regardless of the struct field structure, it is treated as a
  //     sequence of 64-bit chunks. If a chunk consists solely of a double
  //     float field (but not a double, which is part of a union), it is
  //     passed in a floating point register. Any other chunk is passed in
  //     an integer register
  //
  // CHECK-LABEL: define inreg { i64, i64 } @int_and_long() local_unnamed_addr
  // CHECK: ret { i64, i64 } { i64 8589934592, i64 3 }
  // ASM-LABEL: int_and_long
  // ASM:      daddiu	$1, $zero, 1
  // ASM-NEXT: dsll	$2, $1, 33
  // ASM-NEXT: cjr	$c17
  // ASM-NEXT: daddiu	$3, $zero, 3
  IntAndLong t = { 2, 3 };
  return t;
}

extern IntAndLong extern_int_and_long();

int read_int_and_long_1() {
  // This function needs to shift the l1 value by 32 to get the int value
  // (since the registers hold the in-memory representation)
  return extern_int_and_long().l1;
  // ASM-LABEL: read_int_and_long_1:
  // ASM: clcbi	$c12, %capcall20(extern_int_and_long)($c1)
  // ASM-NEXT: cjalr	$c12, $c17
  // ASM-NEXT: nop
  // This shift undoes the left-shift from int_and_long():
  // ASM-NEXT: dsra	$2, $2, 32
  // ASM-NEXT: clc	$c17, $zero, 0($c11)
  // ASM-NEXT: cjr	$c17
  // ASM-NEXT: cincoffset	$c11, $c11, [[#CAP_SIZE]]
}

long read_int_and_long_2() {
  // ASM-LABEL: read_int_and_long_2:
  // ASM: clcbi	$c12, %capcall20(extern_int_and_long)($c1)
  // ASM-NEXT: cjalr	$c12, $c17
  // ASM-NEXT: nop
  // Read the second 64-bit value from $v1 and move it to $v0
  // ASM-NEXT: move $2, $3
  // ASM-NEXT: clc	$c17, $zero, 0($c11)
  // ASM-NEXT: cjr	$c17
  // ASM-NEXT: cincoffset	$c11, $c11, [[#CAP_SIZE]]
  return extern_int_and_long().l2;
}

IntAndLong int_and_long2(IntAndLong arg) {
  // CHECK-LABEL: define inreg { i64, i64 } @int_and_long2(i64 inreg %arg.coerce0, i64 inreg %arg.coerce1) local_unnamed_addr
  // TODO-ASM: daddiu  $2, $zero, 3
  // ASM-LABEL: int_and_long2
  // ASM:     move     $2, $4
  // ASM-NEXT: cjr     $c17
  // ASM-NEXT: move     $3, $5
  return arg;
}
