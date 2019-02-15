// RUN: %clang %s -O1 -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s
// XFAIL: *

struct example {
  int x;
};

typedef __capability struct example *example_t;

static __capability void *example_key;

#pragma opaque example_t example_key

static struct example example_object = {0};

static char *entry[] = {0};

void example_init(void)
{
  example_key = (__capability void *) entry;
}

int example_method(example_t o)
{
  return o->x++;
}

void invoke_method()
{
  example_t e;
  int rc;

  e = (example_t) &example_object;
  rc = example_method(e);
  // CHECK: llvm.mips.seal.cap.data
}

