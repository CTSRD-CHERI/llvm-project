// RUN:  %cheri_purecap_cc1 -emit-llvm -std=c++11 -o - %s | FileCheck %s
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
  // CHECK: store i32 addrspace(200)* getelementptr inbounds ([3 x i32], [3 x i32] addrspace(200)* @.ref.tmp, i64 0, i64 0), i32 addrspace(200)* addrspace(200)* %__begin_
  std::initializer_list<int> l1 = {1, 2, 3};
}
