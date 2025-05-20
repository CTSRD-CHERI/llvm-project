.. title:: clang-tidy - cheri-FixIntToPtrCast

cheri-FixIntToPtrCast
=====================

Warn about and fix casts where a smaller integer type is cast
to a pointer directly. This results in a compiler error on CHERI
and can be fixed by adding an intermediate cast to uintptr_t.

Warn about cases where we have determined that the cast is ok
and suggest the insertion of the intermediate cast.
