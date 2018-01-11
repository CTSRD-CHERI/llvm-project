// RUN: %clang_cc1 -emit-llvm -triple mips64-unknown-freebsd12.0 -mrelocation-model pic -pic-level 2 -target-cpu mips4 \
// RUN: -target-feature -noabicalls -target-abi n64 -mllvm -mips-ssection-threshold=0 -sys-header-deps -O2 -std=gnu99 \
// RUN: -cheri-linker -x c -o - -Werror=implicit-int %s | FileCheck %s -check-prefix N64
// RUN: %cheri_purecap_cc1 -emit-llvm -target-cpu mips4 -target-feature -noabicalls -mllvm -mips-ssection-threshold=0 \
// RUN: -sys-header-deps -D NEED_SOLARIS_BOOLEAN -O2 -std=gnu99 -cheri-linker -x c -o - -Werror=implicit-int %s | FileCheck %s -check-prefix PURECAP
// This was causing crashes in cap-table mode

typedef struct { long a; } b;
b c = {(long)"123"};


// N64: @.str = private unnamed_addr constant [4 x i8] c"123\00", align 1
// N64: @c = local_unnamed_addr global %struct.b { i64 ptrtoint ([4 x i8]* @.str to i64) }, align 8

// PURECAP: @.str = private unnamed_addr addrspace(200) constant [4 x i8] c"123\00", align 1
// PURECAP: @c = local_unnamed_addr addrspace(200) global %struct.b { i64 ptrtoint ([4 x i8] addrspace(200)* @.str to i64) }, align 8





