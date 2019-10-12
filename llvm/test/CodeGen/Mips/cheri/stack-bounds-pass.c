// REQUIRES: asserts
// RUN: %cheri_purecap_clang -mcpu=cheri128 -cheri=128 -xc -O0 -fsanitize-address-use-after-scope -g0 %s -o %t.ll -c -S -emit-llvm -Xclang -disable-O0-optnone -Wno-array-bounds -Wno-return-stack-address
// RUN: %cheri128_purecap_opt -cheri-bound-allocas %t.ll -o /dev/null -S -cheri-stack-bounds-single-intrinsic-threshold=10 \
// RUN:    -cheri-stack-bounds=if-needed -debug-only="cheri-bound-allocas" 2>&1 | FileCheck -check-prefix DBG %s

// RUN: %cheri_purecap_clang -mcpu=cheri128 -cheri=128 -xc -O1 -fsanitize-address-use-after-scope -g0 %s -o %t-lifetime.ll -c -S -emit-llvm -Xclang -disable-O0-optnone -Wno-array-bounds -Wno-return-stack-address
// RUN: %cheri128_purecap_opt -cheri-bound-allocas %t-lifetime.ll -o /dev/null -S -cheri-stack-bounds-single-intrinsic-threshold=10 \
// RUN:     -cheri-stack-bounds=if-needed -debug-only="cheri-bound-allocas" 2>&1 | FileCheck -check-prefix DBG-OPT %s

// check that we ignore lifetime.start and lifetime.end intrinisics at -O1 and higher
// DBG-OPT: cheri-bound-allocas:  -No need for stack bounds for lifetime_{start,end}:   call void @llvm.lifetime.end.p200i8(i64 4, i8 addrspace(200)* nonnull %0) #6
// DBG-OPT: cheri-bound-allocas:  -No need for stack bounds for lifetime_{start,end}:   call void @llvm.lifetime.start.p200i8(i64 4, i8 addrspace(200)* nonnull %0) #6
// DBG-OPT: cheri-bound-allocas: pass_arg: 2 of 7 users need bounds for   %x = alloca i32, align 4, addrspace(200)

int *call(int *arg);
void use(void *arg);

void *return_stack(void) {
  char buffer[16];
  return buffer;
}
// DBG: Checking function return_stack
// DBG-NEXT:  -Checking if getelementptr needs stack bounds:   %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT:   -Adding stack bounds for alloca that is returned:   ret i8 addrspace(200)* %arraydecay
// DBG-NEXT:  -Adding stack bounds since getelementptr user needs bounds:   ret i8 addrspace(200)* %arraydecay
// DBG-NEXT: Found alloca use that needs bounds:   %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: return_stack: 1 of 1 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-NEXT: return_stack: setting bounds on stack alloca to 16  %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-EMPTY:


