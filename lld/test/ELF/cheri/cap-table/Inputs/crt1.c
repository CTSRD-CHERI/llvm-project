#include <cheri_init_globals.h>


typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
#define __used __attribute__((used))
#define __unused __attribute__((unused))
#define NULL ((void*)0)
#define __hidden __attribute__((__visibility__("hidden")))
int atexit(void (*function)(void));
void _init_tls(void);
void exit(int) __attribute__((noreturn));

struct Struct_Obj_Entry;

static void _start(void *, void (*)(void), struct Struct_Obj_Entry *) __used;
extern void crt_call_constructors(void);

#include "ignore_init.c"


typedef struct _Elf_Auxinfo {
  int a_type;
  union {
      uintptr_t a_val;
      void* a_ptr;
  } a_un;
} Elf_Auxinfo;
#define AT_COUNT 10
#define AT_NULL 0
#define AT_ARGC 1
#define AT_ARGV 2
#define AT_ENVV 3

Elf_Auxinfo *__auxargs;

/* Define an assembly stub that sets up $cgp and jumps to _start */
DEFINE_CHERI_START_FUNCTION(_start)

/* The entry function, C part. This performs the bulk of program initialisation
 * before handing off to main(). It is called by __start, which is defined in
 * crt1_s.s, and necessarily written in raw assembly so that it can re-align
 * the stack before setting up the first stack frame and calling _start1().
 *
 * It would be nice to be able to hide the _start1 symbol, but that's not
 * possible, since it must be present in the GOT in order to be resolvable by
 * the position independent code in __start.
 * See: http://stackoverflow.com/questions/8095531/mips-elf-and-partial-linking
 */
static void
_start(void *auxv,
	void (*cleanup)(void),			/* from shared loader */
	struct Struct_Obj_Entry *obj __unused)	/* from shared loader */
{
	Elf_Auxinfo *aux_info[AT_COUNT];
	int i;
	int argc = 0;
	char **argv = NULL;
	char **env = NULL;
	Elf_Auxinfo *auxp;
	/* This must be called before accessing any globals! */
	cheri_init_globals();

	__auxargs = auxv;
	/* Digest the auxiliary vector. */
	for (i = 0;  i < AT_COUNT;  i++)
	    aux_info[i] = NULL;
	for (auxp = __auxargs;  auxp->a_type != AT_NULL;  auxp++) {
		if (auxp->a_type < AT_COUNT)
			aux_info[auxp->a_type] = auxp;
	}
	argc = aux_info[AT_ARGC]->a_un.a_val;
	argv = (char **)aux_info[AT_ARGV]->a_un.a_ptr;
	env = (char **)aux_info[AT_ENVV]->a_un.a_ptr;

	handle_argv(argc, argv, env);

	if (&_DYNAMIC != NULL)
		atexit(cleanup);
	else
		_init_tls();

	crt_call_constructors();

#ifdef GCRT
	/* Set up profiling support for the program, if we're being compiled
	 * with profiling support enabled (-DGCRT).
	 * See: http://sourceware.org/binutils/docs/gprof/Implementation.html
	 */
	atexit(_mcleanup);
	monstartup(&eprol, &etext);

	/* Create an 'eprol' (end of prologue?) label which delimits the start
	 * of the .text section covered by profiling. This must be before
	 * main(). */
__asm__("eprol:");
#endif

	handle_static_init(argc, argv, env);

	exit(main(argc, argv, env));
}
