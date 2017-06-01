// RUN:  not --crash %clang_cc1 "-triple" "cheri-unknown-freebsd" "-emit-obj" "-target-cpu" "cheri128" "-target-feature" "+soft-float" "-target-abi" "purecap" "-O2" "-std=gnu99" "-cheri-linker" "-mllvm" "-cheri128" "-x" "c" %s -mllvm -cheri-test-mode -o -
// This crashes with an assertion: void llvm::CCState::getInRegsParamInfo(unsigned int, unsigned int &, unsigned int &) const: Assertion `InRegsParamRecordIndex < ByValRegs.size() && "Wrong ByVal parameter index"' failed.

typedef struct { int err_msg[1024]; } Dwarf_Error;
Dwarf_Error a;
void fn2();
int fn1() {
  fn2(a);
}
