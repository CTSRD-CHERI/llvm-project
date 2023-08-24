// RUN: %cheri_cc1 %s -O1 -o - -emit-llvm | FileCheck %s --check-prefix=CAPS
// RUN: %clang_cc1 %s -O1 -triple i386-unknown-freebsd -o - -emit-llvm | FileCheck %s --check-prefix=PTRS

// Remove the static from all of the function prototypes so that this really exists.
#define static
#define inline 
#include <cheri.h>

// PTRS: define dso_local i32 @cheri_length_get(ptr nocapture noundef readnone
// PTRS: ret i32 -1
// PTRS: define dso_local i32 @cheri_base_get(ptr nocapture noundef readnone
// PTRS: ret i32 -1
// PTRS: define dso_local i32 @cheri_offset_get(ptr nocapture noundef readnone
// PTRS: ret i32 -1
// PTRS: define dso_local ptr @cheri_offset_set(ptr noundef readnone returned{{( %.+)?}}, i32
// PTRS: ret ptr
// PTRS: define dso_local i32 @cheri_type_get(ptr nocapture noundef readnone
// PTRS: ret i32 0
// PTRS: define dso_local zeroext i16 @cheri_perms_get(ptr nocapture noundef readnone
// PTRS: ret i16 0
// PTRS: define dso_local ptr @cheri_perms_and(ptr noundef readnone returned{{( %.+)?}}, i16 noundef zeroext
// PTRS: ret ptr
// PTRS: define dso_local zeroext i16 @cheri_flags_get(ptr nocapture noundef readnone
// PTRS: ret i16 0
// PTRS: define dso_local ptr @cheri_flags_set(ptr noundef readnone returned{{( %.+)?}}, i16 noundef zeroext
// PTRS: ret ptr
// PTRS: define dso_local zeroext i1 @cheri_tag_get(ptr nocapture noundef readnone
// PTRS: ret i1 false
// PTRS: define dso_local zeroext i1 @cheri_sealed_get(ptr nocapture noundef readnone
// PTRS: ret i1 false
// PTRS: define dso_local ptr @cheri_offset_increment(ptr noundef readnone{{( %.+)?}}, i32
// PTRS: %[[TEMP1:[0-9a-z.]+]] = getelementptr inbounds i8, ptr{{( %.+)?}}, i32
// PTRS: ret ptr %[[TEMP1]]
// PTRS: define dso_local ptr @cheri_tag_clear(ptr noundef readnone returned
// PTRS: ret ptr
// PTRS: define dso_local ptr @cheri_seal(ptr noundef readnone returned{{( %.+)?}}, ptr nocapture noundef readnone
// PTRS: ret ptr
// PTRS: define dso_local ptr @cheri_unseal(ptr noundef readnone returned{{( %.+)?}}, ptr nocapture noundef readnone
// PTRS: ret ptr
// PTRS: define dso_local ptr @cheri_cap_from_pointer(ptr nocapture noundef readnone{{( %.+)?}}, ptr noundef readnone returned{{( %.+)?}})
// PTRS: ret ptr
// PTRS: define dso_local ptr @cheri_cap_to_pointer(ptr nocapture noundef readnone{{( %.+)?}}, ptr noundef readnone returned{{( %.+)?}})
// PTRS: ret ptr
// PTRS: define dso_local void @cheri_perms_check(ptr nocapture noundef{{( %.+)?}}, i16 noundef zeroext
// PTRS: ret void
// PTRS: define dso_local void @cheri_type_check(ptr nocapture noundef{{( %.+)?}}, ptr nocapture
// PTRS: ret void
// PTRS: define dso_local noalias ptr @cheri_global_data_get()
// PTRS: ret ptr null
// PTRS: define dso_local noalias ptr @cheri_program_counter_get()
// PTRS: ret ptr null

