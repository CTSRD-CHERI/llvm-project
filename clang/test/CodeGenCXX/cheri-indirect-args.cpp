// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -emit-llvm -o - %s | FileCheck %s

// Check that arguments that are passed indirectly by pointer have the correct
// address space

class unique_lock {
  public:
    unique_lock() { }
    unique_lock(unique_lock const&) { }
};

// CHECK: define void @_Z25notify_all_at_thread_exit11unique_lock(%class.unique_lock addrspace(200)* %lk)
void notify_all_at_thread_exit(unique_lock lk) { }

int main(void) {
  unique_lock l;
  notify_all_at_thread_exit(l);
  return 0;
}

