# RUN: not llvm-mc -filetype=obj -triple=riscv64 -mattr=+xcheri -o /dev/null < %s 2>&1 \
# RUN:     | FileCheck %s

foo:

.data

## No add/sub relocations
.chericap foo - bar
# CHECK: error: symbol 'bar' can not be undefined in a subtraction expression

## Make sure we don't fold this to bar
.chericap foo + (bar - foo)
# CHECK: error: expected relocatable expression