int pass_arg(void) {
  int x = 5;
  int *result = call(&x);
  x += *result;
  call(&x);
  return x;
}
// DBG-NEXT: Checking function pass_arg
// DBG-NEXT:  -Checking if load/store needs bounds (GEP offset is 0):   %3 = load i32, i32 addrspace(200)* %x, align 4
// DBG-NEXT:   -Load/store size=4, alloca size=4, current GEP offset=0 for i32
// DBG-NEXT:   -Load/store is in bounds -> can reuse $csp for   %3 = load i32, i32 addrspace(200)* %x, align 4
// DBG-NEXT:  -Adding stack bounds since it is passed to call:   %call1 = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
// DBG-NEXT: Found alloca use that needs bounds:   %call1 = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
// DBG-NEXT:  -Checking if load/store needs bounds (GEP offset is 0):   store i32 %add, i32 addrspace(200)* %x, align 4
// DBG-NEXT:   -Load/store size=4, alloca size=4, current GEP offset=0 for i32
// DBG-NEXT:   -Load/store is in bounds -> can reuse $csp for   store i32 %add, i32 addrspace(200)* %x, align 4
// DBG-NEXT:  -Checking if load/store needs bounds (GEP offset is 0):   %2 = load i32, i32 addrspace(200)* %x, align 4
// DBG-NEXT:   -Load/store size=4, alloca size=4, current GEP offset=0 for i32
// DBG-NEXT:   -Load/store is in bounds -> can reuse $csp for   %2 = load i32, i32 addrspace(200)* %x, align 4
// DBG-NEXT:  -Adding stack bounds since it is passed to call:   %call = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
// DBG-NEXT: Found alloca use that needs bounds:   %call = call i32 addrspace(200)* @call(i32 addrspace(200)* %x)
// DBG-NEXT:  -Checking if load/store needs bounds (GEP offset is 0):   store i32 5, i32 addrspace(200)* %x, align 4
// DBG-NEXT:   -Load/store size=4, alloca size=4, current GEP offset=0 for i32
// DBG-NEXT:   -Load/store is in bounds -> can reuse $csp for   store i32 5, i32 addrspace(200)* %x, align 4
// DBG-NEXT: pass_arg: 2 of 6 users need bounds for   %x = alloca i32, align 4, addrspace(200)
// DBG-NEXT: pass_arg: setting bounds on stack alloca to 4  %x = alloca i32, align 4, addrspace(200)
// DBG-NEXT:  -Checking if load/store needs bounds (GEP offset is 0):   %3 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %result, align 16
// DBG-NEXT:   -Load/store size=16, alloca size=16, current GEP offset=0 for i32 addrspace(200)*
// DBG-NEXT:   -Load/store is in bounds -> can reuse $csp for   %3 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %result, align 16
// DBG-NEXT:  -Checking if load/store needs bounds (GEP offset is 0):   store i32 addrspace(200)* %call, i32 addrspace(200)* addrspace(200)* %result, align 16
// DBG-NEXT:   -Load/store size=16, alloca size=16, current GEP offset=0 for i32 addrspace(200)*
// DBG-NEXT:   -Load/store is in bounds -> can reuse $csp for   store i32 addrspace(200)* %call, i32 addrspace(200)* addrspace(200)* %result, align 16
// DBG-NEXT: pass_arg: 0 of 2 users need bounds for   %result = alloca i32 addrspace(200)*, align 16, addrspace(200)
// DBG-NEXT: No need to set bounds on stack alloca  %result = alloca i32 addrspace(200)*, align 16, addrspace(200)
// DBG-EMPTY:

void store_in_bounds_min(void) {
  char buffer[3];
  buffer[0] = '\0';
}

// DBG-NEXT: Checking function store_in_bounds_min
// DBG-NEXT:  -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT:   -Checking if load/store needs bounds (GEP offset is 0):   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT:    -Load/store size=1, alloca size=3, current GEP offset=0 for i8
// DBG-NEXT:    -Load/store is in bounds -> can reuse $csp for   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT:  -no getelementptr users need bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: store_in_bounds_min: 0 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: No need to set bounds on stack alloca  %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-EMPTY:

void store_in_bounds_max(void) {
  char buffer[3];
  buffer[2] = '\0';
}
// DBG-NEXT: Checking function store_in_bounds_max
// DBG-NEXT:  -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 2
// DBG-NEXT:   -Checking if load/store needs bounds (GEP offset is 2):   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT:    -Load/store size=1, alloca size=3, current GEP offset=2 for i8
// DBG-NEXT:    -Load/store is in bounds -> can reuse $csp for   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT:  -no getelementptr users need bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 2
// DBG-NEXT: store_in_bounds_max: 0 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: No need to set bounds on stack alloca  %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-EMPTY:


extern int take_int(int);
extern int unknown_value(void);

int access_array_unknown_index() {
    int buffer[128];
    // __asm__ volatile(""::"r"(buffer):"memory");
    //for (int i = 0; i <= n; i++)
    //    buffer[n] = random_value();
    return ((volatile int*)buffer)[unknown_value()];
}
// DBG-NEXT: Checking function access_array_unknown_index
// DBG-NEXT:  -Checking if getelementptr needs stack bounds:   %arraydecay = getelementptr inbounds [128 x i32], [128 x i32] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT:   -Could not accumulate constant GEP Offset -> need bounds:   %arrayidx = getelementptr inbounds i32, i32 addrspace(200)* %arraydecay, i64 %idxprom
// DBG-NEXT:  -Adding stack bounds since getelementptr user needs bounds:   %arrayidx = getelementptr inbounds i32, i32 addrspace(200)* %arraydecay, i64 %idxprom
// DBG-NEXT: Found alloca use that needs bounds:   %arraydecay = getelementptr inbounds [128 x i32], [128 x i32] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: access_array_unknown_index: 1 of 1 users need bounds for   %buffer = alloca [128 x i32], align 4, addrspace(200)
// DBG-NEXT: access_array_unknown_index: setting bounds on stack alloca to 512  %buffer = alloca [128 x i32], align 4, addrspace(200)
// DBG-EMPTY:

