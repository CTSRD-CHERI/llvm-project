// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s --check-prefix=CAPS
// RUN: %clang %s -O1 -target i386-unknown-freebsd -o - -emit-llvm -S | FileCheck %s --check-prefix=PTRS

// Remove the static from all of the function prototypes so that this really exists.
#define static
#define inline 
#include <memcap.h>

// PTRS: define i32 @memcap_length_get(i8* nocapture readnone %__cap)
// PTRS: ret i32 -1
// PTRS: define i8* @memcap_length_set(i8* readnone %__cap, i32  %__val)
// PTRS: ret i8* %__cap
// PTRS: define i32 @memcap_base_get(i8* nocapture readnone %__cap)
// PTRS: ret i32 -1
// PTRS: define i8* @memcap_base_increment(i8* readnone %__cap, i32  %__val)
// PTRS: ret i8* %__cap
// PTRS: define i32 @memcap_offset_get(i8* nocapture readnone %__cap)
// PTRS: ret i32 -1
// PTRS: define i8* @memcap_offset_set(i8* readnone %__cap, i32  %__val)
// PTRS: ret i8* %__cap
// PTRS: define i32 @memcap_type_get(i8* nocapture readnone %__cap)
// PTRS: ret i32 0
// PTRS: define  zeroext i16 @memcap_perms_get(i8* nocapture readnone %__cap)
// PTRS: ret i16 0
// PTRS: define i8* @memcap_perms_and(i8* readnone %__cap, i16 zeroext  %__val)
// PTRS: ret i8* %__cap
// PTRS: define zeroext i1 @memcap_tag_get(i8* nocapture readnone %__cap)
// PTRS: ret i1 false
// PTRS: define zeroext i1 @memcap_sealed_get(i8* nocapture readnone %__cap)
// PTRS: ret i1 false
// PTRS: define i8* @memcap_offset_increment(i8* readnone %__cap, i32 %__offset)
// PTRS: %add.ptr = getelementptr inbounds i8, i8* %__cap, i32 %__offset
// PTRS: ret i8* %add.ptr
// PTRS: define i8* @memcap_base_only_increment(i8* readnone %__cap, i32 %__offset)
// PTRS: ret i8* %__cap
// PTRS: define i8* @memcap_tag_clear(i8* readnone %__cap)
// PTRS: ret i8* %__cap
// PTRS: define i8* @memcap_seal(i8* readnone %__cap, i8* nocapture readnone %__type)
// PTRS: ret i8* %__cap
// PTRS: define i8* @memcap_unseal(i8* readnone %__cap, i8* nocapture readnone %__type)
// PTRS: ret i8* %__cap
// PTRS: define void @memcap_perms_check(i8* nocapture %__cap, i16 zeroext %__perms)
// PTRS: ret void
// PTRS: define void @memcap_type_check(i8* nocapture %__cap, i8* nocapture %__type)
// PTRS: ret void
// PTRS: define noalias i8* @memcap_global_data_get()
// PTRS: ret i8* null
// PTRS: define noalias i8* @memcap_program_counter_get()
// PTRS: ret i8* null

// CAPS: define i64 @memcap_length_get(i8 addrspace(200)* readnone %__cap)
// CAPS: call i64 @llvm.mips.cap.length.get(i8 addrspace(200)* %__cap)
// CAPS: define i8 addrspace(200)* @memcap_length_set(i8 addrspace(200)* readnone %__cap, i64 zeroext %__val)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.length.set(i8 addrspace(200)* %__cap, i64 %__val)
// CAPS: define i64 @memcap_base_get(i8 addrspace(200)* readnone %__cap)
// CAPS: call i64 @llvm.mips.cap.base.get(i8 addrspace(200)* %__cap)
// CAPS: define i8 addrspace(200)* @memcap_base_increment(i8 addrspace(200)* readnone %__cap, i64 zeroext %__val)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.base.increment(i8 addrspace(200)* %__cap, i64 %__val)
// CAPS: define i64 @memcap_offset_get(i8 addrspace(200)* readnone %__cap)
// CAPS: call i64 @llvm.mips.cap.offset.get(i8 addrspace(200)* %__cap)
// CAPS: define i8 addrspace(200)* @memcap_offset_set(i8 addrspace(200)* readnone %__cap, i64 zeroext %__val)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.offset.set(i8 addrspace(200)* %__cap, i64 %__val)
// CAPS: define i32 @memcap_type_get(i8 addrspace(200)* %__cap)
// CAPS: call i64 @llvm.mips.cap.type.get(i8 addrspace(200)* %__cap)
// CAPS: define zeroext i16 @memcap_perms_get(i8 addrspace(200)* %__cap)
// CAPS: call i64 @llvm.mips.cap.perms.get(i8 addrspace(200)* %__cap)
// CAPS: define i8 addrspace(200)* @memcap_perms_and(i8 addrspace(200)* readnone %__cap, i16 zeroext %__val)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.perms.and(i8 addrspace(200)* %__cap, i64 %conv)
// CAPS: define zeroext i1 @memcap_tag_get(i8 addrspace(200)* readnone %__cap)
// CAPS: call i64 @llvm.mips.cap.tag.get(i8 addrspace(200)* %__cap)
// CAPS: define zeroext i1 @memcap_sealed_get(i8 addrspace(200)* readnone %__cap)
// CAPS: call i64 @llvm.mips.cap.sealed.get(i8 addrspace(200)* %__cap)
// CAPS: define i8 addrspace(200)* @memcap_offset_increment(i8 addrspace(200)* readnone %__cap, i64 signext %__offset)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.offset.increment(i8 addrspace(200)* %__cap, i64 %__offset)
// CAPS: define i8 addrspace(200)* @memcap_base_only_increment(i8 addrspace(200)* readnone %__cap, i64 signext %__offset)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.base.only.increment(i8 addrspace(200)* %__cap, i64 %__offset)
// CAPS: define i8 addrspace(200)* @memcap_tag_clear(i8 addrspace(200)* readnone %__cap)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.tag.clear(i8 addrspace(200)* %__cap)
// CAPS: define i8 addrspace(200)* @memcap_seal(i8 addrspace(200)* readnone %__cap, i8 addrspace(200)* readnone %__type)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.seal(i8 addrspace(200)* %__cap, i8 addrspace(200)* %__type)
// CAPS: define i8 addrspace(200)* @memcap_unseal(i8 addrspace(200)* readnone %__cap, i8 addrspace(200)* readnone %__type)
// CAPS: call i8 addrspace(200)* @llvm.mips.cap.unseal(i8 addrspace(200)* %__cap, i8 addrspace(200)* %__type)
// CAPS: define void @memcap_perms_check(i8 addrspace(200)* %__cap, i16 zeroext %__perms)
// CAPS: call void @llvm.mips.cap.perms.check(i8 addrspace(200)* %__cap, i64 %conv)
// CAPS: define void @memcap_type_check(i8 addrspace(200)* %__cap, i8 addrspace(200)* %__type)
// CAPS: call void @llvm.mips.cap.type.check(i8 addrspace(200)* %__cap, i8 addrspace(200)* %__type)
// CAPS: define i8 addrspace(200)* @memcap_global_data_get()
// CAPS: call i8 addrspace(200)* @llvm.mips.c0.get()
// CAPS: define i8 addrspace(200)* @memcap_program_counter_get()
// CAPS: call i8 addrspace(200)* @llvm.mips.pcc.get()

