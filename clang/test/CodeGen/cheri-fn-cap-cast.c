// RUN: %clang_cc1 %s -emit-llvm -triple cheri-unknown-freebsd -o - 
void (* __capability c)(void);
typedef void(*fnptr)(void);
void cheri_codeptr(const fnptr ptr)
{
  // Check that this cast is PCC-relative and not GDC-relative
  // CHECK: llvm.mips.pcc.get
  c = ptr;
}

