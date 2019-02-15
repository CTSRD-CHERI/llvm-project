// RUN: %cheri_purecap_cc1 -fsyntax-only -ast-dump %s | FileCheck %s

template <class _ValueType>
bool __libcpp_atomic_compare_exchange(_ValueType* __val,
    _ValueType* __expected, _ValueType __after,
    int __success_order,
    int __fail_order)
{
    // CHECK-NOT: ImplicitCastExpr {{.*}} {{.*}} 'long *' <NoOp>
    return __atomic_compare_exchange_n(__val, __expected, __after, true,
                                       __success_order, __fail_order);
}

void f() {
  long l;
  __libcpp_atomic_compare_exchange(&l, &l, l, 0, 0);
}
