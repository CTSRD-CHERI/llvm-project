// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py
// Check that the default is address interpretation of uintcap_t:
// RUN: %cheri_purecap_cc1 -emit-llvm %s -O2 -o - -Wno-cheri-bitwise-operations -Wno-cheri-provenance | %cheri_FileCheck %s -check-prefixes BOTH,ADDR -enable-var-scope
// RUN: %cheri_purecap_cc1 -cheri-uintcap=offset -emit-llvm %s -O2 -o - -Wno-cheri-provenance -Wno-cheri-bitwise-operations | %cheri_FileCheck %s -check-prefixes BOTH,OFFSET -enable-var-scope
// RUN: %cheri_purecap_cc1 -cheri-uintcap=addr -emit-llvm %s -O2 -o - -Wno-cheri-provenance -Wno-cheri-bitwise-operations | %cheri_FileCheck %s -check-prefixes BOTH,ADDR -enable-var-scope

// ADDR-LABEL: @uintcap_to_long(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    ret i64 [[TMP0]]
//
// OFFSET-LABEL: @uintcap_to_long(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    ret i64 [[TMP0]]
//
long uintcap_to_long(__uintcap_t cap) {
  return (long)cap;
}

// BOTH-LABEL: @cap_to_long(
// BOTH-NEXT:  entry:
// BOTH-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// BOTH-NEXT:    ret i64 [[TMP0]]
//
long cap_to_long(void *__capability cap) {
  // Casting a pointer to long should give the address in both modes:
  return (long)cap;
}

// BOTH-LABEL: @write_uintcap(
// BOTH-NEXT:  entry:
// BOTH-NEXT:    store ptr addrspace(200) getelementptr (i8, ptr addrspace(200) null, i64 5), ptr addrspace(200) [[CAP:%.*]], align 16, !tbaa [[TBAA2:![0-9]+]]
// BOTH-NEXT:    ret i64 5
//
long write_uintcap(__uintcap_t *cap) {
  *cap = 5;
  return *cap;
}

// ADDR-LABEL: @get_low_bits(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 3
// ADDR-NEXT:    ret i64 [[AND]]
//
// OFFSET-LABEL: @get_low_bits(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 3
// OFFSET-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[CAP]], i64 [[AND]])
// OFFSET-NEXT:    [[TMP2:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[TMP1]])
// OFFSET-NEXT:    ret i64 [[TMP2]]
//
long get_low_bits(__uintcap_t cap) {
  return cap & 3;
}

// ADDR-LABEL: @get_low_bits_uintcap(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 3
// ADDR-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200) [[CAP]], i64 [[AND]])
// ADDR-NEXT:    ret ptr addrspace(200) [[TMP1]]
//
// OFFSET-LABEL: @get_low_bits_uintcap(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 3
// OFFSET-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[CAP]], i64 [[AND]])
// OFFSET-NEXT:    ret ptr addrspace(200) [[TMP1]]
//
__uintcap_t get_low_bits_uintcap(__uintcap_t cap) {
  return cap & 3;
}

// ADDR-LABEL: @xor_self(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200) [[CAP:%.*]], i64 0)
// ADDR-NEXT:    ret ptr addrspace(200) [[TMP0]]
//
// OFFSET-LABEL: @xor_self(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[CAP:%.*]], i64 0)
// OFFSET-NEXT:    ret ptr addrspace(200) [[TMP0]]
//
__uintcap_t xor_self(__uintcap_t cap) {
  return cap ^ cap;
}

// BOTH-LABEL: @xor_self_return_long(
// BOTH-NEXT:  entry:
// BOTH-NEXT:    ret i64 0
//
long xor_self_return_long(__uintcap_t cap) {
  return cap ^ cap;
}

// ADDR-LABEL: @xor_uintcap(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP2:%.*]])
// ADDR-NEXT:    [[XOR:%.*]] = xor i64 [[TMP1]], [[TMP0]]
// ADDR-NEXT:    [[TMP2:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200) [[CAP]], i64 [[XOR]])
// ADDR-NEXT:    ret ptr addrspace(200) [[TMP2]]
//
// OFFSET-LABEL: @xor_uintcap(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP2:%.*]])
// OFFSET-NEXT:    [[XOR:%.*]] = xor i64 [[TMP1]], [[TMP0]]
// OFFSET-NEXT:    [[TMP2:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[CAP]], i64 [[XOR]])
// OFFSET-NEXT:    ret ptr addrspace(200) [[TMP2]]
//
__uintcap_t xor_uintcap(__uintcap_t cap, __uintcap_t cap2) {
  return cap ^ cap2;
}

// ADDR-LABEL: @xor_uintcap_return_long(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP2:%.*]])
// ADDR-NEXT:    [[XOR:%.*]] = xor i64 [[TMP1]], [[TMP0]]
// ADDR-NEXT:    ret i64 [[XOR]]
//
// OFFSET-LABEL: @xor_uintcap_return_long(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[TMP1:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP2:%.*]])
// OFFSET-NEXT:    [[XOR:%.*]] = xor i64 [[TMP1]], [[TMP0]]
// OFFSET-NEXT:    [[TMP2:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[CAP]], i64 [[XOR]])
// OFFSET-NEXT:    [[TMP3:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[TMP2]])
// OFFSET-NEXT:    ret i64 [[TMP3]]
//
long xor_uintcap_return_long(__uintcap_t cap, __uintcap_t cap2) {
  return cap ^ cap2;
}

