// RUN: %clang_cc1 -triple mips64-unknown-freebsd12.0 -target-cpu mips4 -target-abi n64 -fsyntax-only %s

// previously __builtin_align_up constant expr evaluation returned a 64-bit APInt even for type int
// if the second parameter has a 64-bit width
// Assertion failed: (I.getBitWidth() == Info.Ctx.getIntWidth(E->getType()) && "Invalid evaluation result."), function Success, file /Users/alex/cheri/llvm/tools/clang/lib/AST/ExprConstant.cpp, line 7015.
// (I.getBitWidth()=64 Info.Ctx.getIntWidth(E->getType())=32)
void check_align_up_width(void) {
   int x = __builtin_align_up ( 0, sizeof(int) ) / sizeof (int);
   char c =  __builtin_align_up ((char)0, sizeof(int) ) / sizeof (int);
   long l =  __builtin_align_up (0llu, (char)1) / sizeof (int);
}

void check_align_down_width(void) {
   int x = __builtin_align_down ( 0, sizeof(int) ) / sizeof (int);
   char c =  __builtin_align_down ((char)0, sizeof(int) ) / sizeof (int);
   long l =  __builtin_align_down (0llu, (char)1 ) / sizeof (int);
}

void check_is_aligned_width(void) {
   int x = __builtin_is_aligned ( 0, sizeof(int) ) / sizeof (int);
   char c =  __builtin_is_aligned ((char)0, sizeof(int) ) / sizeof (int);
   long l =  __builtin_is_aligned (0llu, (char)1) / sizeof (int);
}

