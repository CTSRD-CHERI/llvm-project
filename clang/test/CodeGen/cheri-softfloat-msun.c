// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -O0 -msoft-float | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -O1 -msoft-float | FileCheck %s -check-prefix FAIL
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -O1 -msoft-float -fno-builtin | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -O2 -msoft-float | FileCheck %s -check-prefix FAIL
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -O2 -msoft-float -fno-builtin | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -O3 -msoft-float | FileCheck %s -check-prefix FAIL
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -O3 -msoft-float -fno-builtin | FileCheck %s
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -Os -msoft-float | FileCheck %s -check-prefix FAIL
// RUN: %clang -target cheri-unknown-freebsd -o - %s -S -emit-llvm -Os -msoft-float -fno-builtin | FileCheck %s

typedef	unsigned int	fenv_t;

double	rint(double);
float	rintf(float);
long double	rintl(long double);

#ifdef __mips_soft_float
int fesetenv(const fenv_t *__envp);
int fegetenv(fenv_t *__envp);
#endif

#define	DECL(type, fn, rint)	\
type				\
fn(type x)			\
{				\
	volatile type ret;	\
	fenv_t env;		\
				\
	fegetenv(&env);		\
	ret = rint(x);		\
	fesetenv(&env);		\
	return (ret);		\
}

DECL(double, nearbyint, rint)
// CHECK-LABEL: define double @nearbyint(double %x)
// CHECK:       %call1 = call double @rint(double %{{.*}})
// FAIL: %call1 = call double @llvm.rint.f64(double %{{.*}})
DECL(float, nearbyintf, rintf)
// CHECK-LABEL: define float @nearbyintf(float %{{.*}})
// CHECK:       %call1 = call float @rintf(float %{{.*}})
// FAIL:       %call1 = call float @rintf(float %{{.*}})
DECL(long double, nearbyintl, rintl)
// CHECK-LABEL: define double @nearbyintl(double %{{.*}})
// CHECK:       %call1 = call double @rintl(double %{{.*}})
// FAIL:       %call1 = call double @rintl(double %{{.*}})
