// REQUIRES: asserts
// RUN: %cheri_purecap_clang -mcpu=cheri128 -cheri=128 -xc -O0 -fsanitize-address-use-after-scope -g0 %s -o %t.ll -c -S -emit-llvm -Xclang -disable-O0-optnone -Wno-array-bounds -Wno-return-stack-address
// RUN: %cheri_purecap_clang -mcpu=cheri128 -cheri=128 -xc -O2 -fsanitize-address-use-after-scope -g0 %s -o - -c -S -mllvm -stop-before=cheriaddrmodefolder -Xclang -disable-O0-optnone -Wno-array-bounds -Wno-return-stack-address
// RUN: %cheri128_purecap_opt -cheri-purecap-alloca %t.ll -o /dev/null -S -cheri-stack-bounds-single-intrinsic-threshold=10 \
// RUN:    -cheri-stack-bounds=if-needed -debug-only="cheri-purecap-alloca" 2>&1 | FileCheck -check-prefix DBG %s

// RUN: %cheri_purecap_clang -mcpu=cheri128 -cheri=128 -xc -O1 -fsanitize-address-use-after-scope -g0 %s -o %t-lifetime.ll -c -S -emit-llvm -Xclang -disable-O0-optnone -Wno-array-bounds -Wno-return-stack-address
// RUN: %cheri128_purecap_opt -cheri-purecap-alloca %t-lifetime.ll -o /dev/null -S -cheri-stack-bounds-single-intrinsic-threshold=10 \
// RUN:     -cheri-stack-bounds=if-needed -debug-only="cheri-purecap-alloca" 2>&1 | FileCheck -check-prefix DBG-OPT %s

// chec that we ignore lifetime.start and lifetime.end intrinisics at -O1 and higher
// DBG-OPT: cheri-purecap-alloca:  -No need for stack bounds for lifetime_{start,end}:   call void @llvm.lifetime.end.p200i8(i64 4, i8 addrspace(200)* nonnull %0) #6
// DBG-OPT: cheri-purecap-alloca:  -No need for stack bounds for lifetime_{start,end}:   call void @llvm.lifetime.start.p200i8(i64 4, i8 addrspace(200)* nonnull %0) #6
// DBG-OPT: cheri-purecap-alloca: pass_arg: 2 of 7 users need bounds for   %x = alloca i32, align 4, addrspace(200)

int *call(int *arg);
void use(void *arg);

void *return_stack(void) {
  char buffer[16];
  return buffer;
}
// DBG-LABEL: Checking function return_stack
// DBG: -Adding stack bounds for alloca that is returned:   ret i8 addrspace(200)* %arraydecay
// DBG: return_stack: 1 of 1 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG: return_stack: setting bounds on stack alloca to 16

int pass_arg(void) {
  int x = 5;
  int *result = call(&x);
  x += *result;
  call(&x);
  return x;
}
// DBG-LABEL: Checking function pass_arg
// DBG: -Adding stack bounds since it is passed to call: %call1 = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
// DBG: -Adding stack bounds since it is passed to call: %call = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
// DBG: pass_arg: 2 of 6 users need bounds for   %x = alloca i32, align 4, addrspace(200)
// DBG: pass_arg: setting bounds on stack alloca to 4  %x = alloca i32, align 4, addrspace(200)
// DBG: -Load/store size=16, alloca size=16, current GEP offset=0 for i32 addrspace(200)*
// DBG: -Load/store is in bounds -> can reuse $csp for   %3 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %result
// DBG: -Load/store size=16, alloca size=16, current GEP offset=0 for i32 addrspace(200)*
// DBG: -Load/store is in bounds -> can reuse $csp for   store i32 addrspace(200)* %call, i32 addrspace(200)* addrspace(200)* %result
// DBG: pass_arg: 0 of 2 users need bounds for   %result = alloca i32 addrspace(200)*,
// DBG: No need to set bounds on stack alloca  %result = alloca i32 addrspace(200)*,

