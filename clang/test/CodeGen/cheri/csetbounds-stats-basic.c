// REQUIRES: asserts, mips-registered-target

// RUN: rm -f %t.json %t.csv
// RUN: %cheri_clang -g %s -mllvm -collect-csetbounds-stats -Xclang -cheri-stats-file=%t.json -c -o - -###
// RUN: %cheri_cc1 %s -mllvm -collect-csetbounds-stats=json -cheri-stats-file=%t.json -S -o /dev/null "-dwarf-column-info" "-debug-info-kind=standalone"
// RUN: FileCheck -input-file %t.json %s -check-prefix JSON

// RUN: %cheri_cc1 %s -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t.csv -S -o /dev/null "-dwarf-column-info" "-debug-info-kind=standalone"
// RUN: FileCheck -input-file %t.csv %s -check-prefix CSV
// Should not create a second header:
// RUN: %cheri_cc1 %s -mllvm -collect-csetbounds-stats=csv -cheri-stats-file=%t.csv -S -o /dev/null "-dwarf-column-info" "-debug-info-kind=standalone"
// RUN: FileCheck -input-file %t.csv %s -check-prefixes CSV,CSV-APPEND

extern void* malloc(unsigned long);
extern int do_stuff_with_cap(void *__capability cap);

int pass_stack_cap(void) {
  char buffer[4096];
  return do_stuff_with_cap(buffer);
}

int pass_stack_cap2(void) {
  int x;
  return do_stuff_with_cap(&x);
}

int use_stack_cap(void) {
  int x;
  int* y = &x;
  *y = 1;
  return x;
}

int* return_stack_cap(void) {
  int x;
  // Note: clang only warns about the simple case return &x, but not this
  int* y = &x;
  return y;
}

void* __capability csetbounds_malloc(void) {
  // The CHERI range checker pass also inserts bounds for return values of malloc, etc.
  void* __capability x = (__cheri_tocap void* __capability)malloc(16);
  return x;
}

// JSON:     { "csetbounds_stats": {
// JSON-NEXT: 	"count": 3,
// JSON-NEXT: 	"details": [
// JSON-NEXT: 	{
// JSON-NEXT: 		"alignment": 1,
// JSON-NEXT: 		"size": 4096,
// JSON-NEXT: 		"location": "{{.+}}/CodeGen/cheri/csetbounds-stats-basic.c:19:10",
// JSON-NEXT: 		"pass": "CHERI range checker"
// JSON-NEXT: 	},
// JSON-NEXT: 	{
// JSON-NEXT: 		"alignment": 4,
// JSON-NEXT: 		"size": 4,
// JSON-NEXT: 		"location": "{{.+}}/CodeGen/cheri/csetbounds-stats-basic.c:24:10",
// JSON-NEXT: 		"pass": "CHERI range checker"
// JSON-NEXT: 	},
// JSON-NEXT: 	{
// JSON-NEXT: 		"alignment": 1,
// JSON-NEXT: 		"size": 16,
// JSON-NEXT: 		"location": "{{.+}}/CodeGen/cheri/csetbounds-stats-basic.c:43:22",
// JSON-NEXT: 		"pass": "CHERI range checker"
// JSON-NEXT: 	}
// JSON-NEXT: 	]
// JSON-NEXT: } }

// CSV: alignment_bits,size,kind,source_loc,compiler_pass,details
// CSV-NEXT: 0,4096,s,"{{.+}}/CodeGen/cheri/csetbounds-stats-basic.c:19:10","CHERI range checker",""
// CSV-NEXT: 2,4,s,"{{.+}}/CodeGen/cheri/csetbounds-stats-basic.c:24:10","CHERI range checker",""
// CSV-NEXT: 0,16,h,"{{.+}}/CodeGen/cheri/csetbounds-stats-basic.c:43:22","CHERI range checker",""
// There should not be a header for the second write:
// CSV-APPEND-NEXT: 0,4096,s,"{{.+}}/CodeGen/cheri/csetbounds-stats-basic.c:19:10","CHERI range checker",""
