// REQUIRES: mips-registered-target

// RUN: %cheri_cc1 -Wcheri-pointer-conversion -S %s -o - -O0 -verify | FileCheck %s -implicit-check-not ctoptr
// don't match the ctoptr in the .file directive
// CHECK: .file "{{.+}}"

#define __kerncap __capability

typedef	void __sighandler_t(int);
typedef	__sighandler_t	* __kerncap sig_t;	/* type of pointer to a signal function */
#define __SIGHANDLER_CONSTANT(value)	\
    ((__sighandler_t * __kerncap)(__intcap_t)value)
#define	SIG_DFL		__SIGHANDLER_CONSTANT(0)
#define	SIG_IGN		__SIGHANDLER_CONSTANT(1)
#define	SIG_ERR		__SIGHANDLER_CONSTANT(-1)
/* #define	SIG_CATCH	__SIGHANDLER_CONSTANT(2) See signalvar.h */
#define SIG_HOLD        __SIGHANDLER_CONSTANT(3)
// See kern_sig.c
#define	SIG_DFL_N	(void *)0
#define	SIG_IGN_N	(void *)1

typedef long sigset_t;
struct __siginfo;

#if __has_feature(capabilities)
struct sigaction_c {
	union {
		void    (* __capability __sa_handler)(int);
		void    (* __capability __sa_sigaction)
			    (int, struct __siginfo *, void *);
	} __sigaction_u;		/* signal handler */
	int	sa_flags;		/* see signal options below */
	sigset_t sa_mask;		/* signal mask to apply */
};
#endif
struct sigaction_native {
	union {
		void    (*__sa_handler)(int);
		void    (*__sa_sigaction)(int, struct __siginfo *, void *);
	} __sigaction_u;		/* signal handler */
	int	sa_flags;		/* see signal options below */
	sigset_t sa_mask;		/* signal mask to apply */
};

#if __has_feature(capabilities)
typedef struct sigaction_c	ksigaction_t;
#else
typedef	struct sigaction_native	ksigaction_t;
#endif

#define	sa_handler	__sigaction_u.__sa_handler
#define	sa_sigaction	__sigaction_u.__sa_sigaction

#define	SA_NOCLDSTOP	0x0008	/* do not generate SIGCHLD on child stop */
#define	SA_ONSTACK	0x0001	/* take signal on signal stack */
#define	SA_RESTART	0x0002	/* restart system call on signal return */
#define	SA_RESETHAND	0x0004	/* reset to SIG_DFL when taking signal */
#define	SA_NODEFER	0x0010	/* don't mask the signal we're delivering */
#define	SA_NOCLDWAIT	0x0020	/* don't keep zombies around */
#define	SA_SIGINFO	0x0040	/* signal handler with SA_SIGINFO args */

typedef _Bool bool;
bool
sigact_flag_test(const ksigaction_t *act, int flag)
{

	/*
	 * SA_SIGINFO is reset when signal disposition is set to
	 * ignore or default.  Other flags are kept according to user
	 * settings.
	 */
	return ((act->sa_flags & flag) != 0 && (flag != SA_SIGINFO ||
	    ((__sighandler_t * __capability)act->sa_sigaction != SIG_IGN && (__sighandler_t * __capability)act->sa_sigaction != SIG_DFL)));
}

bool test(const struct sigaction_c *act, const struct sigaction_native* act_n) {
  return (__sighandler_t * __capability) act->sa_sigaction ==
         (__sighandler_t * __capability) act_n->sa_sigaction;
}

bool test2(const struct sigaction_c *act, const struct sigaction_native* act_n) {
  return (__sighandler_t * __capability) act->sa_handler ==
         (__sighandler_t * __capability) act_n->sa_handler;
}

__sighandler_t * __capability test3(const struct sigaction_c *act, const struct sigaction_native* act_n) {
  return act_n->sa_handler; // expected-warning{{converting non-capability type 'void (*)(int)' to capability type 'void (* __capability)(int)' without an explicit cast; if this is intended use __cheri_tocap}}
}
