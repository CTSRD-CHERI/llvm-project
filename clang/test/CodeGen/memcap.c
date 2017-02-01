// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s --check-prefix=CAPS
// RUN: %clang %s -O1 -target i386-unknown-freebsd -o - -emit-llvm -S | FileCheck %s --check-prefix=PTRS

// Remove the static from all of the function prototypes so that this really exists.
#define static
#define inline 
#include <memcap.h>

// PTRS: define i32 @memcap_length_get(i8* nocapture readnone
// PTRS: ret i32 -1
// PTRS: define i32 @memcap_base_get(i8* nocapture readnone
// PTRS: ret i32 -1
// PTRS: define i32 @memcap_offset_get(i8* nocapture readnone
// PTRS: ret i32 -1
// PTRS: define i8* @memcap_offset_set(i8* readnone returned{{.*}}, i32
// PTRS: ret i8*
// PTRS: define i32 @memcap_type_get(i8* nocapture readnone
// PTRS: ret i32 0
// PTRS: define  zeroext i16 @memcap_perms_get(i8* nocapture readnone
// PTRS: ret i16 0
// PTRS: define i8* @memcap_perms_and(i8* readnone returned{{.*}}, i16 zeroext
// PTRS: ret i8*
// PTRS: define zeroext i1 @memcap_tag_get(i8* nocapture readnone
// PTRS: ret i1 false
// PTRS: define zeroext i1 @memcap_sealed_get(i8* nocapture readnone
// PTRS: ret i1 false
// PTRS: define i8* @memcap_offset_increment(i8* readnone{{.*}}, i32
// PTRS: %[[TEMP1:[0-9a-z.]+]] = getelementptr inbounds i8, i8*{{.*}}, i32
// PTRS: ret i8* %[[TEMP1]]
// PTRS: define i8* @memcap_tag_clear(i8* readnone returned
// PTRS: ret i8*
// PTRS: define i8* @memcap_seal(i8* readnone returned{{.*}}, i8* nocapture readnone
// PTRS: ret i8*
// PTRS: define i8* @memcap_unseal(i8* readnone returned{{.*}}, i8* nocapture readnone
// PTRS: ret i8*
// PTRS: define void @memcap_perms_check(i8* nocapture{{.*}}, i16 zeroext
// PTRS: ret void
// PTRS: define void @memcap_type_check(i8* nocapture{{.*}}, i8* nocapture
// PTRS: ret void
// PTRS: define noalias i8* @memcap_global_data_get()
// PTRS: ret i8* null
// PTRS: define noalias i8* @memcap_program_counter_get()
// PTRS: ret i8* null

// CAPS: define i64 @memcap_length_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.memcap.cap.length.get(i8 addrspace(200)*
// CAPS: define i64 @memcap_base_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.memcap.cap.base.get(i8 addrspace(200)*
// CAPS: define i64 @memcap_offset_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.memcap.cap.offset.get(i8 addrspace(200)*
// CAPS: define i8 addrspace(200)* @memcap_offset_set(i8 addrspace(200)* readnone{{.*}}, i64 zeroext{{.*}}
// CAPS: call i8 addrspace(200)* @llvm.memcap.cap.offset.set(i8 addrspace(200)*{{.*}}, i64{{.*}})
// CAPS: define i32 @memcap_type_get(i8 addrspace(200)*
// CAPS: call i64 @llvm.memcap.cap.type.get(i8 addrspace(200)*
// CAPS: define zeroext i16 @memcap_perms_get(i8 addrspace(200)*
// CAPS: call i64 @llvm.memcap.cap.perms.get(i8 addrspace(200)*
// CAPS: define i8 addrspace(200)* @memcap_perms_and(i8 addrspace(200)* readnone{{.*}}, i16 zeroext
// CAPS: call i8 addrspace(200)* @llvm.memcap.cap.perms.and(i8 addrspace(200)*{{.*}}, i64
// CAPS: define zeroext i1 @memcap_tag_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.memcap.cap.tag.get(i8 addrspace(200)*
// CAPS: define zeroext i1 @memcap_sealed_get(i8 addrspace(200)* readnone
// CAPS: call i64 @llvm.memcap.cap.sealed.get(i8 addrspace(200)*
// CAPS: define i8 addrspace(200)* @memcap_offset_increment(i8 addrspace(200)* readnone{{.*}}, i64 signext
// CAPS: call i8 addrspace(200)* @llvm.memcap.cap.offset.increment(i8 addrspace(200)*{{.*}}, i64
// CAPS: define i8 addrspace(200)* @memcap_tag_clear(i8 addrspace(200)* readnone
// CAPS: call i8 addrspace(200)* @llvm.memcap.cap.tag.clear(i8 addrspace(200)*
// CAPS: define i8 addrspace(200)* @memcap_seal(i8 addrspace(200)* readnone{{.*}}, i8 addrspace(200)* readnone
// CAPS: call i8 addrspace(200)* @llvm.memcap.cap.seal(i8 addrspace(200)*{{.*}}, i8 addrspace(200)*
// CAPS: define i8 addrspace(200)* @memcap_unseal(i8 addrspace(200)* readnone{{.*}}, i8 addrspace(200)* readnone
// CAPS: call i8 addrspace(200)* @llvm.memcap.cap.unseal(i8 addrspace(200)*{{.*}}, i8 addrspace(200)*
// CAPS: define void @memcap_perms_check(i8 addrspace(200)*{{.*}}, i16 zeroext
// CAPS: call void @llvm.memcap.cap.perms.check(i8 addrspace(200)*{{.*}}, i64
// CAPS: define void @memcap_type_check(i8 addrspace(200)*{{.*}}, i8 addrspace(200)*
// CAPS: call void @llvm.memcap.cap.type.check(i8 addrspace(200)*{{.*}}, i8 addrspace(200)*
// CAPS: define i8 addrspace(200)* @memcap_global_data_get()
// CAPS: call i8 addrspace(200)* @llvm.memcap.ddc.get()
// CAPS: define i8 addrspace(200)* @memcap_program_counter_get()
// CAPS: call i8 addrspace(200)* @llvm.memcap.pcc.get()

