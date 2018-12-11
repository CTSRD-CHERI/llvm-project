#include <libunwind.h>
#include <stdlib.h>
#include <stdio.h>

void backtrace(int lower_bound) {
  unw_context_t context;
  unw_getcontext(&context);

  unw_cursor_t cursor;
  unw_init_local(&cursor, &context);

  char buffer[1024];
  unw_word_t offset = 0;

  int n = 0;
  while (unw_step(&cursor) > 0) {
    n++;
    if (unw_get_proc_name(&cursor, buffer, sizeof(buffer), &offset) == 0) {
      fprintf(stderr, "Frame %d: %s+%p\n", n, buffer, (void*)offset);
    } else {
      fprintf(stderr, "Frame %d: Could not get name for cursor\n", n);
    }
    if (n > 100) {
      fprintf(stderr, "ERROR: Got %d frames, but expected at most 100\n", n);
      abort();
    }
  };

  if (n < lower_bound) {
    fprintf(stderr, "ERROR: Got %d frames, but expected at least %d\n", n, lower_bound);
    abort();
  }
}

void test1(int i) {
  fprintf(stderr, "starting %s\n", __func__);
  backtrace(i);
}

void test2(int i, int j) {
  fprintf(stderr, "starting %s\n", __func__);
  backtrace(i);
  test1(j);
}

void test3(int i, int j, int k) {
  fprintf(stderr, "starting %s\n", __func__);
  backtrace(i);
  test2(j, k);
}

int main() {
  test1(1);
  test2(1, 2);
  test3(1, 2, 3);
}
