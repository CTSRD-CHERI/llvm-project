// RUN: %cheri_cc1 -emit-llvm %s -o - | FileCheck %s -check-prefix CHECK-HYBRID
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -emit-llvm %s -o - | %cheri_FileCheck %s -check-prefixes CHECK-PURECAP,PURECAP-LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -emit-llvm %s -o - | %cheri_FileCheck %s -check-prefixes CHECK-PURECAP,PURECAP-NEWABI
// RUN: %cheri_cc1 -S %s -o - | FileCheck %s -check-prefix HYBRID-ASM
// RUN: %cheri_purecap_cc1 -S %s -o - | %cheri_FileCheck %s -check-prefix PURECAP-ASM
// RUN: %cheri_purecap_cc1 -S %s -o - | %cheri_FileCheck %s -check-prefix PURECAP-ASM

// We were miscompiling qhooks.cpp from QtBase: even in the purecap ABI we
// were emitting .8byte directives for the qtHookData members instead of
// capability sized elements. Because of this the qtHookData symbol only
// had a size of 7*8 which resulted in length violations on access for
// CHERI256 when trying to read the RemoveQObject hook.

typedef __UINTPTR_TYPE__ quintptr;

#define QT_VERSION 0x051000
namespace QHooks {
enum HookIndex {
    HookDataVersion = 0,
    HookDataSize = 1,
    QtVersion = 2,
    AddQObject = 3,
    RemoveQObject = 4,
    Startup = 5,
    TypeInformationVersion = 6,
    LastHookIndex
};
}
quintptr qtHookData[] = {
    3, // hook data version
    QHooks::LastHookIndex, // size of qtHookData
    QT_VERSION,
    0,
    0,
    0,
    16
};

// CHECK-HYBRID: @qtHookData = global [7 x i64] [i64 3, i64 7, i64 331776, i64 0, i64 0, i64 0, i64 16], align 8
// CHECK-PURECAP: @qtHookData = addrspace(200) global [7 x i8 addrspace(200)*]
// CHECK-PURECAP-SAME: [i8 addrspace(200)* inttoptr (i64 3 to i8 addrspace(200)*),
// CHECK-PURECAP-SAME:  i8 addrspace(200)* inttoptr (i64 7 to i8 addrspace(200)*),
// CHECK-PURECAP-SAME:  i8 addrspace(200)* inttoptr (i64 331776 to i8 addrspace(200)*),
// CHECK-PURECAP-SAME:  i8 addrspace(200)* null, i8 addrspace(200)* null, i8 addrspace(200)* null,
// CHECK-PURECAP-SAME:  i8 addrspace(200)* inttoptr (i64 16 to i8 addrspace(200)*)], align [[#CAP_SIZE]]

// HYBRID-ASM-LABEL: qtHookData:
// HYBRID-ASM-NEXT: 	.8byte	3
// HYBRID-ASM-NEXT: 	.8byte	7
// HYBRID-ASM-NEXT: 	.8byte	331776
// HYBRID-ASM-NEXT: 	.8byte	0
// HYBRID-ASM-NEXT: 	.8byte	0
// HYBRID-ASM-NEXT: 	.8byte	0
// HYBRID-ASM-NEXT: 	.8byte	16
// HYBRID-ASM-NEXT: 	.size	qtHookData, 56

// PURECAP-ASM-LABEL: qtHookData:
// PURECAP-ASM-NEXT: 	.chericap	3
// PURECAP-ASM-NEXT: 	.chericap	7
// PURECAP-ASM-NEXT: 	.chericap	331776
// PURECAP-ASM-NEXT: 	.chericap	0
// PURECAP-ASM-NEXT: 	.chericap	0
// PURECAP-ASM-NEXT: 	.chericap	0
// PURECAP-ASM-NEXT: 	.chericap	16
// PURECAP-ASM-NEXT: 	.size	qtHookData, [[#CAP_SIZE * 7]]


