// CHECK that x86 works:
// RUN: %clang -target x86_64-unknown-freebsd -x c++ -fPIC %s -c -o %t.x86.o
// RUN: %clang -target x86_64-unknown-freebsd -x c++ -fPIC %s -c -S -emit-llvm -o %t.x86.ll
// RUN: %clang -target x86_64-unknown-freebsd -x c++ -fPIC %s -c -S -o %t.x86.s
// RUN: ld.lld --eh-frame-hdr --shared -z notext -o /dev/null %t.x86.o

// Check Mips variants:
// RUN: %clang -integrated-as -target mips-unknown-freebsd -x c++ -fPIC %s -c -o %t.mips.o
// RUN: %clang -integrated-as -target mips-unknown-freebsd -x c++ -fPIC %s -c -S -emit-llvm -o %t.mips.ll
// RUN: %clang -integrated-as -target mips-unknown-freebsd -x c++ -fPIC %s -c -S -o %t.mips.s
// RUN: ld.lld --eh-frame-hdr --shared -z notext -o /dev/null %t.mips.o
// RUN: %clang -integrated-as -target mips64-unknown-freebsd -x c++ -fPIC -mabi=n64 %s -c -o %t.mips-n64.o
// RUN: ld.lld --eh-frame-hdr --shared -z notext -o /dev/null %t.mips-n64.o
// RUN: %clang -integrated-as -target cheri-unknown-freebsd -x c++ -fPIC -mabi=n64 %s -c -o %t.cheri-n64.o
// RUN: ld.lld --eh-frame-hdr --shared -z notext -o /dev/null %t.cheri-n64.o

// RUN: %clang -integrated-as -target cheri-unknown-freebsd -x c++ -fPIC -mabi=purecap %s -c -o %t.cheri-purecap.o
// RUN: ld.lld --eh-frame-hdr --shared -z notext -o /dev/null -m elf64btsmip_cheri_fbsd %t.cheri-purecap.o

// RUN: %clang_cheri_purecap -x c++ -fPIC %s -c -o %t.o
// RUN: %clang_cheri_purecap -x c++ -fPIC %s -c -o %t.ll -S -emit-llvm
// RUN: ld.lld --eh-frame-hdr --shared -z notext -o /dev/null -m elf64btsmip_cheri_fbsd %t.o
// RUN: %clang_cheri_purecap -x c++ -fPIC %s -c -S -o %t.s
// RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd %t.s -o %t.s.o
// RUN: ld.lld --eh-frame-hdr --shared -z notext -o /dev/null -m elf64btsmip_cheri_fbsd %t.s.o

void a(int);
void b();
long c;
void d() { a(c); }
void e() throw() { b(); }
