
// This is the FreeBSD startup code which uses lots of linker synthesized symbols

#define NOTE_FREEBSD_VENDOR "FreeBSD"

#define NOTE_SECTION ".note.tag"

#define ABI_NOTETYPE 1
#define CRT_NOINIT_NOTETYPE 2
#define ARCH_NOTETYPE 3

extern int main(int, char **, char **);

extern void (*__preinit_array_start[])(int, char **, char **) __hidden;

extern void (*__preinit_array_end[])(int, char **, char **) __hidden;

extern void (*__init_array_start[])(int, char **, char **) __hidden;

extern void (*__init_array_end[])(int, char **, char **) __hidden;

extern void (*__fini_array_start[])(void) __hidden;

extern void (*__fini_array_end[])(void) __hidden;

extern void _fini(void) __hidden;

extern void _init(void) __hidden;

extern int _DYNAMIC;
#pragma weak _DYNAMIC

/*
 * When linking with LLD the *_array_[start/end] symbols are undefined if the
 * linker discards the matching array section. When using BFD they will contain
 * the address of the next section after the discarded .array instead.
 * Marking these symbols as weak and checking for NULL is the simplest workaround
 * works around this issue. An alternative solution would be to use a custom
 * linker script for every executable or hardcode the retaining behavior in LLD.
 *
 * XXXAR: TODO: fix in upstream LLD
 */
#pragma weak __preinit_array_start
#pragma weak __preinit_array_end
#pragma weak __init_array_start
#pragma weak __init_array_end
#pragma weak __fini_array_start
#pragma weak __fini_array_end
#define weak_array_size(name)  \
  ((name##_start == NULL) ? 0 : ((name##_end) - (name##_start)))

char **environ = NULL;
const char *__progname = "";

static void
finalizer(void) {
  void (*fn)(void);
  size_t array_size, n;

  array_size = weak_array_size(__fini_array);
  for (n = array_size; n > 0; n--) {
    fn = __fini_array_start[n - 1];
    if ((uintptr_t) fn != 0 && (uintptr_t) fn != 1)
      (fn)();
  }
#ifndef __CHERI_PURE_CAPABILITY__
  _fini();
#endif
}

static inline void
handle_static_init(int argc, char **argv, char **env) {
  void (*fn)(int, char **, char **);
  size_t array_size, n;

  if (&_DYNAMIC != NULL)
    return;

  atexit(finalizer);

  array_size = weak_array_size(__preinit_array);
  for (n = 0; n < array_size; n++) {
    fn = __preinit_array_start[n];
    if ((uintptr_t) fn != 0 && (uintptr_t) fn != 1)
      fn(argc, argv, env);
  }
#ifndef __CHERI_PURE_CAPABILITY__
  _init();
#endif
  array_size = weak_array_size(__init_array);
  for (n = 0; n < array_size; n++) {
    fn = __init_array_start[n];
    if ((uintptr_t) fn != 0 && (uintptr_t) fn != 1)
      fn(argc, argv, env);
  }
}

static inline void
handle_argv(int argc, char *argv[], char **env) {
  const char *s;

  if (environ == NULL)
    environ = env;
  if (argc > 0 && argv[0] != NULL) {
    __progname = argv[0];
    for (s = __progname; *s != '\0'; s++) {
      if (*s == '/')
        __progname = s + 1;
    }
  }
}

static const struct {
    int32_t namesz;
    int32_t descsz;
    int32_t type;
    char name[sizeof(NOTE_FREEBSD_VENDOR)];
    uint32_t desc;
} crt_noinit_tag __attribute__ ((section (NOTE_SECTION),
aligned(4))) __used = {
  .namesz = sizeof(NOTE_FREEBSD_VENDOR),
  .descsz = sizeof(uint32_t),
  .type = CRT_NOINIT_NOTETYPE,
  .name = NOTE_FREEBSD_VENDOR,
  .desc = 0
};
