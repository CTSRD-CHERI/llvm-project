.. title:: clang-tidy - cheri-FixCapAsArraySubscript

cheri-FixCapAsArraySubscript
============================

Explicitly cast capabilities used as an array index to an unsigned long.
Not doing this will crash the compiler with -fsanitize=array-bounds.