int access_array_unknown_index_2() {
    volatile int buffer[128];
    // __asm__ volatile(""::"r"(buffer):"memory");
    return take_int(buffer[unknown_value()]);
}
// DBG-NEXT: Checking function access_array_unknown_index_2
// DBG-NEXT:  -Could not accumulate constant GEP Offset -> need bounds:   %arrayidx = getelementptr inbounds [128 x i32], [128 x i32] addrspace(200)* %buffer, i64 0, i64 %idxprom
// DBG-NEXT: Found alloca use that needs bounds:   %arrayidx = getelementptr inbounds [128 x i32], [128 x i32] addrspace(200)* %buffer, i64 0, i64 %idxprom
// DBG-NEXT: access_array_unknown_index_2: 1 of 1 users need bounds for   %buffer = alloca [128 x i32], align 4, addrspace(200)
// DBG-NEXT: access_array_unknown_index_2: setting bounds on stack alloca to 512  %buffer = alloca [128 x i32], align 4, addrspace(200)
// DBG-EMPTY:
void store_out_of_bounds_1(void) {
  char buffer[3];
  buffer[-1] = '\0';
}
// DBG-NEXT: Checking function store_out_of_bounds_1
// DBG-NEXT: cheri-bound-allocas: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-bound-allocas:  -Checking if load/store needs bounds (GEP offset is -1):   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-bound-allocas:  -Load/store size=1, alloca size=3, current GEP offset=-1 for i8
// DBG-NEXT: cheri-bound-allocas:  -store_out_of_bounds_1: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-bound-allocas: -Adding stack bounds since getelementptr user needs bounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-bound-allocas: Found alloca use that needs bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-bound-allocas: store_out_of_bounds_1: 1 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_1: setting bounds on stack alloca to 3  %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-EMPTY:
void store_out_of_bounds_2(void) {
  char buffer[3];
  buffer[3] = '\0';
}

// DBG-NEXT: Checking function store_out_of_bounds_2
// DBG-NEXT: cheri-bound-allocas: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 3
// DBG-NEXT: cheri-bound-allocas:  -Checking if load/store needs bounds (GEP offset is 3):   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-bound-allocas:  -Load/store size=1, alloca size=3, current GEP offset=3 for i8
// DBG-NEXT: cheri-bound-allocas:  -store_out_of_bounds_2: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-bound-allocas: -Adding stack bounds since getelementptr user needs bounds:   store i8 0, i8 addrspace(200)* %arrayidx, align 1
// DBG-NEXT: cheri-bound-allocas: Found alloca use that needs bounds:   %arrayidx = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 3
// DBG-NEXT: cheri-bound-allocas: store_out_of_bounds_2: 1 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_2: setting bounds on stack alloca to 3  %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-EMPTY:
void store_out_of_bounds_3(void) {
  char buffer[3];
  *((int *)buffer) = 42;
}

// DBG-NEXT: Checking function store_out_of_bounds_3
// DBG-NEXT: cheri-bound-allocas: -Checking if getelementptr needs stack bounds:   %arraydecay = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-bound-allocas:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas:   -Checking if load/store needs bounds (GEP offset is 0):   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas:   -Load/store size=4, alloca size=3, current GEP offset=0 for i32
// DBG-NEXT: cheri-bound-allocas:   -store_out_of_bounds_3: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas:  -Adding stack bounds since bitcast user needs bounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas: -Adding stack bounds since getelementptr user needs bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas: Found alloca use that needs bounds:   %arraydecay = getelementptr inbounds [3 x i8], [3 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-bound-allocas: store_out_of_bounds_3: 1 of 1 users need bounds for   %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_3: setting bounds on stack alloca to 3  %buffer = alloca [3 x i8], align 1, addrspace(200)
// DBG-EMPTY:
void store_out_of_bounds_4(void) {
  char buffer[16];
  *((int *)&buffer[-1]) = 42;
}

