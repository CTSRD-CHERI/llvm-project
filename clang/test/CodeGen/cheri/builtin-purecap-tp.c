// RUN: %cheri_purecap_cc1 -Wall -Werror -disable-O0-optnone -emit-llvm -o - %s | opt -S -passes=mem2reg | FileCheck %s

// CHECK-LABEL: test_tp_char
// CHECK: llvm.thread.pointer.p200
void *test_tp_char() {
  return __builtin_thread_pointer();
}

// CHECK-LABEL: test_tp_int
// CHECK: llvm.thread.pointer.p200
int *test_tp_int() {
  return __builtin_thread_pointer();
}
