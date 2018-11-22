// RUN: rm -f %t-hybrid.csv %t-purecap.csv
// RUN: %cheri_purecap_cc1 %s -mllvm -cheri-cap-table-abi=pcrel -cheri-bounds=aggressive \
// RUN:   -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t-purecap.csv -S -o /dev/null
// RUN: cat %t-purecap.csv
// RUN: FileCheck -input-file %t-purecap.csv %s -check-prefixes CSV,CSV-CAPTABLE
// RUN: rm -f %t-hybrid.csv %t-purecap.csv
// RUN: %cheri_purecap_cc1 %s -mllvm -cheri-cap-table-abi=legacy -debug-info-kind=standalone -cheri-bounds=aggressive \
// RUN:    -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t-purecap.csv -S -o /dev/null
// RUN: cat %t-purecap.csv
// RUN: FileCheck -input-file %t-purecap.csv %s -check-prefixes CSV,CSV-LEGACY

// CSV: alignment_bits,size,kind,source_loc,compiler_pass,details
struct Foo {
  int a;
  float b;
  int c;
};
extern void do_stuff_with_buf(char *);
extern void do_stuff_with_int(int *);
extern void do_stuff_with_int_ref(int &);
extern void do_stuff_with_float(float *);

void test_subobject_addrof_basic(struct Foo *s) {
  do_stuff_with_int(&s->a);
  // CSV-NEXT: 0,4,?,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-1]]:21","Add subobject bounds","addrof operator on int"
  do_stuff_with_float(&s->b);
  // CSV-NEXT: 0,4,?,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-1]]:23","Add subobject bounds","addrof operator on float"
  do_stuff_with_int_ref(s->a);
  // CSV-NEXT: 0,4,?,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-1]]:28","Add subobject bounds","C++ reference on int"
}

void test_onstack_int(struct Foo *s) {
  int x = 0;
  do_stuff_with_int(&x);
  // CSV-NEXT: 2,4,?,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-1]]:21","Add subobject bounds","addrof operator on int"
  do_stuff_with_int_ref(x);
  // CSV-NEXT: 2,4,?,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-1]]:25","Add subobject bounds","C++ reference on int"
}

void test_onstack_int_overaligned(struct Foo *s) {
  alignas(32) int x = 0;
  // We know this int is at least 5 bits aligned:
  do_stuff_with_int(&x);
  // CSV-NEXT: 5,4,?,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-1]]:21","Add subobject bounds","addrof operator on int"
  do_stuff_with_int_ref(x);
  // CSV-NEXT: 5,4,?,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-1]]:25","Add subobject bounds","C++ reference on int"
}

void test__stack_array(void) {
  char buf[333];
  do_stuff_with_buf(buf);
}

void test_alloca(int n) {
  char *allocabuf = (char *)__builtin_alloca(n);
  do_stuff_with_buf(allocabuf);
}

void test_varlen_stack_array(int n) {
  char varlenbuf[n];
  do_stuff_with_buf(varlenbuf);
}

struct Foo global_foo;

int load_global_variable() {
  // This should also result in a dump from MIPSISelLowering (and we know it's a global)
  int x = global_foo.a;
  // captable is compiled without debug info -> we only get the function name as location:
  // CSV-CAPTABLE-NEXT: 2,12,g,"somewhere in _Z20load_global_variablev","MipsTargetLowering::lowerGlobalAddress","load of global global_foo (alloc size=12)"
  // CSV-LEGACY-NEXT: 2,12,g,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-3]]","MipsTargetLowering::lowerGlobalAddress","load of global global_foo (alloc size=12)"
  // CSV-LEGACY-NEXT: 0,8,g,"{{.+}}/csetbounds-stats-all.cpp:[[@LINE-4]]","MipsTargetLowering::lowerGlobalAddress","load of global .size.global_foo (alloc size=8)"
  return x;
}

extern int foo(int, ...);

int test(void) {
  return foo(3, 1, 2, 3, 4ULL); // promoted to u64 in variadic call -> 4 * 8 bytes
  // CSV-NEXT:   0,32,s,"{{.+}}/csetbounds-stats-variadic-call.c:[[@LINE-1]]","variadic call lowering","setting varargs bounds for call to foo"
}

// CSV-NEXT: 4,16,s,"<somewhere in _Z27test_subobject_addrof_basicU3capP3Foo>","CHERI sandbox ABI setup","set bounds on AllocaInst s.addr"
// CSV-NEXT: 4,16,s,"<somewhere in _Z16test_onstack_intU3capP3Foo>","CHERI sandbox ABI setup","set bounds on AllocaInst s.addr"
// CSV-NEXT: 2,4,s,"<somewhere in _Z16test_onstack_intU3capP3Foo>","CHERI sandbox ABI setup","set bounds on AllocaInst x"
// CSV-NEXT: 4,16,s,"<somewhere in _Z28test_onstack_int_overalignedU3capP3Foo>","CHERI sandbox ABI setup","set bounds on AllocaInst s.addr"
// CSV-NEXT: 5,4,s,"<somewhere in _Z28test_onstack_int_overalignedU3capP3Foo>","CHERI sandbox ABI setup","set bounds on AllocaInst x"
// CSV-NEXT: 0,333,s,"<somewhere in _Z17test__stack_arrayv>","CHERI sandbox ABI setup","set bounds on AllocaInst buf"
// CSV-NEXT: 2,4,s,"<somewhere in _Z11test_allocai>","CHERI sandbox ABI setup","set bounds on AllocaInst n.addr"
// CSV-NEXT: 4,16,s,"<somewhere in _Z11test_allocai>","CHERI sandbox ABI setup","set bounds on AllocaInst allocabuf"
// CSV-NEXT: 7,<unknown>,s,"<somewhere in _Z11test_allocai>","CHERI sandbox ABI setup","set bounds on anonymous AllocaInst of type i8 addrspace(200)*"
// CSV-NEXT: 2,4,s,"<somewhere in _Z23test_varlen_stack_arrayi>","CHERI sandbox ABI setup","set bounds on AllocaInst n.addr"
// CSV-NEXT: 4,16,s,"<somewhere in _Z23test_varlen_stack_arrayi>","CHERI sandbox ABI setup","set bounds on AllocaInst saved_stack"
// CSV-NEXT: 3,8,s,"<somewhere in _Z23test_varlen_stack_arrayi>","CHERI sandbox ABI setup","set bounds on AllocaInst __vla_expr"
// CSV-NEXT: 7,<unknown>,s,"<somewhere in _Z23test_varlen_stack_arrayi>","CHERI sandbox ABI setup","set bounds on AllocaInst vla"
// CSV-NEXT: 2,4,s,"<somewhere in _Z20load_global_variablev>","CHERI sandbox ABI setup","set bounds on AllocaInst x"
// CSV-NEXT: 7,<unknown>,s,"<somewhere in _Z11test_allocai>","ExpandDYNAMIC_STACKALLOC",""
// CSV-NEXT: 7,<unknown>,s,"<somewhere in _Z23test_varlen_stack_arrayi>","ExpandDYNAMIC_STACKALLOC",""
// CSV-NEXT: 2,12,g,"<somewhere in _Z20load_global_variablev>","MipsTargetLowering::lowerGlobalAddress","load of global global_foo (alloc size=12)"
// CSV-NEXT: 0,32,s,"<somewhere in _Z4testv>","variadic call lowering","setting varargs bounds for call to _Z3fooiz"


// CSV-EMPTY:

