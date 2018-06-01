// RUN: %cheri_purecap_cc1 -emit-llvm %s -O2 -o - -Wno-cheri-bitwise-operations | %cheri_FileCheck %s -check-prefixes BOTH,OFFSET -enable-var-scope
// RUN: %cheri_purecap_cc1 -cheri-uintcap=offset -emit-llvm %s -O2 -o - -Wno-cheri-bitwise-operations | %cheri_FileCheck %s -check-prefixes BOTH,OFFSET -enable-var-scope
// RUN: %cheri_purecap_cc1 -cheri-uintcap=addr -emit-llvm %s -O2 -o - -Wno-cheri-bitwise-operations | %cheri_FileCheck %s -check-prefixes BOTH,ADDR -enable-var-scope

long uintcap_to_long(__uintcap_t cap) {
  // BOTH-LABEL: define i64 @uintcap_to_long(i8 addrspace(200)* readnone
  // OFFSET: [[RESULT:%.+]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %{{.+}})
  // ADDR: [[RESULT:%.+]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* %{{.+}})
  // BOTH-NEXT: ret i64 [[RESULT]]
  return (long)cap;
}

long cap_to_long(void *__capability cap) {
  // Casting a pointer to long should give the address in both modes:
  // BOTH-LABEL: define i64 @cap_to_long(i8 addrspace(200)* readnone
  // BOTH: [[RESULT:%.+]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* %{{.+}})
  // BOTH-NEXT: ret i64 [[RESULT]]
  return (long)cap;
}

long write_uintcap(__uintcap_t *cap) {
  // BOTH-LABEL: @write_uintcap(
  // OFFSET: [[RESULT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 5)
  // ADDR: [[RESULT:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set(i8 addrspace(200)* null, i64 5)
  // BOTH-NEXT: store i8 addrspace(200)* [[RESULT]], i8 addrspace(200)* addrspace(200)* %cap
  // BOTH-NEXT: ret i64 5
  *cap = 5;
  return *cap;
}

// BOTH-LABEL: @get_low_bits(
// OFFSET:       [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP:%.*]])
// ADDR:         [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP:%.*]])
// BOTH-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 3
// BOTH-NEXT:    ret i64 [[AND]]
long get_low_bits(__uintcap_t cap) {
  return cap & 3;
}

// BOTH-LABEL: @get_low_bits_uintcap(
// OFFSET:       [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP:%.*]])
// ADDR:         [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP:%.*]])
// BOTH-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 3
// OFFSET-NEXT:  [[TMP1:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[CAP]], i64 [[AND]])
// ADDR-NEXT: [[TMP1:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set(i8 addrspace(200)* [[CAP]], i64 [[AND]])
// BOTH-NEXT:    ret i8 addrspace(200)* [[TMP1]]
__uintcap_t get_low_bits_uintcap(__uintcap_t cap) {
  return cap & 3;
}

// BOTH-LABEL: @xor_self(
// OFFSET:       [[TMP0:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[CAP:%.*]], i64 0)
// ADDR:         [[TMP0:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set(i8 addrspace(200)* [[CAP:%.*]], i64 0)
// BOTH-NEXT:    ret i8 addrspace(200)* [[TMP0]]
__uintcap_t xor_self(__uintcap_t cap) {
  return cap ^ cap;
}

// BOTH-LABEL: @xor_self_return_long(
// BOTH:    ret i64 0
//
long xor_self_return_long(__uintcap_t cap) {
  return cap ^ cap;
}

// BOTH-LABEL: @xor_uintcap(
// OFFSET:       [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP:%.*]])
// OFFSET-NEXT:  [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP2:%.*]])
// ADDR:         [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP:%.*]])
// ADDR-NEXT:    [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP2:%.*]])
// BOTH-NEXT:    [[XOR:%.*]] = xor i64 [[TMP1]], [[TMP0]]
// OFFSET-NEXT:  [[TMP2:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[CAP]], i64 [[XOR]])
// ADDR-NEXT:    [[TMP2:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set(i8 addrspace(200)* [[CAP]], i64 [[XOR]])
// BOTH-NEXT:    ret i8 addrspace(200)* [[TMP2]]
//
__uintcap_t xor_uintcap(__uintcap_t cap, __uintcap_t cap2) {
  return cap ^ cap2;
}

