// RUN: mkdir -p %S/helloworld_src
// RUN: tar xJf %S/Inputs/helloworld.tar.xz -C %S/helloworld_src
// RUN: cd %S/helloworld_src/reproduce && ld.lld @%S/helloworld_src/reproduce/response.txt -o helloworld.exe
// RUN: cd %S/helloworld_src/reproduce && ld.lld @%S/helloworld_src/reproduce/response.txt -o %t.exe
// RUN: llvm-objdump -t -C -s %t.exe > /dev/null
// RUN: false
