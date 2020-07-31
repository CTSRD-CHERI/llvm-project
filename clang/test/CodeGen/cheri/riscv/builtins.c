// RUN: %riscv32_cheri_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=RV32IXCHERI
// RUN: %riscv64_cheri_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=RV64IXCHERI

void standard(void * __capability cap) {
  // RV32IXCHERI-LABEL: @standard(
  // RV64IXCHERI-LABEL: @standard(
  volatile __SIZE_TYPE__ x;
  x = __builtin_cheri_length_get(cap);
  // RV32IXCHERI: call i32 @llvm.cheri.cap.length.get.i32
  // RV64IXCHERI: call i64 @llvm.cheri.cap.length.get.i64
  x = __builtin_cheri_perms_get(cap);
  // RV32IXCHERI: call i32 @llvm.cheri.cap.perms.get.i32
  // RV64IXCHERI: call i64 @llvm.cheri.cap.perms.get.i64
  x = __builtin_cheri_type_get(cap);
  // RV32IXCHERI: call i32 @llvm.cheri.cap.type.get.i32
  // RV64IXCHERI: call i64 @llvm.cheri.cap.type.get.i64
  x = __builtin_cheri_tag_get(cap);
  // RV32IXCHERI: call i1 @llvm.cheri.cap.tag.get
  // RV64IXCHERI: call i1 @llvm.cheri.cap.tag.get
  x = __builtin_cheri_offset_get(cap);
  // RV32IXCHERI: call i32 @llvm.cheri.cap.offset.get.i32
  // RV64IXCHERI: call i64 @llvm.cheri.cap.offset.get.i64
  x = __builtin_cheri_base_get(cap);
  // RV32IXCHERI: call i32 @llvm.cheri.cap.base.get.i32
  // RV64IXCHERI: call i64 @llvm.cheri.cap.base.get.i64
  x = __builtin_cheri_sealed_get(cap);
  // RV32IXCHERI: call i1 @llvm.cheri.cap.sealed.get
  // RV64IXCHERI: call i1 @llvm.cheri.cap.sealed.get

  void * volatile y;
  y = __builtin_cheri_cap_to_pointer(cap, cap);
  // RV32IXCHERI: call i32 @llvm.cheri.cap.to.pointer.i32
  // RV64IXCHERI: call i64 @llvm.cheri.cap.to.pointer.i64

  void * __capability volatile z;
  z = __builtin_cheri_cap_from_pointer(cap, y);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.i32
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.i64
  z = __builtin_cheri_perms_and(cap, 12);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i32
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64
  z = __builtin_cheri_seal(cap, cap);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.seal
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.seal
  z = __builtin_cheri_unseal(cap, cap);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.unseal
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.unseal
  z = __builtin_cheri_bounds_set(cap, 42);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i32(i8 addrspace(200)* {{.+}}, i32 42)
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* {{.+}}, i64 42)
  z = __builtin_cheri_bounds_set_exact(cap, 43);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.i32(i8 addrspace(200)* {{.+}}, i32 43)
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.i64(i8 addrspace(200)* {{.+}}, i64 43)

  __builtin_cheri_perms_check(cap, 12);
  // RV32IXCHERI: call void @llvm.cheri.cap.perms.check.i32
  // RV64IXCHERI: call void @llvm.cheri.cap.perms.check.i64
  __builtin_cheri_type_check(cap, z);
  // RV32IXCHERI: call void @llvm.cheri.cap.type.check
  // RV64IXCHERI: call void @llvm.cheri.cap.type.check
}

void buildcap(void * __capability auth, void * __capability bits) {
  // RV32IXCHERI-LABEL: @buildcap(
  // RV64IXCHERI-LABEL: @buildcap(
  void * __capability volatile tagged = __builtin_cheri_cap_build(auth, (__uintcap_t)bits);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.build
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.build
  void * __capability volatile unseal_auth = __builtin_cheri_cap_type_copy(auth, bits);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.type.copy
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.type.copy
  void * __capability volatile condseal = __builtin_cheri_conditional_seal(tagged, unseal_auth);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.conditional.seal
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.cap.conditional.seal
}

__SIZE_TYPE__ crrl_cram(__SIZE_TYPE__ len) {
  // RV32IXCHERI-LABEL: @crrl_cram(
  // RV64IXCHERI-LABEL: @crrl_cram(
  return __builtin_cheri_round_representable_length(len) & __builtin_cheri_representable_alignment_mask(len);
  // RV32IXCHERI: call i32 @llvm.cheri.round.representable.length.i32
  // RV32IXCHERI: call i32 @llvm.cheri.representable.alignment.mask.i32
  // RV64IXCHERI: call i64 @llvm.cheri.round.representable.length.i64
  // RV64IXCHERI: call i64 @llvm.cheri.representable.alignment.mask.i64
}