// DBG-NEXT:  Checking function store_out_of_bounds_4
// DBG-NEXT: cheri-bound-allocas: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-bound-allocas:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas:   -Checking if load/store needs bounds (GEP offset is -1):   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas:   -Load/store size=4, alloca size=16, current GEP offset=-1 for i32
// DBG-NEXT: cheri-bound-allocas:   -store_out_of_bounds_4: stack load/store offset OUT OF BOUNDS -> adding csetbounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas:  -Adding stack bounds since bitcast user needs bounds:   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas: -Adding stack bounds since getelementptr user needs bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas: Found alloca use that needs bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -1
// DBG-NEXT: cheri-bound-allocas: store_out_of_bounds_4: 1 of 1 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-NEXT: store_out_of_bounds_4: setting bounds on stack alloca to 16  %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-EMPTY:
void store_in_bounds_cast(void) {
  char buffer[16];
  *((int *)buffer) = 42;
  *((int *)&buffer[12]) = 42;
}

// DBG-NEXT: Checking function store_in_bounds_cast
// DBG-NEXT: cheri-bound-allocas: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 12
// DBG-NEXT: cheri-bound-allocas:  -Checking if bitcast needs stack bounds:   %1 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas:   -Checking if load/store needs bounds (GEP offset is 12):   store i32 42, i32 addrspace(200)* %1, align 1
// DBG-NEXT: cheri-bound-allocas:   -Load/store size=4, alloca size=16, current GEP offset=12 for i32
// DBG-NEXT: cheri-bound-allocas:   -Load/store is in bounds -> can reuse $csp for   store i32 42, i32 addrspace(200)* %1, align 1
// DBG-NEXT: cheri-bound-allocas:  -no bitcast users need bounds:   %1 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas: -no getelementptr users need bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 12
// DBG-NEXT: cheri-bound-allocas: -Checking if getelementptr needs stack bounds:   %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-bound-allocas:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas:   -Checking if load/store needs bounds (GEP offset is 0):   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas:   -Load/store size=4, alloca size=16, current GEP offset=0 for i32
// DBG-NEXT: cheri-bound-allocas:   -Load/store is in bounds -> can reuse $csp for   store i32 42, i32 addrspace(200)* %0, align 1
// DBG-NEXT: cheri-bound-allocas:  -no bitcast users need bounds:   %0 = bitcast i8 addrspace(200)* %arraydecay to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas: -no getelementptr users need bounds:   %arraydecay = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 0
// DBG-NEXT: cheri-bound-allocas: store_in_bounds_cast: 0 of 2 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-NEXT: cheri-bound-allocas: No need to set bounds on stack alloca  %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-EMPTY:
void store_int_bounds_cast_tmp_out_of_bounds(void) {
  char buffer[16];
  ((int *)&buffer[-4])[1] = 42;
}

// DBG-NEXT: Checking function store_int_bounds_cast_tmp_out_of_bounds
// DBG-NEXT: cheri-bound-allocas: -Checking if getelementptr needs stack bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -4
// DBG-NEXT: cheri-bound-allocas:  -Checking if bitcast needs stack bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas:   -Checking if getelementptr needs stack bounds:   %arrayidx1 = getelementptr inbounds i32, i32 addrspace(200)* %0, i64 1
// DBG-NEXT: cheri-bound-allocas:    -Checking if load/store needs bounds (GEP offset is 0):   store i32 42, i32 addrspace(200)* %arrayidx1, align 1
// DBG-NEXT: cheri-bound-allocas:    -Load/store size=4, alloca size=16, current GEP offset=0 for i32
// DBG-NEXT: cheri-bound-allocas:    -Load/store is in bounds -> can reuse $csp for   store i32 42, i32 addrspace(200)* %arrayidx1, align 1
// DBG-NEXT: cheri-bound-allocas:   -no getelementptr users need bounds:   %arrayidx1 = getelementptr inbounds i32, i32 addrspace(200)* %0, i64 1
// DBG-NEXT: cheri-bound-allocas:  -no bitcast users need bounds:   %0 = bitcast i8 addrspace(200)* %arrayidx to i32 addrspace(200)*
// DBG-NEXT: cheri-bound-allocas: -no getelementptr users need bounds:   %arrayidx = getelementptr inbounds [16 x i8], [16 x i8] addrspace(200)* %buffer, i64 0, i64 -4
// DBG-NEXT: cheri-bound-allocas: store_int_bounds_cast_tmp_out_of_bounds: 0 of 1 users need bounds for   %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-NEXT: cheri-bound-allocas: No need to set bounds on stack alloca  %buffer = alloca [16 x i8], align 1, addrspace(200)
// DBG-EMPTY:
