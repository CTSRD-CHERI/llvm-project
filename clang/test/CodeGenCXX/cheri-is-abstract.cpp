// RUN: %cheri_purecap_cc1 -std=c++14 -emit-llvm -o - %s 2>&1 | FileCheck %s


struct __two {char __lx[2];};

template <class _Tp, _Tp __v>
struct integral_constant
{
    static constexpr const _Tp      value = __v;
    typedef _Tp               value_type;
    typedef integral_constant type;
    constexpr operator value_type() const noexcept {return value;}
    inline constexpr value_type operator ()() const noexcept {return value;}
};
template <class _Tp, _Tp __v>
constexpr const _Tp integral_constant<_Tp, __v>::value;

template <bool __b>
using bool_constant = integral_constant<bool, __b>;
#define _LIBCPP_BOOL_CONSTANT(__b) bool_constant<(__b)>
typedef _LIBCPP_BOOL_CONSTANT(true)  true_type;
typedef _LIBCPP_BOOL_CONSTANT(false) false_type;

template <class _Tp> struct is_class : public integral_constant<bool, __is_class(_Tp)> {};


namespace __is_abstract_imp
{
template <class _Tp> char  __test(_Tp (*)[1]);
template <class _Tp> __two __test(...);
}

template <class _Tp, bool = is_class<_Tp>::value>
struct __libcpp_abstract : public integral_constant<bool, sizeof(__is_abstract_imp::__test<_Tp>(0)) != 1> {};

template <class _Tp> struct __libcpp_abstract<_Tp, false> : public false_type {};

template <class _Tp> struct is_abstract : public __libcpp_abstract<_Tp> {};

template <class _Tp> constexpr bool is_abstract_v
    = is_abstract<_Tp>::value;

struct A {
    int m;
};

struct B {
    virtual void foo();
};

struct C {
    virtual void foo() = 0;
};

struct D : C {};

int main()
{
    bool x1 = is_abstract<A>::value;
    bool x2 = is_abstract<B>::value;
    bool x3 = is_abstract<C>::value;
    bool x4 = is_abstract<D>::value;
    bool x5 = is_abstract<int>::value;
    // CHECK-NOT: Assertion
}