void store_in_bounds_min(void) {
  char buffer[3];
  buffer[0] = '\0';
}

// DBG-LABEL: Checking function store_in_bounds_min
// DBG: cheri-purecap-alloca:  -Load/store size=1, alloca size=3, current GEP offset=0 for i8
// DBG: cheri-purecap-alloca:  -Load/store is in bounds -> can reuse $csp for   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG: cheri-purecap-alloca: store_in_bounds_min: 0 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)

void store_in_bounds_max(void) {
  char buffer[3];
  buffer[2] = '\0';
}

// DBG-LABEL: Checking function store_in_bounds_max
// DBG: cheri-purecap-alloca:  -Load/store size=1, alloca size=3, current GEP offset=2 for i8
// DBG: cheri-purecap-alloca:  -Load/store is in bounds -> can reuse $csp for   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG: cheri-purecap-alloca: store_in_bounds_max: 0 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)

void store_out_of_bounds_1(void) {
  char buffer[3];
  buffer[-1] = '\0';
}

// DBG-LABEL: Checking function store_out_of_bounds_1
// DBG-NEXT: cheri-purecap-alloca: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-purecap-alloca:  -Checking if load/store needs bounds (GEP offset is -1):   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-purecap-alloca:  -Load/store size=1, alloca size=3, current GEP offset=-1 for i8
// DBG-NEXT: cheri-purecap-alloca:  -store_out_of_bounds_1: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-purecap-alloca: -Adding stack bounds since getelementptr user needs bounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-purecap-alloca: Found alloca use that needs bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-purecap-alloca: store_out_of_bounds_1: 1 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_1: setting bounds on stack alloca to 3  %buffer = alloca [3 x i8], align 1, addrspace(200)

void store_out_of_bounds_2(void) {
  char buffer[3];
  buffer[3] = '\0';
}

// DBG-LABEL: Checking function store_out_of_bounds_2
// DBG-NEXT: cheri-purecap-alloca: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 3
// DBG-NEXT: cheri-purecap-alloca:  -Checking if load/store needs bounds (GEP offset is 3):   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-purecap-alloca:  -Load/store size=1, alloca size=3, current GEP offset=3 for i8
// DBG-NEXT: cheri-purecap-alloca:  -store_out_of_bounds_2: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-purecap-alloca: -Adding stack bounds since getelementptr user needs bounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-purecap-alloca: Found alloca use that needs bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 3
// DBG-NEXT: cheri-purecap-alloca: store_out_of_bounds_2: 1 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_2: setting bounds on stack alloca to 3  %buffer = alloca [3 x i8], align 1, addrspace(200)

void store_out_of_bounds_3(void) {
  char buffer[3];
  *((int *)buffer) = 42;
}

// DBG-LABEL: Checking function store_out_of_bounds_3
// DBG-NEXT: cheri-purecap-alloca: -Checking if getelementptr needs stack bounds:   %arraydecay = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-purecap-alloca:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca:   -Checking if load/store needs bounds (GEP offset is 0):   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca:   -Load/store size=4, alloca size=3, current GEP offset=0 for i32
// DBG-NEXT: cheri-purecap-alloca:   -store_out_of_bounds_3: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca:  -Adding stack bounds since bitcast user needs bounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca: -Adding stack bounds since getelementptr user needs bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca: Found alloca use that needs bounds:   %arraydecay = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-purecap-alloca: store_out_of_bounds_3: 1 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_3: setting bounds on stack alloca to 3  %buffer = alloca [3 x i8], align 1, addrspace(200)

void store_out_of_bounds_4(void) {
  char buffer[16];
  *((int *)&buffer[-1]) = 42;
}

