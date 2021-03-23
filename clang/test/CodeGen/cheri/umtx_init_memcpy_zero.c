// RUN: %cheri_purecap_cc1 -emit-llvm -o - -O0 -disable-O0-optnone %s | opt -S -mem2reg | FileCheck %s

// When generating machine code the memcpy here used to be converted into a memset zero with
// capability type. This then crashed the compiler since only integers and vectors were handled.
// The code generation test has been moved to llvm/test/CodeGen/CHERI-Generic/*/memcpy-zeroinit.ll,
// but we still check the original source here and validate that the emitted llvm.memcpy() is volatile.

#define DEFAULT_UMUTEX	{0,0,{0,0},0,0,{0,0}}

typedef __UINT32_TYPE__ __lwpid_t;
typedef __UINT32_TYPE__ __uint32_t;
typedef __UINTPTR_TYPE__ __uintptr_t;

struct umutex {
	volatile __lwpid_t	m_owner;	/* Owner of the mutex */
	__uint32_t		m_flags;	/* Flags of the mutex */
	__uint32_t		m_ceilings[2];	/* Priority protect ceiling */
	__uintptr_t		m_rb_lnk;	/* Robust linkage */
	__uint32_t		m_pad;
	__uint32_t		m_spare[2];
};
// CHECK: %struct.umutex = type { i32, i32, [2 x i32], i8 addrspace(200)*, i32, [2 x i32] }

// CHECK: @_thr_umutex_init.default_mtx = internal addrspace(200) constant %struct.umutex zeroinitializer, align 16

// CHECK-LABEL: @_thr_umutex_init(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = bitcast [[STRUCT_UMUTEX:%.*]] addrspace(200)* [[MTX:%.*]] to i8 addrspace(200)*
// CHECK-NEXT:    call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 [[TMP0]], i8 addrspace(200)* align 16 bitcast ({{.+}} addrspace(200)* @_thr_umutex_init.default_mtx to i8 addrspace(200)*),
// Note: This should be a volatile memcpy because the struct contains a volatile member.
// CHECK-SAME:     i64 48, i1 true)
// CHECK-NEXT:    ret void
//
void _thr_umutex_init(struct umutex *mtx)
{
	static const struct umutex default_mtx = DEFAULT_UMUTEX;
	*mtx = default_mtx;
}