// ADDR-LABEL: @modulo_return_uintcap(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[REM:%.*]] = and i64 [[TMP0]], 31
// ADDR-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200) [[CAP]], i64 [[REM]])
// ADDR-NEXT:    ret ptr addrspace(200) [[TMP1]]
//
// OFFSET-LABEL: @modulo_return_uintcap(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[REM:%.*]] = and i64 [[TMP0]], 31
// OFFSET-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[CAP]], i64 [[REM]])
// OFFSET-NEXT:    ret ptr addrspace(200) [[TMP1]]
//
__uintcap_t modulo_return_uintcap(__uintcap_t cap) {
  return cap % 32;
}

// ADDR-LABEL: @modulo_return_long(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[REM:%.*]] = and i64 [[TMP0]], 31
// ADDR-NEXT:    ret i64 [[REM]]
//
// OFFSET-LABEL: @modulo_return_long(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[REM:%.*]] = and i64 [[TMP0]], 31
// OFFSET-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[CAP]], i64 [[REM]])
// OFFSET-NEXT:    [[TMP2:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[TMP1]])
// OFFSET-NEXT:    ret i64 [[TMP2]]
//
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
// ADDR-LABEL: @this_broke_qmutex(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[MTX:%.*]])
// ADDR-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 1
// ADDR-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200) [[MTX]], i64 [[AND]])
// ADDR-NEXT:    [[CMP:%.*]] = icmp eq ptr addrspace(200) [[TMP1]], getelementptr (i8, ptr addrspace(200) null, i64 1)
// ADDR-NEXT:    br i1 [[CMP]], label [[IF_THEN:%.*]], label [[IF_END:%.*]]
// ADDR:       if.then:
// ADDR-NEXT:    tail call void @do_unlock() #[[ATTR6:[0-9]+]]
// ADDR-NEXT:    br label [[IF_END]]
// ADDR:       if.end:
// ADDR-NEXT:    ret void
//
// OFFSET-LABEL: @this_broke_qmutex(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[MTX:%.*]])
// OFFSET-NEXT:    [[AND:%.*]] = and i64 [[TMP0]], 1
// OFFSET-NEXT:    [[TMP1:%.*]] = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) [[MTX]], i64 [[AND]])
// OFFSET-NEXT:    [[CMP:%.*]] = icmp eq ptr addrspace(200) [[TMP1]], getelementptr (i8, ptr addrspace(200) null, i64 1)
// OFFSET-NEXT:    br i1 [[CMP]], label [[IF_THEN:%.*]], label [[IF_END:%.*]]
// OFFSET:       if.then:
// OFFSET-NEXT:    tail call void @do_unlock() #[[ATTR6:[0-9]+]]
// OFFSET-NEXT:    br label [[IF_END]]
// OFFSET:       if.end:
// OFFSET-NEXT:    ret void
//
void this_broke_qmutex(__uintcap_t mtx) {
  if ((mtx & (__uintcap_t)1) == (__uintcap_t)1) {
    do_unlock();
  }
}

// ADDR-LABEL: @uintcap_to_oversized(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[CONV:%.*]] = zext i64 [[TMP0]] to i128
// ADDR-NEXT:    ret i128 [[CONV]]
//
// OFFSET-LABEL: @uintcap_to_oversized(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[CONV:%.*]] = zext i64 [[TMP0]] to i128
// OFFSET-NEXT:    ret i128 [[CONV]]
//
__int128 uintcap_to_oversized(__uintcap_t cap) {
  return (__int128)cap;
}

// ADDR-LABEL: @intcap_to_oversized(
// ADDR-NEXT:  entry:
// ADDR-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// ADDR-NEXT:    [[CONV:%.*]] = sext i64 [[TMP0]] to i128
// ADDR-NEXT:    ret i128 [[CONV]]
//
// OFFSET-LABEL: @intcap_to_oversized(
// OFFSET-NEXT:  entry:
// OFFSET-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) [[CAP:%.*]])
// OFFSET-NEXT:    [[CONV:%.*]] = sext i64 [[TMP0]] to i128
// OFFSET-NEXT:    ret i128 [[CONV]]
//
__int128 intcap_to_oversized(__intcap_t cap) {
  return (__int128)cap;
}

/// Casts from a capability to an oversized integer previously crashed Clang,
/// but only for purecap and only when lacking a __cheri_addr/__cheri_offset.
///
/// NB: This should be zext despite the type being signed in order to be
/// consistent with integer pointers (which are just the semantics of an
/// oversized ptrtoint).
///
// BOTH-LABEL: @cap_to_oversized(
// BOTH-NEXT:  entry:
// BOTH-NEXT:    [[TMP0:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) [[CAP:%.*]])
// BOTH-NEXT:    [[CONV:%.*]] = zext i64 [[TMP0]] to i128
// BOTH-NEXT:    ret i128 [[CONV]]
//
__int128 cap_to_oversized(void *__capability cap) {
  // Casting a pointer to __int128 should give the address in both modes:
  return (__int128)cap;
}
