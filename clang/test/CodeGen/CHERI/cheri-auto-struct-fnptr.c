// RUN: %cheri_purecap_cc1 -cheri-linker "-emit-llvm" "-o" "-" %s | FileCheck %s
int nis_setgrent(void);
typedef int (*nss_method)(void);
typedef struct _ns_dtab {
 const char *src;
 nss_method method;
} ns_dtab;

void compat_setgrent(void)
{
 // Check that initialisation of this is done via a memcpy from something that
 // the linker will have set up correctly, and not by a simple store of the
 // constant.
 // CHECK: memcpy
 ns_dtab dtab[] = {
  { 0, nis_setgrent }
 };
}

