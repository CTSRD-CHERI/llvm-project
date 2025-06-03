.. title:: clang-tidy - cheri-FixCapFormat

cheri-FixCapFormat
==================

Integer capabilities that are passed to printf-style functions
need a cast to "unsigned long" or printf will misinterpret the
value. This check adds such a cast.

