// REQUIRES: mips-registered-target

// RUN: rm -f %t-hybrid.csv %t-purecap.csv
// RUN: %cheri128_cc1 %s -cheri-bounds=aggressive -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t-hybrid.csv -S -o /dev/null
//
// RUN: FileCheck -input-file %t-hybrid.csv %s -check-prefix HYBRID-CSV
// RUN: %cheri128_purecap_cc1 %s -mllvm -cheri-cap-table-abi=pcrel -cheri-bounds=aggressive \
// RUN:     -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t-purecap.csv -S -o /dev/null -O1
// RUN: FileCheck -input-file %t-purecap.csv %s -check-prefix PURECAP-CSV


// HYBRID-CSV: alignment_bits,size,kind,source_loc,compiler_pass,details
// PURECAP-CSV: alignment_bits,size,kind,source_loc,compiler_pass,details


struct Nested {
  int a;
  int b;
  int c;
};

struct WithNested {
  float f1;
  struct Nested n;
  float f2;
};

void do_stuff_with_int(int *);
void do_stuff_with_int_capability(int * __capability);
void do_stuff_with_float(float *);
void do_stuff_with_nested(struct Nested *nptr);

void test_subobject_addrof_basic(struct WithNested *s) {
  do_stuff_with_int(&s->n.a);
// PURECAP-CSV-NEXT: 0,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:21","Add subobject bounds","addrof operator on int"
  do_stuff_with_nested(&s->n);
// PURECAP-CSV-NEXT: 0,12,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:24","Add subobject bounds","addrof operator on struct Nested"
  do_stuff_with_float(&s->f1);
// PURECAP-CSV-NEXT: 0,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:23","Add subobject bounds","addrof operator on float"
}

void test_subobject_addrof_hybrid(struct Nested * __capability cap) {
  do_stuff_with_int_capability(&cap->a);
  // HYBRID-CSV-NEXT: 0,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:32","Add subobject bounds","addrof operator on int"
  // PURECAP-CSV-NEXT: 0,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-2]]:32","Add subobject bounds","addrof operator on int"
}

struct WithNested GlobalStruct;

void test_subobject_addrof_global() {
  do_stuff_with_float(&GlobalStruct.f1);
// PURECAP-CSV-NEXT: 2,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:23","Add subobject bounds","addrof operator on float"
  do_stuff_with_nested(&GlobalStruct.n);
// PURECAP-CSV-NEXT: 2,12,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:24","Add subobject bounds","addrof operator on struct Nested"
}

void test_subobject_addrof_assume_aligned(struct Nested * __capability cap) {
  struct Nested * __capability cap2 = __builtin_assume_aligned_cap(cap, 4096);
  do_stuff_with_int_capability(&cap2->a);
  // HYBRID-CSV-NEXT: 0,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:32","Add subobject bounds","addrof operator on int"
  // PURECAP-CSV-NEXT: 0,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-2]]:32","Add subobject bounds","addrof operator on int"
}

void test_subobject_addrof_alignas(struct Nested * __capability cap) {
  _Alignas(128) struct Nested onstack128;
  // HYBRID-CSV-NEXT: 7,12,s,"<somewhere in test_subobject_addrof_alignas>","CHERI range checker",""
  do_stuff_with_int(&onstack128.a);
  // PURECAP-CSV-NEXT: 7,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:21","Add subobject bounds","addrof operator on int"
  do_stuff_with_int_capability(&onstack128.a);
  // PURECAP-CSV-NEXT: 7,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:32","Add subobject bounds","addrof operator on int"
  do_stuff_with_int(&onstack128.c);
  // PURECAP-CSV-NEXT: 3,4,o,"{{.+}}/csetbounds-stats-subobject.c:[[@LINE-1]]:21","Add subobject bounds","addrof operator on int"
  // now we only know 3 bits since it's been incremented by 8 -> min align 8
}

// global bounds log should not appear in the hybrid log:
// HYBRID-CSV-EMPTY:

// PURECAP-CSV-NEXT: 7,12,s,"<somewhere in test_subobject_addrof_alignas>","CHERI sandbox ABI setup","set bounds on AllocaInst onstack128"
// PURECAP-CSV-NEXT: 2,20,g,"<somewhere in test_subobject_addrof_global>","MipsTargetLowering::lowerGlobalAddress","load of global GlobalStruct (alloc size=20)"
// PURECAP-CSV-EMPTY:
