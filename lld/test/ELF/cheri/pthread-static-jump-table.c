// RUN: rm -rf %t
// RUN: mkdir %t
// RUN: %cheri_purecap_cc1 %s -emit-obj -o %t/libc-stubs.o -DLIBC_STUBS
// RUN: llvm-objdump -d -t %t/libc-stubs.o
// RUN: %cheri_purecap_cc1 %s -emit-obj -o %t/libc-exit.o -DLIBC_EXIT
// RUN: llvm-ar cq %t/libc.a %t/libc-stubs.o %t/libc-exit.o
// RUN: %cheri_purecap_cc1 %s -emit-obj -o %t/libthr-table.o -DLIBTHR_TABLE
// RUN: %cheri_purecap_cc1 %s -emit-obj -o %t/libthr-impls.o -DLIBTHR_IMPLS -O2
// RUN: llvm-ar cq %t/libthr.a %t/libthr-table.o %t/libthr-impls.o
// RUN: llvm-objdump -d -t %t/libthr.a

// RUN: %cheri_purecap_cc1 %s -emit-obj -o %t/main.o -DEXE
// RUN: %cheri_purecap_cc1 %s -emit-obj -o %t/main.o -DEXE
// RUN: ld.lld --verbose -pie %t/main.o -o %t/thr-first.exe %t/libthr.a %t/libc.a
// RUN: ld.lld --verbose -pie %t/main.o -o %t/libc-first.exe %t/libc.a %t/libthr.a

// RUN: llvm-objdump -t %t/thr-first.exe | FileCheck %s -check-prefix USES-LIBTHR-SYMBOLS
// RUN: llvm-objdump -t %t/libc-first.exe | FileCheck %s -check-prefix USES-LIBC-SYMBOLS

// PTHREAD symbols are a single instruction in the libthr version:

// USES-LIBTHR-SYMBOLS: SYMBOL TABLE:
// USES-LIBTHR-SYMBOLS-DAG: gw    F .text		 00000004 pthread_exit
// USES-LIBTHR-SYMBOLS-DAG: g     F .text		 00000004 _pthread_exit
// USES-LIBTHR-SYMBOLS-DAG: g     F .text		 00000004 _pthread_join
// USES-LIBTHR-SYMBOLS-DAG: g     F .text		 00000004 _pthread_kill
// USES-LIBTHR-SYMBOLS-DAG: g     F .text		 00000004 _pthread_once
// USES-LIBTHR-SYMBOLS-DAG: gw    F .text		 00000004 pthread_join
// USES-LIBTHR-SYMBOLS-DAG: gw    F .text		 00000004 pthread_kill
// USES-LIBTHR-SYMBOLS-DAG: gw    F .text		 00000004 pthread_once

// Static linking of libc and libthr must be done in the right order otherwise we get infinite loops
// FIXME: can we warn about this?
// USES-LIBC-SYMBOLS: SYMBOL TABLE:
// USES-LIBC-SYMBOLS: gw    F .text		 00000058 pthread_exit


#ifndef EXE
// namespace.h:
#define		pthread_exit			_pthread_exit
#define		pthread_join			_pthread_join
#define		pthread_kill			_pthread_kill
#define		pthread_once			_pthread_once
#endif

/*
 * Indexes into the pthread jump table.
 *
 * Warning! If you change this type, you must also change the threads
 * libraries that reference it (libc_r, libpthread).
 */
typedef enum {
	PJT_EXIT,
	PJT_JOIN,
	PJT_KILL,
	PJT_ONCE,
	PJT_MAX
} pjt_index_t;

