// RUN: %cheri_cc1 %s -emit-llvm  -o -
void (* __capability c)(void);
typedef void(*fnptr)(void);
void cheri_codeptr(const fnptr ptr)
{
  // Check that this cast is PCC-relative and not GDC-relative
  // CHECK: llvm.mips.pcc.get
  c = (void (* __capability)(void))ptr;
}

