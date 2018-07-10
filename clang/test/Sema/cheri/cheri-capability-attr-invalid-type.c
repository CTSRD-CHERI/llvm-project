// RUN: not %cheri_cc1 -target-abi n64 -fsyntax-only %s 2>&1 | FileCheck %s
// Test case for https://github.com/CTSRD-CHERI/clang/issues/157
// We only check if the compiler crashes

// CHECK: *

#define a(b)                                                                   \
  {                                                                            \
    typedef __typeof__(b) c;                                                   \
    __capability c d; }
int e() {
  int *pipv = (a(pip));
}


#  define ZEXPORT
#  define FAR
typedef unsigned char  Byte;
#  define Bytef Byte FAR
#define cheri_ptr_to_bounded_cap(ptr)   __extension__({ \
        typedef __typeof__(ptr) __ptr_type;             \
        typedef __capability __ptr_type __cap_type;     \
 z_stream;
typedef z_stream FAR * __capability z_streamp;
int ZEXPORT compress2 (dest, destLen, source, sourceLen, level)     Bytef *dest;
{
  z_stream stream;
  z_streamp stream_cap = cheri_ptr_to_bounded_cap(&stream);
