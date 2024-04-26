// REQUIRES: clang, mips
// RUN: %cheri_cc1 -emit-obj -O2 -cheri-size 256 -target-cpu cheri256 -target-feature +soft-float \
// RUN:   -msoft-float -target-abi purecap -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUN: ld.lld %t.o %S/Inputs/interposing_table.o  -o %t.exe
// RUN: llvm-objdump -t --cap-relocs %t.exe | FileCheck %s

#define SLOT_SYS(name) int __sys_##name(void) { return 0; }
#define SLOT_LIBC(name) int __libc_##name(void) { return 0; }
#define SLOT(unused, name) int name(void) { return 0; }

SLOT_SYS(accept)
SLOT_SYS(accept4)
SLOT_SYS(aio_suspend)
SLOT_SYS(close)
SLOT_SYS(connect)
SLOT_SYS(fcntl)
SLOT_SYS(fsync)
SLOT_SYS(fork)
SLOT_SYS(msync)
SLOT_SYS(nanosleep)
SLOT_SYS(openat)
SLOT_SYS(poll)
SLOT_SYS(pselect)
SLOT_SYS(read)
SLOT_SYS(readv)
SLOT_SYS(recvfrom)
SLOT_SYS(recvmsg)
SLOT_SYS(select)
SLOT_SYS(sendmsg)
SLOT_SYS(sendto)
SLOT_SYS(setcontext)
SLOT_SYS(sigaction)
SLOT_SYS(sigprocmask)
SLOT_SYS(sigsuspend)
SLOT_LIBC(sigwait)
SLOT_SYS(sigtimedwait)
SLOT_SYS(sigwaitinfo)
SLOT_SYS(swapcontext)
SLOT_LIBC(system)
SLOT_LIBC(tcdrain)
SLOT_SYS(wait4)
SLOT_SYS(write)
SLOT_SYS(writev)
SLOT(_pthread_mutex_init_calloc_cb, _pthread_mutex_init_calloc_cb_stub)
SLOT(spinlock, __libc_spinlock_stub)
SLOT(spinunlock, __libc_spinunlock_stub)
SLOT_SYS(kevent)
SLOT_SYS(wait6)
SLOT_SYS(ppoll)
SLOT_LIBC(map_stacks_exec)
SLOT_SYS(fdatasync)
SLOT_SYS(clock_nanosleep)

typedef int (*interpos_func_t)(void);
interpos_func_t *__libc_interposing_slot(int interposno);
extern interpos_func_t __libc_interposing[] __attribute__((visibility("hidden")));

int __start(void) {
  return __libc_interposing[1] != (void*)0;
}

// CHECK: SYMBOL TABLE:
// CHECK: 0000000[[libc_interposing_addr:[0-9a-f]+]] l     O .data		 0000000000000540 .hidden __libc_interposing
// CHECK: CAPABILITY RELOCATION RECORDS:
// CHECK: 0x0000000[[libc_interposing_addr]]	Base: __wrap_accept (0x0000000000021020)	Offset: 0x0000000000000000	Length: 0x0000000000000038	Permissions: 0x8000000000000000 (Function)