// BOTH-LABEL: @xor_uintcap_return_long(
// OFFSET:       [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP:%.*]])
// OFFSET-NEXT:  [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP2:%.*]])
// ADDR:         [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP:%.*]])
// ADDR-NEXT:    [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP2:%.*]])
// BOTH-NEXT:    [[XOR:%.*]] = xor i64 [[TMP1]], [[TMP0]]
// BOTH-NEXT:    ret i64 [[XOR]]
//
long xor_uintcap_return_long(__uintcap_t cap, __uintcap_t cap2) {
  return cap ^ cap2;
}

// BOTH-LABEL: @modulo_return_uintcap(
// OFFSET:       [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP:%.*]])
// ADDR:         [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP:%.*]])
// BOTH-NEXT:    [[REM:%.*]] = and i64 [[TMP0]], 31
// OFFSET-NEXT:  [[TMP1:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[CAP]], i64 [[REM]])
// ADDR-NEXT:    [[TMP1:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set(i8 addrspace(200)* [[CAP]], i64 [[REM]])
// BOTH-NEXT:    ret i8 addrspace(200)* [[TMP1]]
__uintcap_t modulo_return_uintcap(__uintcap_t cap) {
  return cap % 32;
}

// BOTH-LABEL: @modulo_return_long(
// OFFSET:       [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[CAP:%.*]])
// ADDR:         [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[CAP:%.*]])
// BOTH-NEXT:    [[REM:%.*]] = and i64 [[TMP0]], 31
// BOTH-NEXT:    ret i64 [[REM]]
long modulo_return_long(__uintcap_t cap) {
  return cap % 32;
}

// TODO: check bitwise or, shifts, and not
// TODO: check some sample alignment code

// This code is what caused QMutex to deadlock because mutex with offset set to
// cheri_getoffset(mtx) & 1 is always a valid capability with provenance
// inherited from mtx.
// In Vaddr mode this will probably also not work as expected since the setaddr(1)
// call will produce a capability that has become unrepresentable in most cases
// and it is not guaranteed (most likely it will not) to compare equal to untagged 1

// TODO: this needs a value-depenedent provenance for bitwise and (i.e. inherit
// from null when performing an and with small values, but inherit if the value
// has the sign bit set (i.e. we are clearing the low bits of the pointer)

void do_unlock(void);
// BOTH-LABEL: @this_broke_qmutex(
// OFFSET:       [[TMP0:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 1)
// OFFSET-NEXT:  [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* [[MTX:%.*]])
// ADDR:         [[TMP0:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set(i8 addrspace(200)* null, i64 1)
// ADDR-NEXT:    [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* [[MTX:%.*]])
// BOTH-NEXT:    [[AND:%.*]] = and i64 [[TMP1]], 1
// OFFSET-NEXT:  [[TMP2:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[MTX]], i64 [[AND]])
// ADDR-NEXT:    [[TMP2:%.*]] = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set(i8 addrspace(200)* [[MTX]], i64 [[AND]])
// BOTH-NEXT:    [[CMP:%.*]] = icmp eq i8 addrspace(200)* [[TMP2]], [[TMP0]]
// BOTH-NEXT:    br i1 [[CMP]], label [[IF_THEN:%.*]], label [[IF_END:%.*]]
// BOTH:       if.then:
// BOTH-NEXT:    tail call void @do_unlock() #5
// BOTH-NEXT:    br label [[IF_END]]
// BOTH:       if.end:
// BOTH-NEXT:    ret void
//
void this_broke_qmutex(__uintcap_t mtx) {
  if ((mtx & (__uintcap_t)1) == (__uintcap_t)1) {
    do_unlock();
  }
}
