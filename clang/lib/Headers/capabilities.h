#include <stdint.h>

typedef unsigned short capperms_t;
#if __has_feature(capabilities)
typedef __intcap_t intcap_t;
typedef __uintcap_t uintcap_t;
#define capability __capability
#define output __output
static inline capability void* cap_set_length(capability void* __cap, size_t __size)
{
	return __builtin_cheri_set_cap_length(__cap, __size);
}
static inline size_t cap_get_length(capability void* __cap)
{
	return __builtin_cheri_get_cap_length(__cap);
}
static inline capability void* cap_and_permissions(capability void* __cap,
                                                   capperms_t __perms)
{
	return __builtin_cheri_and_cap_perms(__cap, __perms);
}
static inline capperms_t cap_get_perms(capability void* __cap)
{
	return __builtin_cheri_get_cap_perms(__cap);
}
static inline
#ifdef __cplusplus
bool
#else
_Bool
#endif
cap_is_valid(capability void* __cap)
{
	return __builtin_cheri_get_cap_tag(__cap);
}
#else // no capability support in the target
typedef intptr_t intcap_t;
typedef uintptr_t uintcap_t;
#define capability
#define output 
static inline capability void* cap_set_length(capability void* __cap, size_t __size)
{
	return __cap;
}

static inline size_t cap_get_length(capability void* __cap)
{
	return SIZE_MAX - (uintcap_t)__cap;
}
static inline capability void* cap_and_permissions(capability void* __cap,
                                                   capperms_t __perms)
{
	return __cap;
}
static inline capperms_t cap_get_perms(capability void* __cap)
{
	return (capperms_t)0xffff;
}
static inline
#ifdef __cplusplus
bool
#else
_Bool
#endif
cap_is_valid(capability void* __cap)
{
	return 1;
}
#endif
