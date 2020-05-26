// RUN: %cheri_cc1 %s -o - -emit-llvm -O1 -Weverything -Werror -verify | FileCheck %s
// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -O1 -Weverything -Werror -verify | FileCheck %s
// expected-no-diagnostics

#include <cheriintrin.h>
// Check that all macros defined in cheriintrin.h work as expected

void use_size_t(__SIZE_TYPE__ s);
void use_bool(_Bool b);
void use_cap(void *__capability p);

void test(void *__capability cap, char *__capability cap2, __SIZE_TYPE__ i);

// CHECK-LABEL: define {{[^@]+}}@test
// CHECK-SAME: (i8 addrspace(200)* [[CAP:%.*]], i8 addrspace(200)* [[CAP2:%.*]], i64 zeroext [[I:%.*]]) local_unnamed_addr
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP0]]) #5
// CHECK-NEXT:    [[TMP1:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* [[CAP]], i64 [[I]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP1]]) #5
// CHECK-NEXT:    [[TMP2:%.*]] = call i64 @llvm.cheri.cap.base.get.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP2]]) #5
// CHECK-NEXT:    [[TMP3:%.*]] = call i64 @llvm.cheri.cap.length.get.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP3]]) #5
// CHECK-NEXT:    [[TMP4:%.*]] = call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP4]]) #5
// CHECK-NEXT:    [[TMP5:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* [[CAP]], i64 [[I]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP5]]) #5
// CHECK-NEXT:    [[TMP6:%.*]] = call i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_bool(i1 zeroext [[TMP6]]) #5
// CHECK-NEXT:    [[TMP7:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.tag.clear(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP7]]) #5
// CHECK-NEXT:    [[TMP8:%.*]] = call i1 @llvm.cheri.cap.subset.test(i8 addrspace(200)* [[CAP]], i8 addrspace(200)* [[CAP2]])
// CHECK-NEXT:    call void @use_bool(i1 zeroext [[TMP8]]) #5
// CHECK-NEXT:    [[TMP9:%.*]] = call i64 @llvm.cheri.round.representable.length.i64(i64 [[I]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP9]]) #5
// CHECK-NEXT:    [[TMP10:%.*]] = call i64 @llvm.cheri.representable.alignment.mask.i64(i64 [[I]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP10]]) #5
// CHECK-NEXT:    [[TMP11:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* [[CAP]], i64 [[I]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP11]]) #5
// CHECK-NEXT:    [[TMP12:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.i64(i8 addrspace(200)* [[CAP]], i64 [[I]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP12]]) #5
// CHECK-NEXT:    [[TMP13:%.*]] = call i64 @llvm.cheri.cap.type.get.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP13]]) #5
// CHECK-NEXT:    [[TMP14:%.*]] = call i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_bool(i1 zeroext [[TMP14]]) #5
// CHECK-NEXT:    [[LNOT:%.*]] = xor i1 [[TMP14]], true
// CHECK-NEXT:    call void @use_bool(i1 zeroext [[LNOT]]) #5
// CHECK-NEXT:    [[CMP:%.*]] = icmp eq i64 [[TMP13]], -2
// CHECK-NEXT:    call void @use_bool(i1 zeroext [[CMP]]) #5
// CHECK-NEXT:    [[TMP15:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.seal.entry(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP15]]) #5
// CHECK-NEXT:    [[TMP16:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)* [[CAP]], i8 addrspace(200)* [[CAP2]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP16]]) #5
// CHECK-NEXT:    [[TMP17:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)* [[CAP]], i8 addrspace(200)* [[CAP2]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP17]]) #5
// CHECK-NEXT:    [[TMP18:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.build(i8 addrspace(200)* [[CAP]], i8 addrspace(200)* [[CAP2]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP18]]) #5
// CHECK-NEXT:    [[TMP19:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.conditional.seal(i8 addrspace(200)* [[CAP]], i8 addrspace(200)* [[CAP2]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP19]]) #5
// CHECK-NEXT:    [[TMP20:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.type.copy(i8 addrspace(200)* [[CAP]], i8 addrspace(200)* [[CAP2]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP20]]) #5
// CHECK-NEXT:    [[TMP21:%.*]] = call i64 @llvm.cheri.cap.perms.get.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    [[CONV1:%.*]] = and i64 [[TMP21]], 4294967295
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[CONV1]]) #5
// CHECK-NEXT:    [[TMP22:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)* [[CAP]], i64 4)
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP22]]) #5
// CHECK-NEXT:    [[TMP23:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)* [[CAP]], i64 -3)
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP23]]) #5
// CHECK-NEXT:    [[TMP24:%.*]] = call i8 addrspace(200)* @llvm.cheri.ddc.get()
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP24]]) #5
// CHECK-NEXT:    [[TMP25:%.*]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP25]]) #5
// CHECK-NEXT:    [[TMP26:%.*]] = call i64 @llvm.cheri.cap.flags.get.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP26]]) #5
// CHECK-NEXT:    [[TMP27:%.*]] = call i8 addrspace(200)* @llvm.cheri.cap.flags.set.i64(i8 addrspace(200)* [[CAP]], i64 [[I]])
// CHECK-NEXT:    call void @use_cap(i8 addrspace(200)* [[TMP27]]) #5
// CHECK-NEXT:    [[TMP28:%.*]] = call i64 @llvm.cheri.cap.load.tags.i64(i8 addrspace(200)* [[CAP]])
// CHECK-NEXT:    call void @use_size_t(i64 zeroext [[TMP28]]) #5
// CHECK-NEXT:    ret void
//
void test(void *__capability cap, char *__capability cap2, __SIZE_TYPE__ i) {
  use_size_t(cheri_get_address(cap));
  use_cap(cheri_set_address(cap, i));

  use_size_t(cheri_get_base(cap));

  use_size_t(cheri_get_length(cap));

  use_size_t(cheri_get_offset(cap));
  use_cap(cheri_set_offset(cap, i));

  use_bool(cheri_is_tagged(cap));
  use_cap(cheri_clear_tag(cap));

  use_bool(cheri_is_subset(cap, cap2));

  use_size_t(cheri_round_representable_length(i));
  use_size_t(cheri_representable_alignment_mask(i));

  use_cap(cheri_set_bounds(cap, i));
  use_cap(cheri_set_bounds_exact(cap, i));

  /* Check that the cheri_otype_t type is defined */
  _Static_assert(__builtin_types_compatible_p(cheri_otype_t *, __typeof__(cheri_get_type(cap)) *), "");
  _Static_assert(__builtin_types_compatible_p(cheri_otype_t *, long *), "");
  _Static_assert(CHERI_OTYPE_UNSEALED == -1, "RISCV and MIPS expect -2");
  _Static_assert(CHERI_OTYPE_SENTRY == -2, "RISCV and MIPS expect -2");
  use_size_t((unsigned long)cheri_get_type(cap));
  use_bool(cheri_is_sealed(cap));
  use_bool(cheri_is_unsealed(cap));
  use_bool(cheri_is_sentry(cap));
  use_cap(cheri_make_sentry(cap));
  use_cap(cheri_seal(cap, cap2));
  use_cap(cheri_unseal(cap, cap2));

  use_cap(cheri_build_cap(cap, cap2));
  use_cap(cheri_conditional_seal(cap, cap2));
  use_cap(cheri_copy_type(cap, cap2));

  _Static_assert(CHERI_PERM_GLOBAL != 0, "must be defined");
  _Static_assert(CHERI_PERM_EXECUTE != 0, "must be defined");
  _Static_assert(CHERI_PERM_LOAD != 0, "must be defined");
  _Static_assert(CHERI_PERM_STORE != 0, "must be defined");
  _Static_assert(CHERI_PERM_LOAD_CAP != 0, "must be defined");
  _Static_assert(CHERI_PERM_STORE_CAP != 0, "must be defined");
  _Static_assert(CHERI_PERM_STORE_LOCAL_CAP != 0, "must be defined");
  _Static_assert(CHERI_PERM_SEAL != 0, "must be defined");
  _Static_assert(CHERI_PERM_CCALL != 0, "must be defined");
  _Static_assert(CHERI_PERM_UNSEAL != 0, "must be defined");
  _Static_assert(CHERI_PERM_SYSTEM_REGS != 0, "must be defined");
  /* Check that CHERI_PERMS_T is defined */
  cheri_perms_t cap_perms = cheri_get_perms(cap);
  use_size_t(cap_perms);
  use_cap(cheri_and_perms(cap, CHERI_PERM_LOAD));
  use_cap(cheri_clear_perms(cap, CHERI_PERM_EXECUTE));

  use_cap(cheri_get_ddc());
  use_cap(cheri_get_pcc());

  use_size_t(cheri_get_flags(cap));
  use_cap(cheri_set_flags(cap, i));

  use_size_t(cheri_load_tags(cap));
}

/* We also define macros for __builtin_is_aligned/__builtin_align_{up,down}().
 * They are not CHERI specific, but using __builtin_* is ugly.
 *
 * TOOD: we may want to provide nicer names in stdalign.h.
 */
void test_alignment_builtins(void *__capability cap, __SIZE_TYPE__ align);
void test_alignment_builtins(void *__capability cap, __SIZE_TYPE__ align) {
  use_cap(cheri_align_up(cap, align));
  use_cap(cheri_align_down(cap, align));
  use_bool(cheri_is_aligned(cap, align));
}