// Some sanity check that this is actually a codegen problem and not a Sema one@
static_assert(QHooks::LastHookIndex == sizeof(qtHookData) / sizeof(qtHookData[0]), "");
static_assert(sizeof(void*) == sizeof(qtHookData[0]), "");
static_assert(sizeof(qtHookData) == QHooks::LastHookIndex * sizeof(void*), "");

#ifdef __CHERI_PURE_CAPABILITY__
static_assert(sizeof(quintptr) == sizeof(void*), "");
static_assert(sizeof(quintptr) == sizeof(__uintcap_t), "");
#else
static_assert(sizeof(quintptr) == sizeof(long), "");
#endif


// Check that it also works with an array filler
quintptr array2[4] = {
  42, (quintptr)&array2,
};

// CHECK-HYBRID: @array2 = global [4 x i64] [i64 42, i64 ptrtoint ([4 x i64]* @array2 to i64), i64 0, i64 0], align 8
// HYBRID-ASM-LABEL: array2:
// HYBRID-ASM-NEXT:	.8byte	42
// HYBRID-ASM-NEXT:	.8byte	array2
// HYBRID-ASM-NEXT:	.8byte	0
// HYBRID-ASM-NEXT:	.8byte	0
// HYBRID-ASM-NEXT:	.size	array2, 32

// CHECK-PURECAP: @array2 = addrspace(200) global [4 x i8 addrspace(200)*]
// CHECK-PURECAP-SAME: [i8 addrspace(200)* inttoptr (i64 42 to i8 addrspace(200)*),
// CHECK-PURECAP-SAME:  i8 addrspace(200)* bitcast ([4 x i8 addrspace(200)*] addrspace(200)* @array2 to i8 addrspace(200)*),
// CHECK-PURECAP-SAME:  i8 addrspace(200)* null,
// CHECK-PURECAP-SAME:  i8 addrspace(200)* null], align [[#CAP_SIZE]]
// PURECAP-ASM-LABEL: array2:
// PURECAP-ASM-NEXT:	.chericap	42
// PURECAP-ASM-NEXT:	.chericap	array2
// PURECAP-ASM-NEXT:	.chericap	0
// PURECAP-ASM-NEXT:	.chericap	0
// PURECAP-ASM-NEXT: 	.size	array2, [[#CAP_SIZE * 4]]

// Check arrays with run-time initializers:
quintptr extern_func();

quintptr array3[4] = {
  0,
  extern_func(),
  (quintptr)&extern_func,
  1234567
};

// CHECK-HYBRID: @array3 = global [4 x i64] zeroinitializer, align 8
// CHECK-HYBRID: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_qhooks_array.cpp, i8* null }]
// HYBRID-ASM-LABEL: array3:
// HYBRID-ASM-NEXT:	.space	32
// HYBRID-ASM-NEXT:	.size	array3, 32

// CHECK-PURECAP: @array3 = addrspace(200) global [4 x i8 addrspace(200)*] zeroinitializer, align [[#CAP_SIZE]]
// CHECK-PURECAP: @llvm.global_ctors = appending addrspace(200) global [1 x { i32, void ()*, i8 addrspace(200)* }] [{ i32, void ()*, i8 addrspace(200)* }
// PURECAP-LEGACY-SAME: { i32 65535, void ()* @_GLOBAL__sub_I_qhooks_array.cpp, i8 addrspace(200)* null }]
// PURECAP-NEWABI-SAME: { i32 65535,  void ()* addrspacecast (void () addrspace(200)* @_GLOBAL__sub_I_qhooks_array.cpp to void ()*), i8 addrspace(200)* null }]
// PURECAP-ASM-LABEL: array3:
// PURECAP-ASM-NEXT:	.space	[[#CAP_SIZE * 4]]
// PURECAP-ASM-NEXT: 	.size	array3, [[#CAP_SIZE * 4]]
