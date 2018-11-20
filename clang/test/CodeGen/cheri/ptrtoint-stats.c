// RUN: %cheri_cc1 -S -o /dev/null %s -mllvm -collect-pointer-cast-stats -cheri-stats-file=%t.json
// RUN: FileCheck -input-file %t.json %s

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

// CHECK:      "pointer_cast_stats": {
// CHECK-NEXT: 	"ptrtoint": {
// CHECK-NEXT: 		"count": 3,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:5:15>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:9:22>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:27:9, line:31:9>"
// CHECK-NEXT: 		]
// CHECK-NEXT: 	}, "inttoptr": {
// CHECK-NEXT: 		"count": 3,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:13:18>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:31:9>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:26:9, line:31:9>"
// CHECK-NEXT: 		]
// CHECK-NEXT: 	}, "captoptr": {
// CHECK-NEXT: 		"count": 2,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:17:34>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:29:9, line:31:9>"
// CHECK-NEXT: 		]
// CHECK-NEXT: 	}, "ptrtocap": {
// CHECK-NEXT: 		"count": 2,
// CHECK-NEXT: 		"locations": [
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:21:44>",
// CHECK-NEXT: 			"<{{.+}}ptrtoint-stats.c:30:9, line:31:9>"
// CHECK-NEXT: 		]
// CHECK-NEXT: 	}
// CHECK-NEXT: }