// CAPS: define dso_local i64 @cheri_length_get(ptr addrspace(200) noundef readnone
// CAPS: call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200)
// CAPS: define dso_local i64 @cheri_base_get(ptr addrspace(200) noundef readnone
// CAPS: call i64 @llvm.cheri.cap.base.get.i64(ptr addrspace(200)
// CAPS: define dso_local i64 @cheri_offset_get(ptr addrspace(200) noundef readnone
// CAPS: call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_offset_set(ptr addrspace(200) noundef readnone{{( %.+)?}}, i64 noundef zeroext{{( %.+)?}}
// CAPS: call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200){{( %.+)?}}, i64{{( %.+)?}})
// CAPS: define dso_local signext i32 @cheri_type_get(ptr addrspace(200)
// CAPS: call i64 @llvm.cheri.cap.type.get.i64(ptr addrspace(200)
// CAPS: define dso_local zeroext i16 @cheri_perms_get(ptr addrspace(200)
// CAPS: call i64 @llvm.cheri.cap.perms.get.i64(ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_perms_and(ptr addrspace(200) noundef readnone{{( %.+)?}}, i16 noundef zeroext
// CAPS: call ptr addrspace(200) @llvm.cheri.cap.perms.and.i64(ptr addrspace(200){{( %.+)?}}, i64
// CAPS: define dso_local zeroext i16 @cheri_flags_get(ptr addrspace(200)
// CAPS: call i64 @llvm.cheri.cap.flags.get.i64(ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_flags_set(ptr addrspace(200) noundef readnone{{( %.+)?}}, i16 noundef zeroext
// CAPS: call ptr addrspace(200) @llvm.cheri.cap.flags.set.i64(ptr addrspace(200){{( %.+)?}}, i64
// CAPS: define dso_local zeroext i1 @cheri_tag_get(ptr addrspace(200) noundef readnone
// CAPS: call i1 @llvm.cheri.cap.tag.get(ptr addrspace(200)
// CAPS: define dso_local zeroext i1 @cheri_sealed_get(ptr addrspace(200) noundef readnone
// CAPS: call i1 @llvm.cheri.cap.sealed.get(ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_offset_increment(ptr addrspace(200) noundef readnone{{( %.+)?}}, i64 noundef signext
// CAPS: %__builtin_cheri_offset_increment = getelementptr i8, ptr addrspace(200) %__cap, i64 %__offset
// CAPS: ret ptr addrspace(200) %__builtin_cheri_offset_increment
// CAPS: define dso_local ptr addrspace(200) @cheri_tag_clear(ptr addrspace(200) noundef readnone
// CAPS: call ptr addrspace(200) @llvm.cheri.cap.tag.clear(ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_seal(ptr addrspace(200) noundef readnone{{( %.+)?}}, ptr addrspace(200) noundef readnone
// CAPS: call ptr addrspace(200) @llvm.cheri.cap.seal(ptr addrspace(200){{( %.+)?}}, ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_unseal(ptr addrspace(200) noundef readnone{{( %.+)?}}, ptr addrspace(200) noundef readnone
// CAPS: call ptr addrspace(200) @llvm.cheri.cap.unseal(ptr addrspace(200){{( %.+)?}}, ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_cap_from_pointer(ptr addrspace(200) noundef readnone{{( %.+)?}}, ptr noundef{{( %.+)?}})
// CAPS: call ptr addrspace(200) @llvm.cheri.cap.from.pointer.i64(ptr addrspace(200){{( %.+)?}}, i64
// CAPS: define dso_local ptr @cheri_cap_to_pointer(ptr addrspace(200) noundef{{( %.+)?}}, ptr addrspace(200) noundef{{( %.+)?}})
// CAPS: call i64 @llvm.cheri.cap.to.pointer.i64(ptr addrspace(200){{( %.+)?}}, ptr addrspace(200)
// CAPS: define dso_local void @cheri_perms_check(ptr addrspace(200) noundef{{( %.+)?}}, i16 noundef zeroext
// CAPS: call void @llvm.cheri.cap.perms.check.i64(ptr addrspace(200){{( %.+)?}}, i64
// CAPS: define dso_local void @cheri_type_check(ptr addrspace(200) noundef{{( %.+)?}}, ptr addrspace(200)
// CAPS: call void @llvm.cheri.cap.type.check(ptr addrspace(200){{( %.+)?}}, ptr addrspace(200)
// CAPS: define dso_local ptr addrspace(200) @cheri_global_data_get()
// CAPS: call ptr addrspace(200) @llvm.cheri.ddc.get()
// CAPS: define dso_local ptr addrspace(200) @cheri_program_counter_get()
// CAPS: call ptr addrspace(200) @llvm.cheri.pcc.get()