#define __used __attribute__((used))
#define	__CONCAT1(x,y)	x ## y
#define	__CONCAT(x,y)	__CONCAT1(x,y)
#define	__STRING(x)	#x		/* stringify without expanding x */
#define	__XSTRING(x)	__STRING(x)	/* expand x, then stringify */
#ifdef __STDC__
#define	__weak_reference(sym,alias)	\
	__asm__(".weak " #alias);	\
	__asm__(".equ "  #alias ", " #sym)
#define	__warn_references(sym,msg)	\
	__asm__(".section .gnu.warning." #sym);	\
	__asm__(".asciz \"" msg "\"");	\
	__asm__(".previous")
#define	__sym_compat(sym,impl,verid)	\
	__asm__(".symver " #impl ", " #sym "@" #verid)
#define	__sym_default(sym,impl,verid)	\
	__asm__(".symver " #impl ", " #sym "@@@" #verid)
#else
#define	__weak_reference(sym,alias)	\
	__asm__(".weak alias");		\
	__asm__(".equ alias, sym")
#define	__warn_references(sym,msg)	\
	__asm__(".section .gnu.warning.sym"); \
	__asm__(".asciz \"msg\"");	\
	__asm__(".previous")
#define	__sym_compat(sym,impl,verid)	\
	__asm__(".symver impl, sym@verid")
#define	__sym_default(impl,sym,verid)	\
	__asm__(".symver impl, sym@@@verid")
#endif	/* __STDC__ */

#define NULL ((void*)0)
#define ENOSYS 44

typedef int (*pthread_func_t)(void);
typedef pthread_func_t pthread_func_entry_t[2];
typedef __UINT32_TYPE__ __uint32_t;

extern pthread_func_entry_t __thr_jtable[];

struct pthread;		/* XXX */
typedef struct pthread *__pthread_t;
typedef __pthread_t pthread_t;
#define	_SIG_WORDS	4
#define	_SIG_MAXSIG	128
#define	_SIG_IDX(sig)	((sig) - 1)
#define	_SIG_WORD(sig)	(_SIG_IDX(sig) >> 5)
#define	_SIG_BIT(sig)	(1 << (_SIG_IDX(sig) & 31))
#define	_SIG_VALID(sig)	((sig) <= _SIG_MAXSIG && (sig) > 0)
typedef struct __sigset {
	__uint32_t __bits[_SIG_WORDS];
} __sigset_t;
typedef	__sigset_t	sigset_t;


void		pthread_exit(void *) __attribute__((noreturn));
int	pthread_kill(__pthread_t, int);
int		pthread_join(pthread_t, void **);
typedef struct	pthread_once		pthread_once_t;
int		pthread_once(pthread_once_t *, void (*) (void));

int	pthread_sigmask(int, const __sigset_t * __restrict,
	    __sigset_t * __restrict);

void exit(int status) __attribute__((noreturn));


#ifndef EXE
// un-namespace.h:
#undef pthread_exit
#undef pthread_join
#undef pthread_kill
#undef pthread_once
#endif

#ifdef LIBC_STUBS
/*
 * Weak symbols: All libc internal usage of these functions should
 * use the weak symbol versions (_pthread_XXX).  If libpthread is
 * linked, it will override these functions with (non-weak) routines.
 * The _pthread_XXX functions are provided solely for internal libc
 * usage to avoid unwanted cancellation points and to differentiate
 * between application locks and libc locks (threads holding the
 * latter can't be allowed to exit/terminate).
 */

/* Define a null pthread structure just to satisfy _pthread_self. */
struct pthread {
};

static struct pthread	main_thread;

static int		stub_main(void);
static void 		*stub_null(void);
static struct pthread	*stub_self(void);
static int		stub_zero(void);
static int		stub_fail(void);
static int		stub_true(void);
static void		stub_exit(void);

#define	PJT_DUAL_ENTRY(entry)	\
	(pthread_func_t)entry, (pthread_func_t)entry

pthread_func_entry_t __thr_jtable[PJT_MAX] = {
	{PJT_DUAL_ENTRY(stub_exit)},    /* PJT_EXIT */
	{PJT_DUAL_ENTRY(stub_zero)},    /* PJT_JOIN */
	{PJT_DUAL_ENTRY(stub_zero)},    /* PJT_KILL */
	{PJT_DUAL_ENTRY(stub_fail)},    /* PJT_ONCE */
};

/*
 * Weak aliases for exported (pthread_*) and internal (_pthread_*) routines.
 */
#define	WEAK_REF(sym, alias)	__weak_reference(sym, alias)

#define	FUNC_TYPE(name)		__CONCAT(name, _func_t)
#define	FUNC_INT(name)		__CONCAT(name, _int)
#define	FUNC_EXP(name)		__CONCAT(name, _exp)

#define	STUB_FUNC(name, idx, ret)				\
	static ret FUNC_EXP(name)(void) __used;			\
	static ret FUNC_INT(name)(void) __used;			\
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(void);			\
	static ret FUNC_EXP(name)(void)				\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func());				\
	}							\
	static ret FUNC_INT(name)(void)				\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func());				\
	}

#define	STUB_FUNC1(name, idx, ret, p0_type)			\
	static ret FUNC_EXP(name)(p0_type) __used;		\
	static ret FUNC_INT(name)(p0_type) __used;		\
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(p0_type);		\
	static ret FUNC_EXP(name)(p0_type p0)			\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func(p0));				\
	}							\
	static ret FUNC_INT(name)(p0_type p0)			\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func(p0));				\
	}

#define	STUB_FUNC2(name, idx, ret, p0_type, p1_type)		\
	static ret FUNC_EXP(name)(p0_type, p1_type) __used;	\
	static ret FUNC_INT(name)(p0_type, p1_type) __used;	\
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(p0_type, p1_type);	\
	static ret FUNC_EXP(name)(p0_type p0, p1_type p1)	\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func(p0, p1));				\
	}							\
	static ret FUNC_INT(name)(p0_type p0, p1_type p1)	\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func(p0, p1));				\
	}

#define	STUB_FUNC3(name, idx, ret, p0_type, p1_type, p2_type)	\
	static ret FUNC_EXP(name)(p0_type, p1_type, p2_type) __used; \
	static ret FUNC_INT(name)(p0_type, p1_type, p2_type) __used; \
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(p0_type, p1_type, p2_type); \
	static ret FUNC_EXP(name)(p0_type p0, p1_type p1, p2_type p2) \
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func(p0, p1, p2));			\
	}							\
	static ret FUNC_INT(name)(p0_type p0, p1_type p1, p2_type p2) \
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func(p0, p1, p2));			\
	}
