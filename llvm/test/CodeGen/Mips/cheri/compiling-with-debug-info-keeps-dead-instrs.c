// Previously the addressing mode folder would keep dead cincoffsets even if
// they were only being used by debug instructions.
// RUN: %cheri_purecap_clang %s -c -o - -ggdb -O2 | llvm-objdump -d - > %t-dbg.dump
// RUN: %cheri_purecap_clang %s -c -o - -g0 -O2 | llvm-objdump -d -  > %t-nodbg.dump
// RUN: diff -u %t-dbg.dump %t-nodbg.dump

void clobber(void* buffer);

struct arg {
  char buffer[160];
  union {
    int i32;
    long i64;
    char i8;
  };
};

int foo(char* buffer) {
  __asm__ volatile("li $zero, 1":::"memory");
  int* value = (int*)&buffer[160];
  int result = *value;
  __asm__ volatile("li $zero, 2":::"memory");
  if (buffer[4] == 'a') {
    __asm__ volatile("li $zero, 3":::"memory");
    result += *value;
  }
  __asm__ volatile("li $zero, 4":::"memory");
  return result;
}
