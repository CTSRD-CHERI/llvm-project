// RUN:  %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -emit-llvm -std=c++11 -o - %s | FileCheck %s
namespace std
{

template<class _Ep>
class initializer_list
{
  const _Ep* __begin_;
  unsigned long    __size_;

  initializer_list(const _Ep* __b, unsigned long __s)
      : __begin_(__b),
        __size_(__s)
      {}
};


}

int main() {
  // CHECK: addrspacecast ([3 x i32]* @.ref.tmp to [3 x i32] addrspace(200)*
  std::initializer_list<int> l1 = {1, 2, 3};
}