STUB_FUNC2(pthread_once, PJT_ONCE, int, void *, void *)
STUB_FUNC1(pthread_exit, PJT_EXIT, void, void *)
STUB_FUNC2(pthread_join, PJT_JOIN, int, void *, void *)
STUB_FUNC2(pthread_kill, PJT_KILL, int, void *, int)

static int
stub_zero(void)
{
	return (0);
}

static void *
stub_null(void)
{
	return (NULL);
}

static struct pthread *
stub_self(void)
{
	return (&main_thread);
}

static int
stub_fail(void)
{
	return (ENOSYS);
}

static int
stub_main(void)
{
	return (-1);
}

static int
stub_true(void)
{
	return (1);
}

static void
stub_exit(void)
{
	exit(0);
}

int link_libc_table = 0;

#endif

#ifdef LIBC_EXIT
void
exit(int status)
{
	/* Ensure that the auto-initialization routine is linked in: */
	extern int _thread_autoinit_dummy_decl;

	_thread_autoinit_dummy_decl = 1;

	__builtin_trap();
}
#endif

#ifdef LIBTHR_TABLE
# define STATIC_LIB_REQUIRE(name) __asm (".globl " #name)
/*
 * All weak references used within libc should be in this table.
 * This is so that static libraries will work.
 */
STATIC_LIB_REQUIRE(_thread_init_hack);

/*
 * These are needed when linking statically.  All references within
 * libgcc (and in the future libc) to these routines are weak, but
 * if they are not (strongly) referenced by the application or other
 * libraries, then the actual functions will not be loaded.
 */
STATIC_LIB_REQUIRE(_pthread_once);
STATIC_LIB_REQUIRE(_pthread_create);

extern typeof(_pthread_exit) libthr_pthread_exit;
extern typeof(_pthread_join) libthr_pthread_join;
extern typeof(_pthread_kill) libthr_pthread_kill;
extern typeof(_pthread_once) libthr_pthread_once;

#define	DUAL_ENTRY(entry)	\
	(pthread_func_t)libthr##entry, (pthread_func_t)libthr##entry

// FIXME: this seems broken in lld -> it resolves to the weak alias stub functions!!!!
static pthread_func_t jmp_table[][2] = {
	{DUAL_ENTRY(_pthread_exit)},	/* PJT_EXIT */
	{DUAL_ENTRY(_pthread_join)},	/* PJT_JOIN */
	{DUAL_ENTRY(_pthread_kill)},	/* PJT_KILL */
	{DUAL_ENTRY(_pthread_once)},		/* PJT_ONCE */
};

static int init_once = 0;

/*
 * For the shared version of the threads library, the above is sufficient.
 * But for the archive version of the library, we need a little bit more.
 * Namely, we must arrange for this particular module to be pulled in from
 * the archive library at link time.  To accomplish that, we define and
 * initialize a variable, "_thread_autoinit_dummy_decl".  This variable is
 * referenced (as an extern) from libc/stdlib/exit.c. This will always
 * create a need for this module, ensuring that it is present in the
 * executable.
 */
extern int _thread_autoinit_dummy_decl;
int _thread_autoinit_dummy_decl = 0;

void
_libpthread_init(struct pthread *curthread)
{
	if (sizeof(jmp_table) != sizeof(pthread_func_t) * PJT_MAX * 2)
		__builtin_trap();
	__builtin_memcpy(__thr_jtable, jmp_table, sizeof(jmp_table));
	// __thr_interpose_libc();
}

#endif


#ifdef LIBTHR_IMPLS
#define	__strong_reference(sym,aliassym)	\
	extern __typeof (sym) aliassym __attribute__ ((__alias__ (#sym)))

__weak_reference(_pthread_exit, pthread_exit);
void _pthread_exit(void *status) { __asm__ volatile ("li $zero, 0x1234"); __builtin_unreachable(); }
__strong_reference(_pthread_exit, libthr_pthread_exit);

__weak_reference(_pthread_kill, pthread_kill);
int _pthread_kill(pthread_t pthread, int sig) { __asm__ volatile ("li $zero, 0x1235"); __builtin_unreachable(); }
__strong_reference(_pthread_kill, libthr_pthread_kill);

__weak_reference(_pthread_once, pthread_once);
int _pthread_once(pthread_once_t *once_control, void (*init_routine) (void)) { __asm__ volatile ("li $zero, 0x1236"); __builtin_unreachable(); }
__strong_reference(_pthread_once, libthr_pthread_once);

__weak_reference(_pthread_join, pthread_join);
int _pthread_join(pthread_t pthread, void **thread_return) { __asm__ volatile ("li $zero, 0x1237"); __builtin_unreachable(); }
__strong_reference(_pthread_join, libthr_pthread_join);

#endif

#ifdef EXE

void __start(void) {
  int status;
  extern int link_libc_table;
  link_libc_table = 0;
  pthread_exit(&status);
  exit(1);

}
#endif
