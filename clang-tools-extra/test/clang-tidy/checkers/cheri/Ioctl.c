// RUN: %check_clang_tidy %s cheri-Ioctl %t

#ifdef __CHERI__
typedef __uintcap_t user_uintptr_t;
#else
typedef unsigned long user_uintptr_t;
#endif

typedef long (*ioctl_t) (int fd, unsigned long, user_uintptr_t);
struct file_operations {
	long (*unlocked_ioctl) (int fd, unsigned long, user_uintptr_t);
};

struct container {
    struct file_operations ops;
};

struct other {
	long (*unlocked_ioctl) (int fd, unsigned long, user_uintptr_t);
};

struct media_file_operations {
        ioctl_t ioctl;
};

extern long smc_ioctl(int fd, unsigned long cmd, unsigned long);
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}}: warning: CHERI: Parameter 3 of ioctl function 'smc_ioctl' should have type 'user_uintptr_t' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}}: warning: CHERI: Replace type with 'user_uintptr_t' [cheri-Ioctl]
extern long other_ioctl(int fd, unsigned long cmd, unsigned long);

extern long smc_ioctl(int fd, unsigned long cmd, unsigned long arg);
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Parameter 3 of ioctl function 'smc_ioctl' should have type 'user_uintptr_t' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: CHERI: Replace type with 'user_uintptr_t' [cheri-Ioctl]
typeof(smc_ioctl) smc_ioctl;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Parameter 3 of ioctl function 'smc_ioctl' should have type 'user_uintptr_t' [cheri-Ioctl]

static struct file_operations ops[] =
{
	{ .unlocked_ioctl = smc_ioctl },
	{ .unlocked_ioctl = other_ioctl },
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Initialization of ioctl field 'unlocked_ioctl' in 'file_operations' with non-ioctl function 'other_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("other_ioctl", 2) [cheri-Ioctl]
	{ smc_ioctl },
	{ other_ioctl },
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Initialization of ioctl field 'unlocked_ioctl' in 'file_operations' with non-ioctl function 'other_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("other_ioctl", 2) [cheri-Ioctl]
};

static struct other other_ops[] = {
	{ .unlocked_ioctl = smc_ioctl },
	{ .unlocked_ioctl = other_ioctl },
	{ smc_ioctl },
	{ other_ioctl },
};

long
smc_ioctl(int fd, unsigned long cmd, unsigned long arg)
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Parameter 3 of ioctl function 'smc_ioctl' should have type 'user_uintptr_t' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: CHERI: Replace type with 'user_uintptr_t' [cheri-Ioctl]
{
	ops[1].unlocked_ioctl(fd, cmd, arg);
	other_ops[1].unlocked_ioctl(fd, cmd, arg);
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Ioctl pointer parameter 'arg' of ioctl function 'smc_ioctl' used as argument 3 in call to non-ioctl function pointed to by field 'unlocked_ioctl' in 'other' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addField("other", "unlocked_ioctl", 2) [cheri-Ioctl]
	smc_ioctl(fd, cmd, arg);
	ops->unlocked_ioctl = smc_ioctl;
	ops->unlocked_ioctl = other_ioctl;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Initialization of ioctl field 'unlocked_ioctl' in 'file_operations' with non-ioctl function 'other_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("other_ioctl", 2) [cheri-Ioctl]
	other_ops->unlocked_ioctl = smc_ioctl;
	other_ops->unlocked_ioctl = other_ioctl;

	smc_ioctl(fd, arg, cmd);
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Ioctl pointer parameter 'arg' of ioctl function 'smc_ioctl' used as argument 2 in call to non-ioctl function 'smc_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("smc_ioctl", 1) [cheri-Ioctl]
	ops[1].unlocked_ioctl(fd, arg, cmd);
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Ioctl pointer parameter 'arg' of ioctl function 'smc_ioctl' used as argument 2 in call to non-ioctl function pointed to by field 'unlocked_ioctl' in 'file_operations' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addField("file_operations", "unlocked_ioctl", 1) [cheri-Ioctl]
	return 0;
}

long
other_ioctl(int fd, unsigned long cmd, unsigned long arg)
{
	ops[1].unlocked_ioctl(fd, cmd, arg);
	other_ops[1].unlocked_ioctl(fd, cmd, arg);
	smc_ioctl(fd, cmd, arg);
	other_ioctl(fd, cmd, arg);
	ops->unlocked_ioctl = smc_ioctl;
	ops->unlocked_ioctl = other_ioctl;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Initialization of ioctl field 'unlocked_ioctl' in 'file_operations' with non-ioctl function 'other_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("other_ioctl", 2) [cheri-Ioctl]
	other_ops->unlocked_ioctl = smc_ioctl;
	other_ops->unlocked_ioctl = other_ioctl;
	return 0;
}

struct file_operations fops_global;

long init(struct container *c, struct file_operations *f)
{
    c->ops.unlocked_ioctl = smc_ioctl;
    f->unlocked_ioctl = smc_ioctl;
    fops_global.unlocked_ioctl = smc_ioctl;
    c->ops.unlocked_ioctl = other_ioctl;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Initialization of ioctl field 'unlocked_ioctl' in 'file_operations' with non-ioctl function 'other_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("other_ioctl", 2) [cheri-Ioctl]
    f->unlocked_ioctl = other_ioctl;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Initialization of ioctl field 'unlocked_ioctl' in 'file_operations' with non-ioctl function 'other_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("other_ioctl", 2) [cheri-Ioctl]
    fops_global.unlocked_ioctl = other_ioctl;
// CHECK-MESSAGES: :[[@LINE-1]]:{{.*}} warning: CHERI: Initialization of ioctl field 'unlocked_ioctl' in 'file_operations' with non-ioctl function 'other_ioctl' [cheri-Ioctl]
// CHECK-MESSAGES: :[[@LINE-2]]:{{.*}} warning: MISSING: addFunc("other_ioctl", 2) [cheri-Ioctl]
}
