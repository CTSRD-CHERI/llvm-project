// RUN: %cheri_cc1 %s -O1 -o - -emit-llvm | FileCheck %s --check-prefix=CAPS
// RUN: %clang_cc1 %s -O1 -triple i386-unknown-freebsd -o - -emit-llvm | FileCheck %s --check-prefix=PTRS

// Remove the static from all of the function prototypes so that this really exists.
#define static
#define inline 
#include <cheri.h>

// PTRS: define dso_local i32 @cheri_length_get(i8* nocapture readnone
// PTRS: ret i32 -1
// PTRS: define dso_local i32 @cheri_base_get(i8* nocapture readnone
// PTRS: ret i32 -1
// PTRS: define dso_local i32 @cheri_offset_get(i8* nocapture readnone
// PTRS: ret i32 -1
// PTRS: define dso_local i8* @cheri_offset_set(i8* readnone returned{{( %.+)?}}, i32
// PTRS: ret i8*
// PTRS: define dso_local i32 @cheri_type_get(i8* nocapture readnone
// PTRS: ret i32 0
// PTRS: define dso_local zeroext i16 @cheri_perms_get(i8* nocapture readnone
// PTRS: ret i16 0
// PTRS: define dso_local i8* @cheri_perms_and(i8* readnone returned{{( %.+)?}}, i16 zeroext
// PTRS: ret i8*
// PTRS: define dso_local zeroext i16 @cheri_flags_get(i8* nocapture readnone
// PTRS: ret i16 0
// PTRS: define dso_local i8* @cheri_flags_set(i8* readnone returned{{( %.+)?}}, i16 zeroext
// PTRS: ret i8*
// PTRS: define dso_local zeroext i1 @cheri_tag_get(i8* nocapture readnone
// PTRS: ret i1 false
// PTRS: define dso_local zeroext i1 @cheri_sealed_get(i8* nocapture readnone
// PTRS: ret i1 false
// PTRS: define dso_local i8* @cheri_offset_increment(i8* readnone{{( %.+)?}}, i32
// PTRS: %[[TEMP1:[0-9a-z.]+]] = getelementptr inbounds i8, i8*{{( %.+)?}}, i32
// PTRS: ret i8* %[[TEMP1]]
// PTRS: define dso_local i8* @cheri_tag_clear(i8* readnone returned
// PTRS: ret i8*
// PTRS: define dso_local i8* @cheri_seal(i8* readnone returned{{( %.+)?}}, i8* nocapture readnone
// PTRS: ret i8*
// PTRS: define dso_local i8* @cheri_unseal(i8* readnone returned{{( %.+)?}}, i8* nocapture readnone
// PTRS: ret i8*
// PTRS: define dso_local i8* @cheri_cap_from_pointer(i8* nocapture readnone{{( %.+)?}}, i8* readnone returned{{( %.+)?}})
// PTRS: ret i8*
// PTRS: define dso_local i8* @cheri_cap_to_pointer(i8* nocapture readnone{{( %.+)?}}, i8* readnone returned{{( %.+)?}})
// PTRS: ret i8*
// PTRS: define dso_local void @cheri_perms_check(i8* nocapture{{( %.+)?}}, i16 zeroext
// PTRS: ret void
// PTRS: define dso_local void @cheri_type_check(i8* nocapture{{( %.+)?}}, i8* nocapture
// PTRS: ret void
// PTRS: define dso_local noalias i8* @cheri_global_data_get()
// PTRS: ret i8* null
// PTRS: define dso_local noalias i8* @cheri_program_counter_get()
// PTRS: ret i8* null

// CAPS: define dso_local i64 @cheri_length_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.cheri.cap.length.get.i64(i8 addrspace(200)*
// CAPS: define dso_local i64 @cheri_base_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.cheri.cap.base.get.i64(i8 addrspace(200)*
// CAPS: define dso_local i64 @cheri_offset_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_offset_set(i8 addrspace(200)* readnone{{( %.+)?}}, i64 zeroext{{( %.+)?}}
// CAPS: call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*{{( %.+)?}}, i64{{( %.+)?}})
// CAPS: define dso_local signext i32 @cheri_type_get(i8 addrspace(200)*
// CAPS: call i64 @llvm.cheri.cap.type.get.i64(i8 addrspace(200)*
// CAPS: define dso_local zeroext i16 @cheri_perms_get(i8 addrspace(200)*
// CAPS: call i64 @llvm.cheri.cap.perms.get.i64(i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_perms_and(i8 addrspace(200)* readnone{{( %.+)?}}, i16 zeroext
// CAPS: call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)*{{( %.+)?}}, i64
// CAPS: define dso_local zeroext i16 @cheri_flags_get(i8 addrspace(200)*
// CAPS: call i64 @llvm.cheri.cap.flags.get.i64(i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_flags_set(i8 addrspace(200)* readnone{{( %.+)?}}, i16 zeroext
// CAPS: call i8 addrspace(200)* @llvm.cheri.cap.flags.set.i64(i8 addrspace(200)*{{( %.+)?}}, i64
// CAPS: define dso_local zeroext i1 @cheri_tag_get(i8 addrspace(200)* readnone
// CAPS: call i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)*
// CAPS: define dso_local zeroext i1 @cheri_sealed_get(i8 addrspace(200)* readnone
// CAPS: call i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_offset_increment(i8 addrspace(200)* readnone{{( %.+)?}}, i64 signext
// CAPS: %__builtin_cheri_offset_increment = getelementptr i8, i8 addrspace(200)* %__cap, i64 %__offset
// CAPS: ret i8 addrspace(200)* %__builtin_cheri_offset_increment
// CAPS: define dso_local i8 addrspace(200)* @cheri_tag_clear(i8 addrspace(200)* readnone
// CAPS: call i8 addrspace(200)* @llvm.cheri.cap.tag.clear(i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_seal(i8 addrspace(200)* readnone{{( %.+)?}}, i8 addrspace(200)* readnone
// CAPS: call i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)*{{( %.+)?}}, i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_unseal(i8 addrspace(200)* readnone{{( %.+)?}}, i8 addrspace(200)* readnone
// CAPS: call i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)*{{( %.+)?}}, i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_cap_from_pointer(i8 addrspace(200)* readnone{{( %.+)?}}, i8*{{( %.+)?}})
// CAPS: call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.i64(i8 addrspace(200)*{{( %.+)?}}, i64
// CAPS: define dso_local i8* @cheri_cap_to_pointer(i8 addrspace(200)*{{( %.+)?}}, i8 addrspace(200)*{{( %.+)?}})
// CAPS: call i64 @llvm.cheri.cap.to.pointer.i64(i8 addrspace(200)*{{( %.+)?}}, i8 addrspace(200)*
// CAPS: define dso_local void @cheri_perms_check(i8 addrspace(200)*{{( %.+)?}}, i16 zeroext
// CAPS: call void @llvm.cheri.cap.perms.check.i64(i8 addrspace(200)*{{( %.+)?}}, i64
// CAPS: define dso_local void @cheri_type_check(i8 addrspace(200)*{{( %.+)?}}, i8 addrspace(200)*
// CAPS: call void @llvm.cheri.cap.type.check(i8 addrspace(200)*{{( %.+)?}}, i8 addrspace(200)*
// CAPS: define dso_local i8 addrspace(200)* @cheri_global_data_get()
// CAPS: call i8 addrspace(200)* @llvm.cheri.ddc.get()
// CAPS: define dso_local i8 addrspace(200)* @cheri_program_counter_get()
// CAPS: call i8 addrspace(200)* @llvm.cheri.pcc.get()
