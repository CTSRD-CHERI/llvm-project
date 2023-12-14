.. title:: clang-tidy - misc-cheri-representable-subobject

misc-cheri-representable-subobject
==================================

Find structures that contain problematic fields for CHERI sub-object bounds.
When the compiler is configured to narrow sub-object bounds, there is no
guarantee that the target structure field is representable.

In the following case:

.. code-block:: c

  // Any size not representable by the target CHERI architecture
  #define LARGE (1 << 15)

  struct foo {
    int32_t value;        // offset = 0, size = 4
    char buffer[LARGE];   // offset = 4, size = 32KiB
  };

the ``buffer`` field will be prompted with a warning, because the CHERI
capability for ``&foo.buffer`` is not representable, under the assumption
that ``struct foo`` will be properly aligned for its size by the allocator.

There are two warnings. One is triggered when the field offset is
aligned down for representablity. Another warning is triggered when the
length of the field does not match the representable length of the capability.
