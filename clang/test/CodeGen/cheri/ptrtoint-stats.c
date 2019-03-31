// REQUIRES: mips-registered-target, asserts
// RUN: rm -f %t.json
// RUN: %cheri_cc1 -S -o /dev/null %s -mllvm -collect-pointer-cast-stats -cheri-stats-file=%t.json
// RUNNOT: cat %t.json
// RUNNOT: jsonlint %t.json
// RUN: cat %t.json && FileCheck -input-file %t.json %s

int ptrtoint(void *value) {
  return (int)value;
}

int ptrtointcap(void *value) {
  return (__intcap_t)value;
}

void *inttoptr(long value) {
  return (void *)value;
}

void *captoptr(void *__capability value) {
  return (__cheri_fromcap void *)value;
}

void *__capability ptrtocap(void *value) {
  return (__cheri_tocap void *__capability)value;
}

void* multicast(long value) {
  return
        (long)
        (char*)
        (__cheri_fromcap void*)
        (__cheri_tocap void* __capability)
        (void*)
        value;
}

// CHECK:      { "pointer_cast_stats": {
// CHECK-NEXT: 	"main_file": "{{.+}}CodeGen/cheri/ptrtoint-stats.c",
// CHECK-NEXT: 	"ptrtoint": {
// CHECK-NEXT: 		"count": 3,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:9:15>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:13:22>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:31:9, line:35:9>"
// CHECK-NEXT: 		]
// CHECK-NEXT: 	}, "inttoptr": {
// CHECK-NEXT: 		"count": 3,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:17:18>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:35:9>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:30:9, line:35:9> (implicit)"
// CHECK-NEXT: 		],
// CHECK-NEXT: 		"implicit_count": 1
// CHECK-NEXT: 	}, "captoptr": {
// CHECK-NEXT: 		"count": 2,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:21:34>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:33:9, line:35:9>"
// CHECK-NEXT: 		]
// CHECK-NEXT: 	}, "ptrtocap": {
// CHECK-NEXT: 		"count": 2,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:25:44>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:34:9, line:35:9>"
// CHECK-NEXT: 		]
// CHECK-NEXT: 	}
// CHECK-NEXT: } }
