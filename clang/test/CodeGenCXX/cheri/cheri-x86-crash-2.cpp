// XXXAR: This was created by creduce from another crash bug. Looks like an upstream bug
// TODO: report upstream
// RUN: not --crash %clang_cc1 %s -o -
template <_Alloc> template <a...> void b template <a _Alloc> b<_Tp_Alloc>
