extern void* __capability external_cap;
extern char external_buffer[];
extern int external_func(int x);

static void* __capability cap_ptr = &external_cap;
static void* __capability buffer_ptr = &external_buffer[25];
static void* __capability func_ptr = &external_func;

int entry(void) {
    return (long)cap_ptr + (long)buffer_ptr + (long)func_ptr;
}