// DBG-LABEL:  Checking function store_out_of_bounds_4
// DBG-NEXT: cheri-purecap-alloca: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-purecap-alloca:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca:   -Checking if load/store needs bounds (GEP offset is -1):   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca:   -Load/store size=4, alloca size=16, current GEP offset=-1 for i32
// DBG-NEXT: cheri-purecap-alloca:   -store_out_of_bounds_4: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca:  -Adding stack bounds since bitcast user needs bounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca: -Adding stack bounds since getelementptr user needs bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca: Found alloca use that needs bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-purecap-alloca: store_out_of_bounds_4: 1 of 1 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_4: setting bounds on stack alloca to 16  %buffer = alloca [16 x i8], align 1, addrspace(200)

void store_in_bounds_cast(void) {
  char buffer[16];
  *((int *)buffer) = 42;
  *((int *)&buffer[12]) = 42;
}

// DBG-LABEL: Checking function store_in_bounds_cast
// DBG-NEXT: cheri-purecap-alloca: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 12
// DBG-NEXT: cheri-purecap-alloca:  -Checking if bitcast needs stack bounds:   %1 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca:   -Checking if load/store needs bounds (GEP offset is 12):   store i32 42, i32 addrspace(200)* %1, align 1
// DBG-NEXT: cheri-purecap-alloca:   -Load/store size=4, alloca size=16, current GEP offset=12 for i32
// DBG-NEXT: cheri-purecap-alloca:   -Load/store is in bounds -> can reuse $csp for   store i32 42, i32 addrspace(200)* %1, align 1
// DBG-NEXT: cheri-purecap-alloca:  -no bitcast users need bounds:   %1 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca: -no getelementptr users need bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 12
// DBG-NEXT: cheri-purecap-alloca: -Checking if getelementptr needs stack bounds:   %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-purecap-alloca:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca:   -Checking if load/store needs bounds (GEP offset is 0):   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca:   -Load/store size=4, alloca size=16, current GEP offset=0 for i32
// DBG-NEXT: cheri-purecap-alloca:   -Load/store is in bounds -> can reuse $csp for   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-purecap-alloca:  -no bitcast users need bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca: -no getelementptr users need bounds:   %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-purecap-alloca: store_in_bounds_cast: 0 of 2 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-NEXT: cheri-purecap-alloca: No need to set bounds on stack alloca  %buffer = alloca [16 x i8], align 1, addrspace(200)

void store_int_bounds_cast_tmp_out_of_bounds(void) {
  char buffer[16];
  ((int *)&buffer[-4])[1] = 42;
}

// DBG-LABEL: Checking function store_int_bounds_cast_tmp_out_of_bounds
// DBG-NEXT: cheri-purecap-alloca: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -4
// DBG-NEXT: cheri-purecap-alloca:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca:   -Checking if getelementptr needs stack bounds:   %arrayidx1 = getelementptr inbounds i32, i32 addrspace(200)* %0, i64 1
// DBG-NEXT: cheri-purecap-alloca:    -Checking if load/store needs bounds (GEP offset is 0):   store i32 42, i32 addrspace(200)* %arrayidx1, align 1
// DBG-NEXT: cheri-purecap-alloca:    -Load/store size=4, alloca size=16, current GEP offset=0 for i32
// DBG-NEXT: cheri-purecap-alloca:    -Load/store is in bounds -> can reuse $csp for   store i32 42, i32 addrspace(200)* %arrayidx1, align 1
// DBG-NEXT: cheri-purecap-alloca:   -no getelementptr users need bounds:   %arrayidx1 = getelementptr inbounds i32, i32 addrspace(200)* %0, i64 1
// DBG-NEXT: cheri-purecap-alloca:  -no bitcast users need bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-purecap-alloca: -no getelementptr users need bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -4
// DBG-NEXT: cheri-purecap-alloca: store_int_bounds_cast_tmp_out_of_bounds: 0 of 1 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-NEXT: cheri-purecap-alloca: No need to set bounds on stack alloca  %buffer = alloca [16 x i8], align 1, addrspace(200)
