// RUNNOT:  not --crash %cheri_cc1 "-triple" "cheri-unknown-freebsd" "-emit-obj" "-target-feature" "+soft-float" "-target-abi" "purecap" "-O2" "-std=gnu99" "-cheri-linker" "-x" "c" %s -o /dev/null
// RUN: %cheri_cc1 "-triple" "cheri-unknown-freebsd" "-emit-obj" "-target-feature" "+soft-float" "-target-abi" "purecap" "-O2" "-std=gnu99" "-cheri-linker" "-x" "c" %s -o /dev/null
// This used to crash with an assertion: void llvm::CCState::getInRegsParamInfo(unsigned int, unsigned int &, unsigned int &) const: Assertion `InRegsParamRecordIndex < ByValRegs.size() && "Wrong ByVal parameter index"' failed.

typedef struct { int err_msg[1024]; } Dwarf_Error;
Dwarf_Error a;
void fn2();
void fn1() {
  fn2(a);
}
