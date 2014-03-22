; Check that this doesn't crash the vectoriser...
; RUN: opt -O2 -mtriple=cheri-unknown-freebsd %s -o %o
; ModuleID = 'print-llc.c'
target datalayout = "E-m:m-p200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

%struct.netdissect_options = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i64, i32, i32, i32, i32, i8*, i8*, %struct.sa_list*, %struct.sa_list*, i8*, %struct.esp_algorithm*, i8*, i32, i8*, i32, i32, i8 addrspace(200)*, i8 addrspace(200)*, i32, void (%struct.netdissect_options*, i64, i8 addrspace(200)*, i32)*, void (%struct.netdissect_options*, i32)*, i32 (%struct.netdissect_options*, i8*, ...)*, void (%struct.netdissect_options*, i8*, ...)*, void (%struct.netdissect_options*, i8*, ...)* }
%struct.sa_list = type opaque
%struct.esp_algorithm = type opaque
%struct.tok = type { i32, i8* }
%struct.oui_tok = type { i32, %struct.tok* }
%struct.unaligned_u_int16_t = type <{ i16 }>

@.str = private unnamed_addr constant [7 x i8] c"[|llc]\00", align 1
@gndo = external global %struct.netdissect_options*
@.str1 = private unnamed_addr constant [12 x i8] c"IPX 802.3: \00", align 1
@.str2 = private unnamed_addr constant [46 x i8] c"LLC, dsap %s (0x%02x) %s, ssap %s (0x%02x) %s\00", align 1
@llc_values = internal global [16 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([5 x i8]* @.str56, i32 0, i32 0) }, %struct.tok { i32 255, i8* getelementptr inbounds ([7 x i8]* @.str57, i32 0, i32 0) }, %struct.tok { i32 2, i8* getelementptr inbounds ([9 x i8]* @.str58, i32 0, i32 0) }, %struct.tok { i32 3, i8* getelementptr inbounds ([9 x i8]* @.str59, i32 0, i32 0) }, %struct.tok { i32 6, i8* getelementptr inbounds ([3 x i8]* @.str60, i32 0, i32 0) }, %struct.tok { i32 4, i8* getelementptr inbounds ([4 x i8]* @.str61, i32 0, i32 0) }, %struct.tok { i32 14, i8* getelementptr inbounds ([10 x i8]* @.str62, i32 0, i32 0) }, %struct.tok { i32 66, i8* getelementptr inbounds ([4 x i8]* @.str63, i32 0, i32 0) }, %struct.tok { i32 78, i8* getelementptr inbounds ([6 x i8]* @.str64, i32 0, i32 0) }, %struct.tok { i32 126, i8* getelementptr inbounds ([8 x i8]* @.str65, i32 0, i32 0) }, %struct.tok { i32 142, i8* getelementptr inbounds ([7 x i8]* @.str66, i32 0, i32 0) }, %struct.tok { i32 170, i8* getelementptr inbounds ([5 x i8]* @.str67, i32 0, i32 0) }, %struct.tok { i32 224, i8* getelementptr inbounds ([4 x i8]* @.str68, i32 0, i32 0) }, %struct.tok { i32 240, i8* getelementptr inbounds ([8 x i8]* @.str69, i32 0, i32 0) }, %struct.tok { i32 254, i8* getelementptr inbounds ([4 x i8]* @.str70, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str3 = private unnamed_addr constant [8 x i8] c"Unknown\00", align 1
@llc_ig_flag_values = internal constant [3 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([11 x i8]* @.str54, i32 0, i32 0) }, %struct.tok { i32 1, i8* getelementptr inbounds ([6 x i8]* @.str55, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@llc_flag_values = internal constant [7 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([8 x i8]* @.str50, i32 0, i32 0) }, %struct.tok { i32 1, i8* getelementptr inbounds ([9 x i8]* @.str51, i32 0, i32 0) }, %struct.tok { i32 16, i8* getelementptr inbounds ([5 x i8]* @.str52, i32 0, i32 0) }, %struct.tok { i32 17, i8* getelementptr inbounds ([6 x i8]* @.str53, i32 0, i32 0) }, %struct.tok { i32 256, i8* getelementptr inbounds ([5 x i8]* @.str52, i32 0, i32 0) }, %struct.tok { i32 257, i8* getelementptr inbounds ([6 x i8]* @.str53, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str4 = private unnamed_addr constant [16 x i8] c", ctrl 0x%02x: \00", align 1
@.str5 = private unnamed_addr constant [16 x i8] c", ctrl 0x%04x: \00", align 1
@.str6 = private unnamed_addr constant [12 x i8] c"IPX 802.2: \00", align 1
@.str7 = private unnamed_addr constant [4 x i8] c"%s \00", align 1
@.str8 = private unnamed_addr constant [20 x i8] c"Unknown DSAP 0x%02x\00", align 1
@.str9 = private unnamed_addr constant [12 x i8] c"%s > %s %s \00", align 1
@.str10 = private unnamed_addr constant [9 x i8] c"%s > %s \00", align 1
@.str11 = private unnamed_addr constant [20 x i8] c"Unknown SSAP 0x%02x\00", align 1
@.str12 = private unnamed_addr constant [15 x i8] c"%s %s > %s %s \00", align 1
@.str13 = private unnamed_addr constant [38 x i8] c"Unnumbered, %s, Flags [%s], length %u\00", align 1
@llc_cmd_values = internal global [9 x %struct.tok] [%struct.tok { i32 3, i8* getelementptr inbounds ([3 x i8]* @.str42, i32 0, i32 0) }, %struct.tok { i32 227, i8* getelementptr inbounds ([5 x i8]* @.str43, i32 0, i32 0) }, %struct.tok { i32 175, i8* getelementptr inbounds ([4 x i8]* @.str44, i32 0, i32 0) }, %struct.tok { i32 99, i8* getelementptr inbounds ([3 x i8]* @.str45, i32 0, i32 0) }, %struct.tok { i32 67, i8* getelementptr inbounds ([5 x i8]* @.str46, i32 0, i32 0) }, %struct.tok { i32 15, i8* getelementptr inbounds ([3 x i8]* @.str47, i32 0, i32 0) }, %struct.tok { i32 111, i8* getelementptr inbounds ([6 x i8]* @.str48, i32 0, i32 0) }, %struct.tok { i32 135, i8* getelementptr inbounds ([5 x i8]* @.str49, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str14 = private unnamed_addr constant [5 x i8] c"%02x\00", align 1
@.str15 = private unnamed_addr constant [2 x i8] c"?\00", align 1
@.str16 = private unnamed_addr constant [12 x i8] c": %02x %02x\00", align 1
@.str17 = private unnamed_addr constant [51 x i8] c"Supervisory, %s, rcv seq %u, Flags [%s], length %u\00", align 1
@llc_supervisory_values = internal constant [4 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([15 x i8]* @.str39, i32 0, i32 0) }, %struct.tok { i32 1, i8* getelementptr inbounds ([19 x i8]* @.str40, i32 0, i32 0) }, %struct.tok { i32 2, i8* getelementptr inbounds ([7 x i8]* @.str41, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str18 = private unnamed_addr constant [60 x i8] c"Information, send seq %u, rcv seq %u, Flags [%s], length %u\00", align 1
@null_values = internal constant [1 x %struct.tok] zeroinitializer, align 8
@oui_to_tok = internal constant [6 x %struct.oui_tok] [%struct.oui_tok { i32 0, %struct.tok* getelementptr inbounds ([0 x %struct.tok]* @ethertype_values, i32 0, i32 0) }, %struct.oui_tok { i32 248, %struct.tok* getelementptr inbounds ([0 x %struct.tok]* @ethertype_values, i32 0, i32 0) }, %struct.oui_tok { i32 524295, %struct.tok* getelementptr inbounds ([0 x %struct.tok]* @ethertype_values, i32 0, i32 0) }, %struct.oui_tok { i32 12, %struct.tok* getelementptr inbounds ([6 x %struct.tok]* @cisco_values, i32 0, i32 0) }, %struct.oui_tok { i32 32962, %struct.tok* getelementptr inbounds ([12 x %struct.tok]* @bridged_values, i32 0, i32 0) }, %struct.oui_tok zeroinitializer], align 8
@.str19 = private unnamed_addr constant [34 x i8] c"oui %s (0x%06x), %s %s (0x%04x): \00", align 1
@oui_values = external constant [0 x %struct.tok]
@.str20 = private unnamed_addr constant [10 x i8] c"ethertype\00", align 1
@.str21 = private unnamed_addr constant [4 x i8] c"pid\00", align 1
@.str22 = private unnamed_addr constant [8 x i8] c"[|snap]\00", align 1
@ethertype_values = external constant [0 x %struct.tok]
@cisco_values = internal constant [6 x %struct.tok] [%struct.tok { i32 8192, i8* getelementptr inbounds ([4 x i8]* @.str34, i32 0, i32 0) }, %struct.tok { i32 8195, i8* getelementptr inbounds ([4 x i8]* @.str35, i32 0, i32 0) }, %struct.tok { i32 8196, i8* getelementptr inbounds ([4 x i8]* @.str36, i32 0, i32 0) }, %struct.tok { i32 273, i8* getelementptr inbounds ([5 x i8]* @.str37, i32 0, i32 0) }, %struct.tok { i32 267, i8* getelementptr inbounds ([5 x i8]* @.str38, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@bridged_values = internal constant [12 x %struct.tok] [%struct.tok { i32 1, i8* getelementptr inbounds ([15 x i8]* @.str23, i32 0, i32 0) }, %struct.tok { i32 7, i8* getelementptr inbounds ([17 x i8]* @.str24, i32 0, i32 0) }, %struct.tok { i32 2, i8* getelementptr inbounds ([12 x i8]* @.str25, i32 0, i32 0) }, %struct.tok { i32 8, i8* getelementptr inbounds ([14 x i8]* @.str26, i32 0, i32 0) }, %struct.tok { i32 3, i8* getelementptr inbounds ([17 x i8]* @.str27, i32 0, i32 0) }, %struct.tok { i32 9, i8* getelementptr inbounds ([19 x i8]* @.str28, i32 0, i32 0) }, %struct.tok { i32 4, i8* getelementptr inbounds ([11 x i8]* @.str29, i32 0, i32 0) }, %struct.tok { i32 10, i8* getelementptr inbounds ([13 x i8]* @.str30, i32 0, i32 0) }, %struct.tok { i32 5, i8* getelementptr inbounds ([12 x i8]* @.str31, i32 0, i32 0) }, %struct.tok { i32 11, i8* getelementptr inbounds ([14 x i8]* @.str32, i32 0, i32 0) }, %struct.tok { i32 14, i8* getelementptr inbounds ([5 x i8]* @.str33, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str23 = private unnamed_addr constant [15 x i8] c"Ethernet + FCS\00", align 1
@.str24 = private unnamed_addr constant [17 x i8] c"Ethernet w/o FCS\00", align 1
@.str25 = private unnamed_addr constant [12 x i8] c"802.4 + FCS\00", align 1
@.str26 = private unnamed_addr constant [14 x i8] c"802.4 w/o FCS\00", align 1
@.str27 = private unnamed_addr constant [17 x i8] c"Token Ring + FCS\00", align 1
@.str28 = private unnamed_addr constant [19 x i8] c"Token Ring w/o FCS\00", align 1
@.str29 = private unnamed_addr constant [11 x i8] c"FDDI + FCS\00", align 1
@.str30 = private unnamed_addr constant [13 x i8] c"FDDI w/o FCS\00", align 1
@.str31 = private unnamed_addr constant [12 x i8] c"802.6 + FCS\00", align 1
@.str32 = private unnamed_addr constant [14 x i8] c"802.6 w/o FCS\00", align 1
@.str33 = private unnamed_addr constant [5 x i8] c"BPDU\00", align 1
@.str34 = private unnamed_addr constant [4 x i8] c"CDP\00", align 1
@.str35 = private unnamed_addr constant [4 x i8] c"VTP\00", align 1
@.str36 = private unnamed_addr constant [4 x i8] c"DTP\00", align 1
@.str37 = private unnamed_addr constant [5 x i8] c"UDLD\00", align 1
@.str38 = private unnamed_addr constant [5 x i8] c"PVST\00", align 1
@.str39 = private unnamed_addr constant [15 x i8] c"Receiver Ready\00", align 1
@.str40 = private unnamed_addr constant [19 x i8] c"Receiver not Ready\00", align 1
@.str41 = private unnamed_addr constant [7 x i8] c"Reject\00", align 1
@.str42 = private unnamed_addr constant [3 x i8] c"ui\00", align 1
@.str43 = private unnamed_addr constant [5 x i8] c"test\00", align 1
@.str44 = private unnamed_addr constant [4 x i8] c"xid\00", align 1
@.str45 = private unnamed_addr constant [3 x i8] c"ua\00", align 1
@.str46 = private unnamed_addr constant [5 x i8] c"disc\00", align 1
@.str47 = private unnamed_addr constant [3 x i8] c"dm\00", align 1
@.str48 = private unnamed_addr constant [6 x i8] c"sabme\00", align 1
@.str49 = private unnamed_addr constant [5 x i8] c"frmr\00", align 1
@.str50 = private unnamed_addr constant [8 x i8] c"Command\00", align 1
@.str51 = private unnamed_addr constant [9 x i8] c"Response\00", align 1
@.str52 = private unnamed_addr constant [5 x i8] c"Poll\00", align 1
@.str53 = private unnamed_addr constant [6 x i8] c"Final\00", align 1
@.str54 = private unnamed_addr constant [11 x i8] c"Individual\00", align 1
@.str55 = private unnamed_addr constant [6 x i8] c"Group\00", align 1
@.str56 = private unnamed_addr constant [5 x i8] c"Null\00", align 1
@.str57 = private unnamed_addr constant [7 x i8] c"Global\00", align 1
@.str58 = private unnamed_addr constant [9 x i8] c"802.1B I\00", align 1
@.str59 = private unnamed_addr constant [9 x i8] c"802.1B G\00", align 1
@.str60 = private unnamed_addr constant [3 x i8] c"IP\00", align 1
@.str61 = private unnamed_addr constant [4 x i8] c"SNA\00", align 1
@.str62 = private unnamed_addr constant [10 x i8] c"ProWay NM\00", align 1
@.str63 = private unnamed_addr constant [4 x i8] c"STP\00", align 1
@.str64 = private unnamed_addr constant [6 x i8] c"RS511\00", align 1
@.str65 = private unnamed_addr constant [8 x i8] c"ISO8208\00", align 1
@.str66 = private unnamed_addr constant [7 x i8] c"ProWay\00", align 1
@.str67 = private unnamed_addr constant [5 x i8] c"SNAP\00", align 1
@.str68 = private unnamed_addr constant [4 x i8] c"IPX\00", align 1
@.str69 = private unnamed_addr constant [8 x i8] c"NetBeui\00", align 1
@.str70 = private unnamed_addr constant [4 x i8] c"OSI\00", align 1

; Function Attrs: nounwind
define i32 @llc_print(i8 addrspace(200)* %p, i32 %length, i32 %caplen, i8 addrspace(200)* %esrc, i8 addrspace(200)* %edst, i16* %extracted_ethertype) #0 {
entry:
  %retval = alloca i32, align 4
  %p.addr = alloca i8 addrspace(200)*, align 32
  %length.addr = alloca i32, align 4
  %caplen.addr = alloca i32, align 4
  %esrc.addr = alloca i8 addrspace(200)*, align 32
  %edst.addr = alloca i8 addrspace(200)*, align 32
  %extracted_ethertype.addr = alloca i16*, align 8
  %dsap_field = alloca i8, align 1
  %dsap = alloca i8, align 1
  %ssap_field = alloca i8, align 1
  %ssap = alloca i8, align 1
  %control = alloca i16, align 2
  %is_u = alloca i32, align 4
  %ret = alloca i32, align 4
  store i8 addrspace(200)* %p, i8 addrspace(200)** %p.addr, align 32
  call void @llvm.dbg.declare(metadata !{i8 addrspace(200)** %p.addr}, metadata !87), !dbg !88
  store i32 %length, i32* %length.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %length.addr}, metadata !89), !dbg !88
  store i32 %caplen, i32* %caplen.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %caplen.addr}, metadata !90), !dbg !88
  store i8 addrspace(200)* %esrc, i8 addrspace(200)** %esrc.addr, align 32
  call void @llvm.dbg.declare(metadata !{i8 addrspace(200)** %esrc.addr}, metadata !91), !dbg !92
  store i8 addrspace(200)* %edst, i8 addrspace(200)** %edst.addr, align 32
  call void @llvm.dbg.declare(metadata !{i8 addrspace(200)** %edst.addr}, metadata !93), !dbg !92
  store i16* %extracted_ethertype, i16** %extracted_ethertype.addr, align 8
  call void @llvm.dbg.declare(metadata !{i16** %extracted_ethertype.addr}, metadata !94), !dbg !92
  call void @llvm.dbg.declare(metadata !{i8* %dsap_field}, metadata !95), !dbg !98
  call void @llvm.dbg.declare(metadata !{i8* %dsap}, metadata !99), !dbg !98
  call void @llvm.dbg.declare(metadata !{i8* %ssap_field}, metadata !100), !dbg !98
  call void @llvm.dbg.declare(metadata !{i8* %ssap}, metadata !101), !dbg !98
  call void @llvm.dbg.declare(metadata !{i16* %control}, metadata !102), !dbg !103
  call void @llvm.dbg.declare(metadata !{i32* %is_u}, metadata !104), !dbg !105
  call void @llvm.dbg.declare(metadata !{i32* %ret}, metadata !106), !dbg !107
  %0 = load i16** %extracted_ethertype.addr, align 8, !dbg !108
  store i16 0, i16* %0, align 2, !dbg !108
  %1 = load i32* %caplen.addr, align 4, !dbg !109
  %cmp = icmp ult i32 %1, 3, !dbg !109
  br i1 %cmp, label %if.then, label %if.end, !dbg !109

if.then:                                          ; preds = %entry
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([7 x i8]* @.str, i32 0, i32 0)), !dbg !111
  %2 = load i8 addrspace(200)** %p.addr, align 32, !dbg !113
  %3 = load i32* %caplen.addr, align 4, !dbg !113
  call void @default_print(i8 addrspace(200)* %2, i32 %3), !dbg !113
  store i32 0, i32* %retval, !dbg !114
  br label %return, !dbg !114

if.end:                                           ; preds = %entry
  %4 = load i8 addrspace(200)** %p.addr, align 32, !dbg !115
  %5 = load i8 addrspace(200)* %4, align 1, !dbg !115
  store i8 %5, i8* %dsap_field, align 1, !dbg !115
  %6 = load i8 addrspace(200)** %p.addr, align 32, !dbg !116
  %7 = ptrtoint i8 addrspace(200)* %6 to i64, !dbg !116
  %add = add i64 %7, 1, !dbg !116
  %8 = inttoptr i64 %add to i8 addrspace(200)*, !dbg !116
  %9 = load i8 addrspace(200)* %8, align 1, !dbg !116
  store i8 %9, i8* %ssap_field, align 1, !dbg !116
  %10 = load i8 addrspace(200)** %p.addr, align 32, !dbg !117
  %11 = ptrtoint i8 addrspace(200)* %10 to i64, !dbg !117
  %add1 = add i64 %11, 2, !dbg !117
  %12 = inttoptr i64 %add1 to i8 addrspace(200)*, !dbg !117
  %13 = load i8 addrspace(200)* %12, align 1, !dbg !117
  %conv = zext i8 %13 to i16, !dbg !117
  store i16 %conv, i16* %control, align 2, !dbg !117
  %14 = load i16* %control, align 2, !dbg !118
  %conv2 = zext i16 %14 to i32, !dbg !118
  %and = and i32 %conv2, 3, !dbg !118
  %cmp3 = icmp eq i32 %and, 3, !dbg !118
  br i1 %cmp3, label %if.then5, label %if.else, !dbg !118

if.then5:                                         ; preds = %if.end
  store i32 1, i32* %is_u, align 4, !dbg !120
  br label %if.end19, !dbg !122

if.else:                                          ; preds = %if.end
  %15 = load i32* %caplen.addr, align 4, !dbg !123
  %cmp6 = icmp ult i32 %15, 4, !dbg !123
  br i1 %cmp6, label %if.then8, label %if.end10, !dbg !123

if.then8:                                         ; preds = %if.else
  %call9 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([7 x i8]* @.str, i32 0, i32 0)), !dbg !126
  %16 = load i8 addrspace(200)** %p.addr, align 32, !dbg !128
  %17 = load i32* %caplen.addr, align 4, !dbg !128
  call void @default_print(i8 addrspace(200)* %16, i32 %17), !dbg !128
  store i32 0, i32* %retval, !dbg !129
  br label %return, !dbg !129

if.end10:                                         ; preds = %if.else
  %18 = load i8 addrspace(200)** %p.addr, align 32, !dbg !130
  %19 = ptrtoint i8 addrspace(200)* %18 to i64, !dbg !130
  %add11 = add i64 %19, 2, !dbg !130
  %20 = inttoptr i64 %add11 to i8 addrspace(200)*, !dbg !130
  %21 = addrspacecast i8 addrspace(200)* %20 to i8*, !dbg !130
  %add.ptr = getelementptr inbounds i8* %21, i64 1, !dbg !130
  %22 = load i8* %add.ptr, align 1, !dbg !130
  %conv12 = zext i8 %22 to i16, !dbg !130
  %conv13 = zext i16 %conv12 to i32, !dbg !130
  %shl = shl i32 %conv13, 8, !dbg !130
  %23 = load i8 addrspace(200)** %p.addr, align 32, !dbg !130
  %24 = ptrtoint i8 addrspace(200)* %23 to i64, !dbg !130
  %add14 = add i64 %24, 2, !dbg !130
  %25 = inttoptr i64 %add14 to i8 addrspace(200)*, !dbg !130
  %26 = addrspacecast i8 addrspace(200)* %25 to i8*, !dbg !130
  %add.ptr15 = getelementptr inbounds i8* %26, i64 0, !dbg !130
  %27 = load i8* %add.ptr15, align 1, !dbg !130
  %conv16 = zext i8 %27 to i16, !dbg !130
  %conv17 = zext i16 %conv16 to i32, !dbg !130
  %or = or i32 %shl, %conv17, !dbg !130
  %conv18 = trunc i32 %or to i16, !dbg !130
  store i16 %conv18, i16* %control, align 2, !dbg !130
  store i32 0, i32* %is_u, align 4, !dbg !131
  br label %if.end19

if.end19:                                         ; preds = %if.end10, %if.then5
  %28 = load i8* %ssap_field, align 1, !dbg !132
  %conv20 = zext i8 %28 to i32, !dbg !132
  %cmp21 = icmp eq i32 %conv20, 255, !dbg !132
  br i1 %cmp21, label %land.lhs.true, label %if.end30, !dbg !132

land.lhs.true:                                    ; preds = %if.end19
  %29 = load i8* %dsap_field, align 1, !dbg !132
  %conv23 = zext i8 %29 to i32, !dbg !132
  %cmp24 = icmp eq i32 %conv23, 255, !dbg !132
  br i1 %cmp24, label %if.then26, label %if.end30, !dbg !132

if.then26:                                        ; preds = %land.lhs.true
  %30 = load %struct.netdissect_options** @gndo, align 8, !dbg !134
  %ndo_eflag = getelementptr inbounds %struct.netdissect_options* %30, i32 0, i32 2, !dbg !134
  %31 = load i32* %ndo_eflag, align 4, !dbg !134
  %tobool = icmp ne i32 %31, 0, !dbg !134
  br i1 %tobool, label %if.then27, label %if.end29, !dbg !134

if.then27:                                        ; preds = %if.then26
  %call28 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([12 x i8]* @.str1, i32 0, i32 0)), !dbg !137
  br label %if.end29, !dbg !137

if.end29:                                         ; preds = %if.then27, %if.then26
  %32 = load i8 addrspace(200)** %p.addr, align 32, !dbg !138
  %33 = load i32* %length.addr, align 4, !dbg !138
  call void @ipx_print(i8 addrspace(200)* %32, i32 %33), !dbg !138
  store i32 1, i32* %retval, !dbg !139
  br label %return, !dbg !139

if.end30:                                         ; preds = %land.lhs.true, %if.end19
  %34 = load i8* %dsap_field, align 1, !dbg !140
  %conv31 = zext i8 %34 to i32, !dbg !140
  %and32 = and i32 %conv31, -2, !dbg !140
  %conv33 = trunc i32 %and32 to i8, !dbg !140
  store i8 %conv33, i8* %dsap, align 1, !dbg !140
  %35 = load i8* %ssap_field, align 1, !dbg !141
  %conv34 = zext i8 %35 to i32, !dbg !141
  %and35 = and i32 %conv34, -2, !dbg !141
  %conv36 = trunc i32 %and35 to i8, !dbg !141
  store i8 %conv36, i8* %ssap, align 1, !dbg !141
  %36 = load %struct.netdissect_options** @gndo, align 8, !dbg !142
  %ndo_eflag37 = getelementptr inbounds %struct.netdissect_options* %36, i32 0, i32 2, !dbg !142
  %37 = load i32* %ndo_eflag37, align 4, !dbg !142
  %tobool38 = icmp ne i32 %37, 0, !dbg !142
  br i1 %tobool38, label %if.then39, label %if.end61, !dbg !142

if.then39:                                        ; preds = %if.end30
  %38 = load i8* %dsap, align 1, !dbg !144
  %conv40 = zext i8 %38 to i32, !dbg !144
  %call41 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8]* @.str3, i32 0, i32 0), i32 %conv40), !dbg !144
  %39 = load i8* %dsap, align 1, !dbg !146
  %conv42 = zext i8 %39 to i32, !dbg !146
  %40 = load i8* %dsap_field, align 1, !dbg !147
  %conv43 = zext i8 %40 to i32, !dbg !147
  %and44 = and i32 %conv43, 1, !dbg !147
  %call45 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([3 x %struct.tok]* @llc_ig_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8]* @.str3, i32 0, i32 0), i32 %and44), !dbg !147
  %41 = load i8* %ssap, align 1, !dbg !148
  %conv46 = zext i8 %41 to i32, !dbg !148
  %call47 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8]* @.str3, i32 0, i32 0), i32 %conv46), !dbg !148
  %42 = load i8* %ssap, align 1, !dbg !146
  %conv48 = zext i8 %42 to i32, !dbg !146
  %43 = load i8* %ssap_field, align 1, !dbg !149
  %conv49 = zext i8 %43 to i32, !dbg !149
  %and50 = and i32 %conv49, 1, !dbg !149
  %call51 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8]* @.str3, i32 0, i32 0), i32 %and50), !dbg !149
  %call52 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([46 x i8]* @.str2, i32 0, i32 0), i8* %call41, i32 %conv42, i8* %call45, i8* %call47, i32 %conv48, i8* %call51), !dbg !146
  %44 = load i32* %is_u, align 4, !dbg !150
  %tobool53 = icmp ne i32 %44, 0, !dbg !150
  br i1 %tobool53, label %if.then54, label %if.else57, !dbg !150

if.then54:                                        ; preds = %if.then39
  %45 = load i16* %control, align 2, !dbg !152
  %conv55 = zext i16 %45 to i32, !dbg !152
  %call56 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([16 x i8]* @.str4, i32 0, i32 0), i32 %conv55), !dbg !152
  br label %if.end60, !dbg !154

if.else57:                                        ; preds = %if.then39
  %46 = load i16* %control, align 2, !dbg !155
  %conv58 = zext i16 %46 to i32, !dbg !155
  %call59 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([16 x i8]* @.str5, i32 0, i32 0), i32 %conv58), !dbg !155
  br label %if.end60

if.end60:                                         ; preds = %if.else57, %if.then54
  br label %if.end61, !dbg !157

if.end61:                                         ; preds = %if.end60, %if.end30
  %47 = load i8* %ssap, align 1, !dbg !158
  %conv62 = zext i8 %47 to i32, !dbg !158
  %cmp63 = icmp eq i32 %conv62, 66, !dbg !158
  br i1 %cmp63, label %land.lhs.true65, label %if.end75, !dbg !158

land.lhs.true65:                                  ; preds = %if.end61
  %48 = load i8* %dsap, align 1, !dbg !158
  %conv66 = zext i8 %48 to i32, !dbg !158
  %cmp67 = icmp eq i32 %conv66, 66, !dbg !158
  br i1 %cmp67, label %land.lhs.true69, label %if.end75, !dbg !158

land.lhs.true69:                                  ; preds = %land.lhs.true65
  %49 = load i16* %control, align 2, !dbg !158
  %conv70 = zext i16 %49 to i32, !dbg !158
  %cmp71 = icmp eq i32 %conv70, 3, !dbg !158
  br i1 %cmp71, label %if.then73, label %if.end75, !dbg !158

if.then73:                                        ; preds = %land.lhs.true69
  %50 = load i8 addrspace(200)** %p.addr, align 32, !dbg !160
  %51 = ptrtoint i8 addrspace(200)* %50 to i64, !dbg !160
  %add74 = add i64 %51, 3, !dbg !160
  %52 = inttoptr i64 %add74 to i8 addrspace(200)*, !dbg !160
  %53 = load i32* %length.addr, align 4, !dbg !160
  %sub = sub i32 %53, 3, !dbg !160
  call void @stp_print(i8 addrspace(200)* %52, i32 %sub), !dbg !160
  store i32 1, i32* %retval, !dbg !162
  br label %return, !dbg !162

if.end75:                                         ; preds = %land.lhs.true69, %land.lhs.true65, %if.end61
  %54 = load i8* %ssap, align 1, !dbg !163
  %conv76 = zext i8 %54 to i32, !dbg !163
  %cmp77 = icmp eq i32 %conv76, 6, !dbg !163
  br i1 %cmp77, label %land.lhs.true79, label %if.end90, !dbg !163

land.lhs.true79:                                  ; preds = %if.end75
  %55 = load i8* %dsap, align 1, !dbg !163
  %conv80 = zext i8 %55 to i32, !dbg !163
  %cmp81 = icmp eq i32 %conv80, 6, !dbg !163
  br i1 %cmp81, label %land.lhs.true83, label %if.end90, !dbg !163

land.lhs.true83:                                  ; preds = %land.lhs.true79
  %56 = load i16* %control, align 2, !dbg !163
  %conv84 = zext i16 %56 to i32, !dbg !163
  %cmp85 = icmp eq i32 %conv84, 3, !dbg !163
  br i1 %cmp85, label %if.then87, label %if.end90, !dbg !163

if.then87:                                        ; preds = %land.lhs.true83
  %57 = load %struct.netdissect_options** @gndo, align 8, !dbg !165
  %58 = load i8 addrspace(200)** %p.addr, align 32, !dbg !165
  %59 = ptrtoint i8 addrspace(200)* %58 to i64, !dbg !165
  %add88 = add i64 %59, 4, !dbg !165
  %60 = inttoptr i64 %add88 to i8 addrspace(200)*, !dbg !165
  %61 = load i32* %length.addr, align 4, !dbg !165
  %sub89 = sub i32 %61, 4, !dbg !165
  call void @ip_print(%struct.netdissect_options* %57, i64 undef, i8 addrspace(200)* %60, i32 %sub89), !dbg !165
  store i32 1, i32* %retval, !dbg !167
  br label %return, !dbg !167

if.end90:                                         ; preds = %land.lhs.true83, %land.lhs.true79, %if.end75
  %62 = load i8* %ssap, align 1, !dbg !168
  %conv91 = zext i8 %62 to i32, !dbg !168
  %cmp92 = icmp eq i32 %conv91, 224, !dbg !168
  br i1 %cmp92, label %land.lhs.true94, label %if.end110, !dbg !168

land.lhs.true94:                                  ; preds = %if.end90
  %63 = load i8* %dsap, align 1, !dbg !168
  %conv95 = zext i8 %63 to i32, !dbg !168
  %cmp96 = icmp eq i32 %conv95, 224, !dbg !168
  br i1 %cmp96, label %land.lhs.true98, label %if.end110, !dbg !168

land.lhs.true98:                                  ; preds = %land.lhs.true94
  %64 = load i16* %control, align 2, !dbg !168
  %conv99 = zext i16 %64 to i32, !dbg !168
  %cmp100 = icmp eq i32 %conv99, 3, !dbg !168
  br i1 %cmp100, label %if.then102, label %if.end110, !dbg !168

if.then102:                                       ; preds = %land.lhs.true98
  %65 = load %struct.netdissect_options** @gndo, align 8, !dbg !170
  %ndo_eflag103 = getelementptr inbounds %struct.netdissect_options* %65, i32 0, i32 2, !dbg !170
  %66 = load i32* %ndo_eflag103, align 4, !dbg !170
  %tobool104 = icmp ne i32 %66, 0, !dbg !170
  br i1 %tobool104, label %if.then105, label %if.end107, !dbg !170

if.then105:                                       ; preds = %if.then102
  %call106 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([12 x i8]* @.str6, i32 0, i32 0)), !dbg !173
  br label %if.end107, !dbg !173

if.end107:                                        ; preds = %if.then105, %if.then102
  %67 = load i8 addrspace(200)** %p.addr, align 32, !dbg !174
  %68 = ptrtoint i8 addrspace(200)* %67 to i64, !dbg !174
  %add108 = add i64 %68, 3, !dbg !174
  %69 = inttoptr i64 %add108 to i8 addrspace(200)*, !dbg !174
  %70 = load i32* %length.addr, align 4, !dbg !174
  %sub109 = sub i32 %70, 3, !dbg !174
  call void @ipx_print(i8 addrspace(200)* %69, i32 %sub109), !dbg !174
  store i32 1, i32* %retval, !dbg !175
  br label %return, !dbg !175

if.end110:                                        ; preds = %land.lhs.true98, %land.lhs.true94, %if.end90
  %71 = load i8* %ssap, align 1, !dbg !176
  %conv111 = zext i8 %71 to i32, !dbg !176
  %cmp112 = icmp eq i32 %conv111, 240, !dbg !176
  br i1 %cmp112, label %land.lhs.true114, label %if.end136, !dbg !176

land.lhs.true114:                                 ; preds = %if.end110
  %72 = load i8* %dsap, align 1, !dbg !176
  %conv115 = zext i8 %72 to i32, !dbg !176
  %cmp116 = icmp eq i32 %conv115, 240, !dbg !176
  br i1 %cmp116, label %land.lhs.true118, label %if.end136, !dbg !176

land.lhs.true118:                                 ; preds = %land.lhs.true114
  %73 = load i16* %control, align 2, !dbg !176
  %conv119 = zext i16 %73 to i32, !dbg !176
  %and120 = and i32 %conv119, 1, !dbg !176
  %tobool121 = icmp ne i32 %and120, 0, !dbg !176
  br i1 %tobool121, label %lor.lhs.false, label %if.then125, !dbg !176

lor.lhs.false:                                    ; preds = %land.lhs.true118
  %74 = load i16* %control, align 2, !dbg !176
  %conv122 = zext i16 %74 to i32, !dbg !176
  %cmp123 = icmp eq i32 %conv122, 3, !dbg !176
  br i1 %cmp123, label %if.then125, label %if.end136, !dbg !176

if.then125:                                       ; preds = %lor.lhs.false, %land.lhs.true118
  %75 = load i32* %is_u, align 4, !dbg !178
  %tobool126 = icmp ne i32 %75, 0, !dbg !178
  br i1 %tobool126, label %if.then127, label %if.else131, !dbg !178

if.then127:                                       ; preds = %if.then125
  %76 = load i8 addrspace(200)** %p.addr, align 32, !dbg !181
  %add.ptr128 = getelementptr inbounds i8 addrspace(200)* %76, i64 3, !dbg !181
  store i8 addrspace(200)* %add.ptr128, i8 addrspace(200)** %p.addr, align 32, !dbg !181
  %77 = load i32* %length.addr, align 4, !dbg !183
  %sub129 = sub i32 %77, 3, !dbg !183
  store i32 %sub129, i32* %length.addr, align 4, !dbg !183
  %78 = load i32* %caplen.addr, align 4, !dbg !184
  %sub130 = sub i32 %78, 3, !dbg !184
  store i32 %sub130, i32* %caplen.addr, align 4, !dbg !184
  br label %if.end135, !dbg !185

if.else131:                                       ; preds = %if.then125
  %79 = load i8 addrspace(200)** %p.addr, align 32, !dbg !186
  %add.ptr132 = getelementptr inbounds i8 addrspace(200)* %79, i64 4, !dbg !186
  store i8 addrspace(200)* %add.ptr132, i8 addrspace(200)** %p.addr, align 32, !dbg !186
  %80 = load i32* %length.addr, align 4, !dbg !188
  %sub133 = sub i32 %80, 4, !dbg !188
  store i32 %sub133, i32* %length.addr, align 4, !dbg !188
  %81 = load i32* %caplen.addr, align 4, !dbg !189
  %sub134 = sub i32 %81, 4, !dbg !189
  store i32 %sub134, i32* %caplen.addr, align 4, !dbg !189
  br label %if.end135

if.end135:                                        ; preds = %if.else131, %if.then127
  %82 = load i16* %control, align 2, !dbg !190
  %83 = load i8 addrspace(200)** %p.addr, align 32, !dbg !190
  %84 = load i32* %length.addr, align 4, !dbg !190
  call void @netbeui_print(i16 zeroext %82, i64 undef, i8 addrspace(200)* %83, i32 %84), !dbg !190
  store i32 1, i32* %retval, !dbg !191
  br label %return, !dbg !191

if.end136:                                        ; preds = %lor.lhs.false, %land.lhs.true114, %if.end110
  %85 = load i8* %ssap, align 1, !dbg !192
  %conv137 = zext i8 %85 to i32, !dbg !192
  %cmp138 = icmp eq i32 %conv137, 254, !dbg !192
  br i1 %cmp138, label %land.lhs.true140, label %if.end152, !dbg !192

land.lhs.true140:                                 ; preds = %if.end136
  %86 = load i8* %dsap, align 1, !dbg !192
  %conv141 = zext i8 %86 to i32, !dbg !192
  %cmp142 = icmp eq i32 %conv141, 254, !dbg !192
  br i1 %cmp142, label %land.lhs.true144, label %if.end152, !dbg !192

land.lhs.true144:                                 ; preds = %land.lhs.true140
  %87 = load i16* %control, align 2, !dbg !192
  %conv145 = zext i16 %87 to i32, !dbg !192
  %cmp146 = icmp eq i32 %conv145, 3, !dbg !192
  br i1 %cmp146, label %if.then148, label %if.end152, !dbg !192

if.then148:                                       ; preds = %land.lhs.true144
  %88 = load i8 addrspace(200)** %p.addr, align 32, !dbg !194
  %89 = ptrtoint i8 addrspace(200)* %88 to i64, !dbg !194
  %add149 = add i64 %89, 3, !dbg !194
  %90 = inttoptr i64 %add149 to i8 addrspace(200)*, !dbg !194
  %91 = load i32* %length.addr, align 4, !dbg !194
  %sub150 = sub i32 %91, 3, !dbg !194
  %92 = load i32* %caplen.addr, align 4, !dbg !194
  %sub151 = sub i32 %92, 3, !dbg !194
  call void @isoclns_print(i8 addrspace(200)* %90, i32 %sub150, i32 %sub151), !dbg !194
  store i32 1, i32* %retval, !dbg !196
  br label %return, !dbg !196

if.end152:                                        ; preds = %land.lhs.true144, %land.lhs.true140, %if.end136
  %93 = load i8* %ssap, align 1, !dbg !197
  %conv153 = zext i8 %93 to i32, !dbg !197
  %cmp154 = icmp eq i32 %conv153, 170, !dbg !197
  br i1 %cmp154, label %land.lhs.true156, label %if.end172, !dbg !197

land.lhs.true156:                                 ; preds = %if.end152
  %94 = load i8* %dsap, align 1, !dbg !197
  %conv157 = zext i8 %94 to i32, !dbg !197
  %cmp158 = icmp eq i32 %conv157, 170, !dbg !197
  br i1 %cmp158, label %land.lhs.true160, label %if.end172, !dbg !197

land.lhs.true160:                                 ; preds = %land.lhs.true156
  %95 = load i16* %control, align 2, !dbg !197
  %conv161 = zext i16 %95 to i32, !dbg !197
  %cmp162 = icmp eq i32 %conv161, 3, !dbg !197
  br i1 %cmp162, label %if.then164, label %if.end172, !dbg !197

if.then164:                                       ; preds = %land.lhs.true160
  %96 = load i8 addrspace(200)** %p.addr, align 32, !dbg !199
  %97 = ptrtoint i8 addrspace(200)* %96 to i64, !dbg !199
  %add165 = add i64 %97, 3, !dbg !199
  %98 = inttoptr i64 %add165 to i8 addrspace(200)*, !dbg !199
  %99 = load i32* %length.addr, align 4, !dbg !199
  %sub166 = sub i32 %99, 3, !dbg !199
  %100 = load i32* %caplen.addr, align 4, !dbg !199
  %sub167 = sub i32 %100, 3, !dbg !199
  %call168 = call i32 @snap_print(i8 addrspace(200)* %98, i32 %sub166, i32 %sub167, i32 2), !dbg !199
  store i32 %call168, i32* %ret, align 4, !dbg !199
  %101 = load i32* %ret, align 4, !dbg !201
  %tobool169 = icmp ne i32 %101, 0, !dbg !201
  br i1 %tobool169, label %if.then170, label %if.end171, !dbg !201

if.then170:                                       ; preds = %if.then164
  %102 = load i32* %ret, align 4, !dbg !203
  store i32 %102, i32* %retval, !dbg !203
  br label %return, !dbg !203

if.end171:                                        ; preds = %if.then164
  br label %if.end172, !dbg !204

if.end172:                                        ; preds = %if.end171, %land.lhs.true160, %land.lhs.true156, %if.end152
  %103 = load %struct.netdissect_options** @gndo, align 8, !dbg !205
  %ndo_eflag173 = getelementptr inbounds %struct.netdissect_options* %103, i32 0, i32 2, !dbg !205
  %104 = load i32* %ndo_eflag173, align 4, !dbg !205
  %tobool174 = icmp ne i32 %104, 0, !dbg !205
  br i1 %tobool174, label %if.end219, label %if.then175, !dbg !205

if.then175:                                       ; preds = %if.end172
  %105 = load i8* %ssap, align 1, !dbg !207
  %conv176 = zext i8 %105 to i32, !dbg !207
  %106 = load i8* %dsap, align 1, !dbg !207
  %conv177 = zext i8 %106 to i32, !dbg !207
  %cmp178 = icmp eq i32 %conv176, %conv177, !dbg !207
  br i1 %cmp178, label %if.then180, label %if.else197, !dbg !207

if.then180:                                       ; preds = %if.then175
  %107 = load i8 addrspace(200)** %esrc.addr, align 32, !dbg !210
  %108 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495), !dbg !210
  %109 = ptrtoint i8 addrspace(200)* %107 to i64, !dbg !210
  %110 = ptrtoint i8 addrspace(200)* %108 to i64, !dbg !210
  %cmp181 = icmp eq i64 %109, %110, !dbg !210
  br i1 %cmp181, label %if.then186, label %lor.lhs.false183, !dbg !210

lor.lhs.false183:                                 ; preds = %if.then180
  %111 = load i8 addrspace(200)** %edst.addr, align 32, !dbg !210
  %112 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495), !dbg !210
  %113 = ptrtoint i8 addrspace(200)* %111 to i64, !dbg !210
  %114 = ptrtoint i8 addrspace(200)* %112 to i64, !dbg !210
  %cmp184 = icmp eq i64 %113, %114, !dbg !210
  br i1 %cmp184, label %if.then186, label %if.else190, !dbg !210

if.then186:                                       ; preds = %lor.lhs.false183, %if.then180
  %115 = load i8* %dsap, align 1, !dbg !213
  %conv187 = zext i8 %115 to i32, !dbg !213
  %call188 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8]* @.str8, i32 0, i32 0), i32 %conv187), !dbg !213
  %call189 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str7, i32 0, i32 0), i8* %call188), !dbg !213
  br label %if.end196, !dbg !213

if.else190:                                       ; preds = %lor.lhs.false183
  %116 = load i8 addrspace(200)** %esrc.addr, align 32, !dbg !214
  %call191 = call i8* @etheraddr_string(i8 addrspace(200)* %116), !dbg !214
  %117 = load i8 addrspace(200)** %edst.addr, align 32, !dbg !215
  %call192 = call i8* @etheraddr_string(i8 addrspace(200)* %117), !dbg !215
  %118 = load i8* %dsap, align 1, !dbg !216
  %conv193 = zext i8 %118 to i32, !dbg !216
  %call194 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8]* @.str8, i32 0, i32 0), i32 %conv193), !dbg !216
  %call195 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([12 x i8]* @.str9, i32 0, i32 0), i8* %call191, i8* %call192, i8* %call194), !dbg !217
  br label %if.end196

if.end196:                                        ; preds = %if.else190, %if.then186
  br label %if.end218, !dbg !218

if.else197:                                       ; preds = %if.then175
  %119 = load i8 addrspace(200)** %esrc.addr, align 32, !dbg !219
  %120 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495), !dbg !219
  %121 = ptrtoint i8 addrspace(200)* %119 to i64, !dbg !219
  %122 = ptrtoint i8 addrspace(200)* %120 to i64, !dbg !219
  %cmp198 = icmp eq i64 %121, %122, !dbg !219
  br i1 %cmp198, label %if.then203, label %lor.lhs.false200, !dbg !219

lor.lhs.false200:                                 ; preds = %if.else197
  %123 = load i8 addrspace(200)** %edst.addr, align 32, !dbg !219
  %124 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495), !dbg !219
  %125 = ptrtoint i8 addrspace(200)* %123 to i64, !dbg !219
  %126 = ptrtoint i8 addrspace(200)* %124 to i64, !dbg !219
  %cmp201 = icmp eq i64 %125, %126, !dbg !219
  br i1 %cmp201, label %if.then203, label %if.else209, !dbg !219

if.then203:                                       ; preds = %lor.lhs.false200, %if.else197
  %127 = load i8* %ssap, align 1, !dbg !222
  %conv204 = zext i8 %127 to i32, !dbg !222
  %call205 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8]* @.str11, i32 0, i32 0), i32 %conv204), !dbg !222
  %128 = load i8* %dsap, align 1, !dbg !223
  %conv206 = zext i8 %128 to i32, !dbg !223
  %call207 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8]* @.str8, i32 0, i32 0), i32 %conv206), !dbg !223
  %call208 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str10, i32 0, i32 0), i8* %call205, i8* %call207), !dbg !224
  br label %if.end217, !dbg !224

if.else209:                                       ; preds = %lor.lhs.false200
  %129 = load i8 addrspace(200)** %esrc.addr, align 32, !dbg !225
  %call210 = call i8* @etheraddr_string(i8 addrspace(200)* %129), !dbg !225
  %130 = load i8* %ssap, align 1, !dbg !226
  %conv211 = zext i8 %130 to i32, !dbg !226
  %call212 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8]* @.str11, i32 0, i32 0), i32 %conv211), !dbg !226
  %131 = load i8 addrspace(200)** %edst.addr, align 32, !dbg !227
  %call213 = call i8* @etheraddr_string(i8 addrspace(200)* %131), !dbg !227
  %132 = load i8* %dsap, align 1, !dbg !228
  %conv214 = zext i8 %132 to i32, !dbg !228
  %call215 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8]* @.str8, i32 0, i32 0), i32 %conv214), !dbg !228
  %call216 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.str12, i32 0, i32 0), i8* %call210, i8* %call212, i8* %call213, i8* %call215), !dbg !229
  br label %if.end217

if.end217:                                        ; preds = %if.else209, %if.then203
  br label %if.end218

if.end218:                                        ; preds = %if.end217, %if.end196
  br label %if.end219, !dbg !230

if.end219:                                        ; preds = %if.end218, %if.end172
  %133 = load i32* %is_u, align 4, !dbg !231
  %tobool220 = icmp ne i32 %133, 0, !dbg !231
  br i1 %tobool220, label %if.then221, label %if.else253, !dbg !231

if.then221:                                       ; preds = %if.end219
  %134 = load i16* %control, align 2, !dbg !233
  %conv222 = zext i16 %134 to i32, !dbg !233
  %and223 = and i32 %conv222, 239, !dbg !233
  %call224 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([9 x %struct.tok]* @llc_cmd_values, i32 0, i32 0), i8* getelementptr inbounds ([5 x i8]* @.str14, i32 0, i32 0), i32 %and223), !dbg !233
  %135 = load i8* %ssap_field, align 1, !dbg !235
  %conv225 = zext i8 %135 to i32, !dbg !235
  %and226 = and i32 %conv225, 1, !dbg !235
  %136 = load i16* %control, align 2, !dbg !235
  %conv227 = zext i16 %136 to i32, !dbg !235
  %and228 = and i32 %conv227, 16, !dbg !235
  %or229 = or i32 %and226, %and228, !dbg !235
  %call230 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8]* @.str15, i32 0, i32 0), i32 %or229), !dbg !235
  %137 = load i32* %length.addr, align 4, !dbg !236
  %call231 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([38 x i8]* @.str13, i32 0, i32 0), i8* %call224, i8* %call230, i32 %137), !dbg !236
  %138 = load i8 addrspace(200)** %p.addr, align 32, !dbg !237
  %add.ptr232 = getelementptr inbounds i8 addrspace(200)* %138, i64 3, !dbg !237
  store i8 addrspace(200)* %add.ptr232, i8 addrspace(200)** %p.addr, align 32, !dbg !237
  %139 = load i32* %length.addr, align 4, !dbg !238
  %sub233 = sub i32 %139, 3, !dbg !238
  store i32 %sub233, i32* %length.addr, align 4, !dbg !238
  %140 = load i32* %caplen.addr, align 4, !dbg !239
  %sub234 = sub i32 %140, 3, !dbg !239
  store i32 %sub234, i32* %caplen.addr, align 4, !dbg !239
  %141 = load i16* %control, align 2, !dbg !240
  %conv235 = zext i16 %141 to i32, !dbg !240
  %and236 = and i32 %conv235, -17, !dbg !240
  %cmp237 = icmp eq i32 %and236, 175, !dbg !240
  br i1 %cmp237, label %if.then239, label %if.end252, !dbg !240

if.then239:                                       ; preds = %if.then221
  %142 = load i8 addrspace(200)** %p.addr, align 32, !dbg !242
  %143 = load i8 addrspace(200)* %142, align 1, !dbg !242
  %conv240 = zext i8 %143 to i32, !dbg !242
  %cmp241 = icmp eq i32 %conv240, 129, !dbg !242
  br i1 %cmp241, label %if.then243, label %if.end251, !dbg !242

if.then243:                                       ; preds = %if.then239
  %144 = load i8 addrspace(200)** %p.addr, align 32, !dbg !245
  %arrayidx = getelementptr inbounds i8 addrspace(200)* %144, i64 1, !dbg !245
  %145 = load i8 addrspace(200)* %arrayidx, align 1, !dbg !245
  %conv244 = zext i8 %145 to i32, !dbg !245
  %146 = load i8 addrspace(200)** %p.addr, align 32, !dbg !245
  %arrayidx245 = getelementptr inbounds i8 addrspace(200)* %146, i64 2, !dbg !245
  %147 = load i8 addrspace(200)* %arrayidx245, align 1, !dbg !245
  %conv246 = zext i8 %147 to i32, !dbg !245
  %call247 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([12 x i8]* @.str16, i32 0, i32 0), i32 %conv244, i32 %conv246), !dbg !245
  %148 = load i8 addrspace(200)** %p.addr, align 32, !dbg !247
  %add.ptr248 = getelementptr inbounds i8 addrspace(200)* %148, i64 3, !dbg !247
  store i8 addrspace(200)* %add.ptr248, i8 addrspace(200)** %p.addr, align 32, !dbg !247
  %149 = load i32* %length.addr, align 4, !dbg !248
  %sub249 = sub i32 %149, 3, !dbg !248
  store i32 %sub249, i32* %length.addr, align 4, !dbg !248
  %150 = load i32* %caplen.addr, align 4, !dbg !249
  %sub250 = sub i32 %150, 3, !dbg !249
  store i32 %sub250, i32* %caplen.addr, align 4, !dbg !249
  br label %if.end251, !dbg !250

if.end251:                                        ; preds = %if.then243, %if.then239
  br label %if.end252, !dbg !251

if.end252:                                        ; preds = %if.end251, %if.then221
  br label %if.end290, !dbg !252

if.else253:                                       ; preds = %if.end219
  %151 = load i16* %control, align 2, !dbg !253
  %conv254 = zext i16 %151 to i32, !dbg !253
  %and255 = and i32 %conv254, 1, !dbg !253
  %cmp256 = icmp eq i32 %and255, 1, !dbg !253
  br i1 %cmp256, label %if.then258, label %if.else272, !dbg !253

if.then258:                                       ; preds = %if.else253
  %152 = load i16* %control, align 2, !dbg !256
  %conv259 = zext i16 %152 to i32, !dbg !256
  %shr = ashr i32 %conv259, 2, !dbg !256
  %and260 = and i32 %shr, 3, !dbg !256
  %call261 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([4 x %struct.tok]* @llc_supervisory_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8]* @.str15, i32 0, i32 0), i32 %and260), !dbg !256
  %153 = load i16* %control, align 2, !dbg !258
  %conv262 = zext i16 %153 to i32, !dbg !258
  %shr263 = ashr i32 %conv262, 9, !dbg !258
  %and264 = and i32 %shr263, 127, !dbg !258
  %154 = load i8* %ssap_field, align 1, !dbg !259
  %conv265 = zext i8 %154 to i32, !dbg !259
  %and266 = and i32 %conv265, 1, !dbg !259
  %155 = load i16* %control, align 2, !dbg !259
  %conv267 = zext i16 %155 to i32, !dbg !259
  %and268 = and i32 %conv267, 256, !dbg !259
  %or269 = or i32 %and266, %and268, !dbg !259
  %call270 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8]* @.str15, i32 0, i32 0), i32 %or269), !dbg !259
  %156 = load i32* %length.addr, align 4, !dbg !258
  %call271 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([51 x i8]* @.str17, i32 0, i32 0), i8* %call261, i32 %and264, i8* %call270, i32 %156), !dbg !258
  br label %if.end286, !dbg !260

if.else272:                                       ; preds = %if.else253
  %157 = load i16* %control, align 2, !dbg !261
  %conv273 = zext i16 %157 to i32, !dbg !261
  %shr274 = ashr i32 %conv273, 1, !dbg !261
  %and275 = and i32 %shr274, 127, !dbg !261
  %158 = load i16* %control, align 2, !dbg !261
  %conv276 = zext i16 %158 to i32, !dbg !261
  %shr277 = ashr i32 %conv276, 9, !dbg !261
  %and278 = and i32 %shr277, 127, !dbg !261
  %159 = load i8* %ssap_field, align 1, !dbg !263
  %conv279 = zext i8 %159 to i32, !dbg !263
  %and280 = and i32 %conv279, 1, !dbg !263
  %160 = load i16* %control, align 2, !dbg !263
  %conv281 = zext i16 %160 to i32, !dbg !263
  %and282 = and i32 %conv281, 256, !dbg !263
  %or283 = or i32 %and280, %and282, !dbg !263
  %call284 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8]* @.str15, i32 0, i32 0), i32 %or283), !dbg !263
  %161 = load i32* %length.addr, align 4, !dbg !261
  %call285 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([60 x i8]* @.str18, i32 0, i32 0), i32 %and275, i32 %and278, i8* %call284, i32 %161), !dbg !261
  br label %if.end286

if.end286:                                        ; preds = %if.else272, %if.then258
  %162 = load i8 addrspace(200)** %p.addr, align 32, !dbg !264
  %add.ptr287 = getelementptr inbounds i8 addrspace(200)* %162, i64 4, !dbg !264
  store i8 addrspace(200)* %add.ptr287, i8 addrspace(200)** %p.addr, align 32, !dbg !264
  %163 = load i32* %length.addr, align 4, !dbg !265
  %sub288 = sub i32 %163, 4, !dbg !265
  store i32 %sub288, i32* %length.addr, align 4, !dbg !265
  %164 = load i32* %caplen.addr, align 4, !dbg !266
  %sub289 = sub i32 %164, 4, !dbg !266
  store i32 %sub289, i32* %caplen.addr, align 4, !dbg !266
  br label %if.end290

if.end290:                                        ; preds = %if.end286, %if.end252
  store i32 1, i32* %retval, !dbg !267
  br label %return, !dbg !267

return:                                           ; preds = %if.end290, %if.then170, %if.then148, %if.end135, %if.end107, %if.then87, %if.then73, %if.end29, %if.then8, %if.then
  %165 = load i32* %retval, !dbg !268
  ret i32 %165, !dbg !268
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata) #1

declare i32 @printf(i8*, ...) #2

declare void @default_print(i8 addrspace(200)*, i32) #2

declare void @ipx_print(i8 addrspace(200)*, i32) #2

declare i8* @tok2str(%struct.tok*, i8*, i32) #2

declare void @stp_print(i8 addrspace(200)*, i32) #2

declare void @ip_print(%struct.netdissect_options*, i64, i8 addrspace(200)*, i32) #2

declare void @netbeui_print(i16 zeroext, i64, i8 addrspace(200)*, i32) #2

declare void @isoclns_print(i8 addrspace(200)*, i32, i32) #2

; Function Attrs: nounwind
define i32 @snap_print(i8 addrspace(200)* %p, i32 %length, i32 %caplen, i32 %bridge_pad) #0 {
entry:
  %retval = alloca i32, align 4
  %p.addr = alloca i8 addrspace(200)*, align 32
  %length.addr = alloca i32, align 4
  %caplen.addr = alloca i32, align 4
  %bridge_pad.addr = alloca i32, align 4
  %orgcode = alloca i32, align 4
  %et = alloca i16, align 2
  %ret = alloca i32, align 4
  %tok = alloca %struct.tok*, align 8
  %otp = alloca %struct.oui_tok*, align 8
  store i8 addrspace(200)* %p, i8 addrspace(200)** %p.addr, align 32
  call void @llvm.dbg.declare(metadata !{i8 addrspace(200)** %p.addr}, metadata !269), !dbg !270
  store i32 %length, i32* %length.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %length.addr}, metadata !271), !dbg !270
  store i32 %caplen, i32* %caplen.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %caplen.addr}, metadata !272), !dbg !270
  store i32 %bridge_pad, i32* %bridge_pad.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %bridge_pad.addr}, metadata !273), !dbg !270
  call void @llvm.dbg.declare(metadata !{i32* %orgcode}, metadata !274), !dbg !275
  call void @llvm.dbg.declare(metadata !{i16* %et}, metadata !276), !dbg !277
  call void @llvm.dbg.declare(metadata !{i32* %ret}, metadata !278), !dbg !279
  %0 = load i8 addrspace(200)** %p.addr, align 32, !dbg !280
  %1 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %0), !dbg !280
  %cmp = icmp ule i64 5, %1, !dbg !280
  br i1 %cmp, label %if.end, label %if.then, !dbg !280

if.then:                                          ; preds = %entry
  br label %trunc, !dbg !280

if.end:                                           ; preds = %entry
  %2 = load i8 addrspace(200)** %p.addr, align 32, !dbg !282
  %3 = addrspacecast i8 addrspace(200)* %2 to i8*, !dbg !282
  %add.ptr = getelementptr inbounds i8* %3, i64 0, !dbg !282
  %4 = load i8* %add.ptr, align 1, !dbg !282
  %conv = zext i8 %4 to i32, !dbg !282
  %shl = shl i32 %conv, 16, !dbg !282
  %5 = load i8 addrspace(200)** %p.addr, align 32, !dbg !282
  %6 = addrspacecast i8 addrspace(200)* %5 to i8*, !dbg !282
  %add.ptr1 = getelementptr inbounds i8* %6, i64 1, !dbg !282
  %7 = load i8* %add.ptr1, align 1, !dbg !282
  %conv2 = zext i8 %7 to i32, !dbg !282
  %shl3 = shl i32 %conv2, 8, !dbg !282
  %or = or i32 %shl, %shl3, !dbg !282
  %8 = load i8 addrspace(200)** %p.addr, align 32, !dbg !282
  %9 = addrspacecast i8 addrspace(200)* %8 to i8*, !dbg !282
  %add.ptr4 = getelementptr inbounds i8* %9, i64 2, !dbg !282
  %10 = load i8* %add.ptr4, align 1, !dbg !282
  %conv5 = zext i8 %10 to i32, !dbg !282
  %or6 = or i32 %or, %conv5, !dbg !282
  store i32 %or6, i32* %orgcode, align 4, !dbg !282
  %11 = load i8 addrspace(200)** %p.addr, align 32, !dbg !283
  %12 = ptrtoint i8 addrspace(200)* %11 to i64, !dbg !283
  %add = add i64 %12, 3, !dbg !283
  %13 = inttoptr i64 %add to i8 addrspace(200)*, !dbg !283
  %call = call zeroext i16 @EXTRACT_16BITS(i8 addrspace(200)* %13), !dbg !283
  store i16 %call, i16* %et, align 2, !dbg !283
  %14 = load %struct.netdissect_options** @gndo, align 8, !dbg !284
  %ndo_eflag = getelementptr inbounds %struct.netdissect_options* %14, i32 0, i32 2, !dbg !284
  %15 = load i32* %ndo_eflag, align 4, !dbg !284
  %tobool = icmp ne i32 %15, 0, !dbg !284
  br i1 %tobool, label %if.then7, label %if.end23, !dbg !284

if.then7:                                         ; preds = %if.end
  call void @llvm.dbg.declare(metadata !{%struct.tok** %tok}, metadata !286), !dbg !288
  store %struct.tok* getelementptr inbounds ([1 x %struct.tok]* @null_values, i32 0, i32 0), %struct.tok** %tok, align 8, !dbg !288
  call void @llvm.dbg.declare(metadata !{%struct.oui_tok** %otp}, metadata !289), !dbg !291
  store %struct.oui_tok* getelementptr inbounds ([6 x %struct.oui_tok]* @oui_to_tok, i32 0, i64 0), %struct.oui_tok** %otp, align 8, !dbg !292
  br label %for.cond, !dbg !292

for.cond:                                         ; preds = %for.inc, %if.then7
  %16 = load %struct.oui_tok** %otp, align 8, !dbg !292
  %tok8 = getelementptr inbounds %struct.oui_tok* %16, i32 0, i32 1, !dbg !292
  %17 = load %struct.tok** %tok8, align 8, !dbg !292
  %cmp9 = icmp ne %struct.tok* %17, null, !dbg !292
  br i1 %cmp9, label %for.body, label %for.end, !dbg !292

for.body:                                         ; preds = %for.cond
  %18 = load %struct.oui_tok** %otp, align 8, !dbg !294
  %oui = getelementptr inbounds %struct.oui_tok* %18, i32 0, i32 0, !dbg !294
  %19 = load i32* %oui, align 4, !dbg !294
  %20 = load i32* %orgcode, align 4, !dbg !294
  %cmp11 = icmp eq i32 %19, %20, !dbg !294
  br i1 %cmp11, label %if.then13, label %if.end15, !dbg !294

if.then13:                                        ; preds = %for.body
  %21 = load %struct.oui_tok** %otp, align 8, !dbg !297
  %tok14 = getelementptr inbounds %struct.oui_tok* %21, i32 0, i32 1, !dbg !297
  %22 = load %struct.tok** %tok14, align 8, !dbg !297
  store %struct.tok* %22, %struct.tok** %tok, align 8, !dbg !297
  br label %for.end, !dbg !299

if.end15:                                         ; preds = %for.body
  br label %for.inc, !dbg !300

for.inc:                                          ; preds = %if.end15
  %23 = load %struct.oui_tok** %otp, align 8, !dbg !292
  %incdec.ptr = getelementptr inbounds %struct.oui_tok* %23, i32 1, !dbg !292
  store %struct.oui_tok* %incdec.ptr, %struct.oui_tok** %otp, align 8, !dbg !292
  br label %for.cond, !dbg !292

for.end:                                          ; preds = %if.then13, %for.cond
  %24 = load i32* %orgcode, align 4, !dbg !301
  %call16 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([0 x %struct.tok]* @oui_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8]* @.str3, i32 0, i32 0), i32 %24), !dbg !301
  %25 = load i32* %orgcode, align 4, !dbg !302
  %26 = load i32* %orgcode, align 4, !dbg !302
  %cmp17 = icmp eq i32 %26, 0, !dbg !302
  %cond = select i1 %cmp17, i8* getelementptr inbounds ([10 x i8]* @.str20, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8]* @.str21, i32 0, i32 0), !dbg !302
  %27 = load %struct.tok** %tok, align 8, !dbg !303
  %28 = load i16* %et, align 2, !dbg !303
  %conv19 = zext i16 %28 to i32, !dbg !303
  %call20 = call i8* @tok2str(%struct.tok* %27, i8* getelementptr inbounds ([8 x i8]* @.str3, i32 0, i32 0), i32 %conv19), !dbg !303
  %29 = load i16* %et, align 2, !dbg !302
  %conv21 = zext i16 %29 to i32, !dbg !302
  %call22 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([34 x i8]* @.str19, i32 0, i32 0), i8* %call16, i32 %25, i8* %cond, i8* %call20, i32 %conv21), !dbg !302
  br label %if.end23, !dbg !304

if.end23:                                         ; preds = %for.end, %if.end
  %30 = load i8 addrspace(200)** %p.addr, align 32, !dbg !305
  %add.ptr24 = getelementptr inbounds i8 addrspace(200)* %30, i64 5, !dbg !305
  store i8 addrspace(200)* %add.ptr24, i8 addrspace(200)** %p.addr, align 32, !dbg !305
  %31 = load i32* %length.addr, align 4, !dbg !306
  %sub = sub i32 %31, 5, !dbg !306
  store i32 %sub, i32* %length.addr, align 4, !dbg !306
  %32 = load i32* %caplen.addr, align 4, !dbg !307
  %sub25 = sub i32 %32, 5, !dbg !307
  store i32 %sub25, i32* %caplen.addr, align 4, !dbg !307
  %33 = load i32* %orgcode, align 4, !dbg !308
  switch i32 %33, label %sw.epilog85 [
    i32 0, label %sw.bb
    i32 248, label %sw.bb
    i32 524295, label %sw.bb30
    i32 12, label %sw.bb40
    i32 32962, label %sw.bb47
  ], !dbg !308

sw.bb:                                            ; preds = %if.end23, %if.end23
  %34 = load %struct.netdissect_options** @gndo, align 8, !dbg !309
  %35 = load i16* %et, align 2, !dbg !309
  %36 = load i8 addrspace(200)** %p.addr, align 32, !dbg !309
  %37 = load i32* %length.addr, align 4, !dbg !309
  %38 = load i32* %caplen.addr, align 4, !dbg !309
  %call26 = call i32 @ethertype_print(%struct.netdissect_options* %34, i16 zeroext %35, i8 addrspace(200)* %36, i32 %37, i32 %38), !dbg !309
  store i32 %call26, i32* %ret, align 4, !dbg !309
  %39 = load i32* %ret, align 4, !dbg !311
  %tobool27 = icmp ne i32 %39, 0, !dbg !311
  br i1 %tobool27, label %if.then28, label %if.end29, !dbg !311

if.then28:                                        ; preds = %sw.bb
  %40 = load i32* %ret, align 4, !dbg !313
  store i32 %40, i32* %retval, !dbg !313
  br label %return, !dbg !313

if.end29:                                         ; preds = %sw.bb
  br label %sw.epilog85, !dbg !314

sw.bb30:                                          ; preds = %if.end23
  %41 = load i16* %et, align 2, !dbg !315
  %conv31 = zext i16 %41 to i32, !dbg !315
  %cmp32 = icmp eq i32 %conv31, 32923, !dbg !315
  br i1 %cmp32, label %if.then34, label %if.end39, !dbg !315

if.then34:                                        ; preds = %sw.bb30
  %42 = load %struct.netdissect_options** @gndo, align 8, !dbg !317
  %43 = load i16* %et, align 2, !dbg !317
  %44 = load i8 addrspace(200)** %p.addr, align 32, !dbg !317
  %45 = load i32* %length.addr, align 4, !dbg !317
  %46 = load i32* %caplen.addr, align 4, !dbg !317
  %call35 = call i32 @ethertype_print(%struct.netdissect_options* %42, i16 zeroext %43, i8 addrspace(200)* %44, i32 %45, i32 %46), !dbg !317
  store i32 %call35, i32* %ret, align 4, !dbg !317
  %47 = load i32* %ret, align 4, !dbg !319
  %tobool36 = icmp ne i32 %47, 0, !dbg !319
  br i1 %tobool36, label %if.then37, label %if.end38, !dbg !319

if.then37:                                        ; preds = %if.then34
  %48 = load i32* %ret, align 4, !dbg !321
  store i32 %48, i32* %retval, !dbg !321
  br label %return, !dbg !321

if.end38:                                         ; preds = %if.then34
  br label %if.end39, !dbg !322

if.end39:                                         ; preds = %if.end38, %sw.bb30
  br label %sw.epilog85, !dbg !323

sw.bb40:                                          ; preds = %if.end23
  %49 = load i16* %et, align 2, !dbg !324
  %conv41 = zext i16 %49 to i32, !dbg !324
  switch i32 %conv41, label %sw.default [
    i32 8192, label %sw.bb42
    i32 8196, label %sw.bb43
    i32 273, label %sw.bb44
    i32 8195, label %sw.bb45
    i32 267, label %sw.bb46
  ], !dbg !324

sw.bb42:                                          ; preds = %sw.bb40
  %50 = load i8 addrspace(200)** %p.addr, align 32, !dbg !325
  %51 = load i32* %length.addr, align 4, !dbg !325
  %52 = load i32* %caplen.addr, align 4, !dbg !325
  call void @cdp_print(i8 addrspace(200)* %50, i32 %51, i32 %52), !dbg !325
  store i32 1, i32* %retval, !dbg !327
  br label %return, !dbg !327

sw.bb43:                                          ; preds = %sw.bb40
  %53 = load i8 addrspace(200)** %p.addr, align 32, !dbg !328
  %54 = load i32* %length.addr, align 4, !dbg !328
  call void @dtp_print(i8 addrspace(200)* %53, i32 %54), !dbg !328
  store i32 1, i32* %retval, !dbg !329
  br label %return, !dbg !329

sw.bb44:                                          ; preds = %sw.bb40
  %55 = load i8 addrspace(200)** %p.addr, align 32, !dbg !330
  %56 = load i32* %length.addr, align 4, !dbg !330
  call void @udld_print(i8 addrspace(200)* %55, i32 %56), !dbg !330
  store i32 1, i32* %retval, !dbg !331
  br label %return, !dbg !331

sw.bb45:                                          ; preds = %sw.bb40
  %57 = load i8 addrspace(200)** %p.addr, align 32, !dbg !332
  %58 = load i32* %length.addr, align 4, !dbg !332
  call void @vtp_print(i8 addrspace(200)* %57, i32 %58), !dbg !332
  store i32 1, i32* %retval, !dbg !333
  br label %return, !dbg !333

sw.bb46:                                          ; preds = %sw.bb40
  %59 = load i8 addrspace(200)** %p.addr, align 32, !dbg !334
  %60 = load i32* %length.addr, align 4, !dbg !334
  call void @stp_print(i8 addrspace(200)* %59, i32 %60), !dbg !334
  store i32 1, i32* %retval, !dbg !335
  br label %return, !dbg !335

sw.default:                                       ; preds = %sw.bb40
  br label %sw.epilog, !dbg !336

sw.epilog:                                        ; preds = %sw.default
  br label %sw.bb47, !dbg !337

sw.bb47:                                          ; preds = %if.end23, %sw.epilog
  %61 = load i16* %et, align 2, !dbg !338
  %conv48 = zext i16 %61 to i32, !dbg !338
  switch i32 %conv48, label %sw.epilog84 [
    i32 1, label %sw.bb49
    i32 7, label %sw.bb49
    i32 3, label %sw.bb58
    i32 9, label %sw.bb58
    i32 4, label %sw.bb69
    i32 10, label %sw.bb69
    i32 14, label %sw.bb83
  ], !dbg !338

sw.bb49:                                          ; preds = %sw.bb47, %sw.bb47
  %62 = load i32* %bridge_pad.addr, align 4, !dbg !339
  %conv50 = zext i32 %62 to i64, !dbg !339
  %63 = load i8 addrspace(200)** %p.addr, align 32, !dbg !339
  %64 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %63), !dbg !339
  %cmp51 = icmp ule i64 %conv50, %64, !dbg !339
  br i1 %cmp51, label %if.end54, label %if.then53, !dbg !339

if.then53:                                        ; preds = %sw.bb49
  br label %trunc, !dbg !339

if.end54:                                         ; preds = %sw.bb49
  %65 = load i32* %bridge_pad.addr, align 4, !dbg !342
  %66 = load i32* %caplen.addr, align 4, !dbg !342
  %sub55 = sub i32 %66, %65, !dbg !342
  store i32 %sub55, i32* %caplen.addr, align 4, !dbg !342
  %67 = load i32* %bridge_pad.addr, align 4, !dbg !343
  %68 = load i32* %length.addr, align 4, !dbg !343
  %sub56 = sub i32 %68, %67, !dbg !343
  store i32 %sub56, i32* %length.addr, align 4, !dbg !343
  %69 = load i32* %bridge_pad.addr, align 4, !dbg !344
  %70 = load i8 addrspace(200)** %p.addr, align 32, !dbg !344
  %idx.ext = zext i32 %69 to i64, !dbg !344
  %add.ptr57 = getelementptr inbounds i8 addrspace(200)* %70, i64 %idx.ext, !dbg !344
  store i8 addrspace(200)* %add.ptr57, i8 addrspace(200)** %p.addr, align 32, !dbg !344
  %71 = load %struct.netdissect_options** @gndo, align 8, !dbg !345
  %72 = load i8 addrspace(200)** %p.addr, align 32, !dbg !345
  %73 = load i32* %length.addr, align 4, !dbg !345
  %74 = load i32* %caplen.addr, align 4, !dbg !345
  call void @ether_print(%struct.netdissect_options* %71, i64 undef, i8 addrspace(200)* %72, i32 %73, i32 %74, void (%struct.netdissect_options*, i64, i8 addrspace(200)*)* null, i64 undef, i8 addrspace(200)* null), !dbg !345
  store i32 1, i32* %retval, !dbg !346
  br label %return, !dbg !346

sw.bb58:                                          ; preds = %sw.bb47, %sw.bb47
  %75 = load i32* %bridge_pad.addr, align 4, !dbg !347
  %conv59 = zext i32 %75 to i64, !dbg !347
  %76 = load i8 addrspace(200)** %p.addr, align 32, !dbg !347
  %77 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %76), !dbg !347
  %cmp60 = icmp ule i64 %conv59, %77, !dbg !347
  br i1 %cmp60, label %if.end63, label %if.then62, !dbg !347

if.then62:                                        ; preds = %sw.bb58
  br label %trunc, !dbg !347

if.end63:                                         ; preds = %sw.bb58
  %78 = load i32* %bridge_pad.addr, align 4, !dbg !349
  %79 = load i32* %caplen.addr, align 4, !dbg !349
  %sub64 = sub i32 %79, %78, !dbg !349
  store i32 %sub64, i32* %caplen.addr, align 4, !dbg !349
  %80 = load i32* %bridge_pad.addr, align 4, !dbg !350
  %81 = load i32* %length.addr, align 4, !dbg !350
  %sub65 = sub i32 %81, %80, !dbg !350
  store i32 %sub65, i32* %length.addr, align 4, !dbg !350
  %82 = load i32* %bridge_pad.addr, align 4, !dbg !351
  %83 = load i8 addrspace(200)** %p.addr, align 32, !dbg !351
  %idx.ext66 = zext i32 %82 to i64, !dbg !351
  %add.ptr67 = getelementptr inbounds i8 addrspace(200)* %83, i64 %idx.ext66, !dbg !351
  store i8 addrspace(200)* %add.ptr67, i8 addrspace(200)** %p.addr, align 32, !dbg !351
  %84 = load i8 addrspace(200)** %p.addr, align 32, !dbg !352
  %85 = load i32* %length.addr, align 4, !dbg !352
  %86 = load i32* %caplen.addr, align 4, !dbg !352
  %call68 = call i32 @token_print(i8 addrspace(200)* %84, i32 %85, i32 %86), !dbg !352
  store i32 1, i32* %retval, !dbg !353
  br label %return, !dbg !353

sw.bb69:                                          ; preds = %sw.bb47, %sw.bb47
  %87 = load i32* %bridge_pad.addr, align 4, !dbg !354
  %add70 = add i32 %87, 1, !dbg !354
  %conv71 = zext i32 %add70 to i64, !dbg !354
  %88 = load i8 addrspace(200)** %p.addr, align 32, !dbg !354
  %89 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %88), !dbg !354
  %cmp72 = icmp ule i64 %conv71, %89, !dbg !354
  br i1 %cmp72, label %if.end75, label %if.then74, !dbg !354

if.then74:                                        ; preds = %sw.bb69
  br label %trunc, !dbg !354

if.end75:                                         ; preds = %sw.bb69
  %90 = load i32* %bridge_pad.addr, align 4, !dbg !356
  %add76 = add i32 %90, 1, !dbg !356
  %91 = load i32* %caplen.addr, align 4, !dbg !356
  %sub77 = sub i32 %91, %add76, !dbg !356
  store i32 %sub77, i32* %caplen.addr, align 4, !dbg !356
  %92 = load i32* %bridge_pad.addr, align 4, !dbg !357
  %add78 = add i32 %92, 1, !dbg !357
  %93 = load i32* %length.addr, align 4, !dbg !357
  %sub79 = sub i32 %93, %add78, !dbg !357
  store i32 %sub79, i32* %length.addr, align 4, !dbg !357
  %94 = load i32* %bridge_pad.addr, align 4, !dbg !358
  %add80 = add i32 %94, 1, !dbg !358
  %95 = load i8 addrspace(200)** %p.addr, align 32, !dbg !358
  %idx.ext81 = zext i32 %add80 to i64, !dbg !358
  %add.ptr82 = getelementptr inbounds i8 addrspace(200)* %95, i64 %idx.ext81, !dbg !358
  store i8 addrspace(200)* %add.ptr82, i8 addrspace(200)** %p.addr, align 32, !dbg !358
  %96 = load i8 addrspace(200)** %p.addr, align 32, !dbg !359
  %97 = load i32* %length.addr, align 4, !dbg !359
  %98 = load i32* %caplen.addr, align 4, !dbg !359
  call void @fddi_print(i8 addrspace(200)* %96, i32 %97, i32 %98), !dbg !359
  store i32 1, i32* %retval, !dbg !360
  br label %return, !dbg !360

sw.bb83:                                          ; preds = %sw.bb47
  %99 = load i8 addrspace(200)** %p.addr, align 32, !dbg !361
  %100 = load i32* %length.addr, align 4, !dbg !361
  call void @stp_print(i8 addrspace(200)* %99, i32 %100), !dbg !361
  store i32 1, i32* %retval, !dbg !362
  br label %return, !dbg !362

sw.epilog84:                                      ; preds = %sw.bb47
  br label %sw.epilog85, !dbg !363

sw.epilog85:                                      ; preds = %sw.epilog84, %if.end23, %if.end39, %if.end29
  store i32 0, i32* %retval, !dbg !364
  br label %return, !dbg !364

trunc:                                            ; preds = %if.then74, %if.then62, %if.then53, %if.then
  %call86 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([8 x i8]* @.str22, i32 0, i32 0)), !dbg !365
  store i32 1, i32* %retval, !dbg !366
  br label %return, !dbg !366

return:                                           ; preds = %trunc, %sw.epilog85, %sw.bb83, %if.end75, %if.end63, %if.end54, %sw.bb46, %sw.bb45, %sw.bb44, %sw.bb43, %sw.bb42, %if.then37, %if.then28
  %101 = load i32* %retval, !dbg !367
  ret i32 %101, !dbg !367
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)*, i64) #1

declare i8* @etheraddr_string(i8 addrspace(200)*) #2

; Function Attrs: nounwind readnone
declare i64 @llvm.mips.get.cap.length(i8 addrspace(200)*) #1

; Function Attrs: inlinehint nounwind
define internal zeroext i16 @EXTRACT_16BITS(i8 addrspace(200)* %p) #3 {
entry:
  %p.addr = alloca i8 addrspace(200)*, align 32
  store i8 addrspace(200)* %p, i8 addrspace(200)** %p.addr, align 32
  call void @llvm.dbg.declare(metadata !{i8 addrspace(200)** %p.addr}, metadata !368), !dbg !369
  %0 = load i8 addrspace(200)** %p.addr, align 32, !dbg !370
  %1 = addrspacecast i8 addrspace(200)* %0 to %struct.unaligned_u_int16_t*, !dbg !370
  %val = getelementptr inbounds %struct.unaligned_u_int16_t* %1, i32 0, i32 0, !dbg !370
  %2 = load i16* %val, align 1, !dbg !370
  ret i16 %2, !dbg !370
}

declare i32 @ethertype_print(%struct.netdissect_options*, i16 zeroext, i8 addrspace(200)*, i32, i32) #2

declare void @cdp_print(i8 addrspace(200)*, i32, i32) #2

declare void @dtp_print(i8 addrspace(200)*, i32) #2

declare void @udld_print(i8 addrspace(200)*, i32) #2

declare void @vtp_print(i8 addrspace(200)*, i32) #2

declare void @ether_print(%struct.netdissect_options*, i64, i8 addrspace(200)*, i32, i32, void (%struct.netdissect_options*, i64, i8 addrspace(200)*)*, i64, i8 addrspace(200)*) #2

declare i32 @token_print(i8 addrspace(200)*, i32, i32) #2

declare void @fddi_print(i8 addrspace(200)*, i32, i32) #2

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { inlinehint nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!84, !85}
!llvm.ident = !{!86}

!0 = metadata !{i32 786449, metadata !1, i32 12, metadata !"clang version 3.5 ", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !32, metadata !2, metadata !""} ; [ DW_TAG_compile_unit ] [/Users/theraven/Documents/Research/cheri/print-llc.c] [DW_LANG_C99]
!1 = metadata !{metadata !"print-llc.c", metadata !"/Users/theraven/Documents/Research/cheri"}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4, metadata !20, metadata !23}
!4 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"llc_print", metadata !"llc_print", metadata !"", i32 152, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i8 addrspace(200)*, i32, i32, i8 addrspace(200)*, i8 addrspace(200)*, i16*)* @llc_print, null, null, metadata !2, i32 154} ; [ DW_TAG_subprogram ] [line 152] [def] [scope 154] [llc_print]
!5 = metadata !{metadata !"/home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c", metadata !"/Users/theraven/Documents/Research/cheri"}
!6 = metadata !{i32 786473, metadata !5}          ; [ DW_TAG_file_type ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!7 = metadata !{i32 786453, i32 0, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{metadata !9, metadata !10, metadata !15, metadata !15, metadata !10, metadata !10, metadata !17}
!9 = metadata !{i32 786468, null, null, metadata !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!10 = metadata !{i32 786454, metadata !5, null, metadata !"packetbody_t", i32 51, i64 0, i64 0, i64 0, i32 0, metadata !11} ; [ DW_TAG_typedef ] [packetbody_t] [line 51, size 0, align 0, offset 0] [from ]
!11 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 256, i64 256, i64 0, i32 0, metadata !12} ; [ DW_TAG_pointer_type ] [line 0, size 256, align 256, offset 0] [from ]
!12 = metadata !{i32 786470, null, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, metadata !13} ; [ DW_TAG_const_type ] [line 0, size 0, align 0, offset 0] [from u_char]
!13 = metadata !{i32 786454, metadata !5, null, metadata !"u_char", i32 50, i64 0, i64 0, i64 0, i32 0, metadata !14} ; [ DW_TAG_typedef ] [u_char] [line 50, size 0, align 0, offset 0] [from unsigned char]
!14 = metadata !{i32 786468, null, null, metadata !"unsigned char", i32 0, i64 8, i64 8, i64 0, i32 0, i32 8} ; [ DW_TAG_base_type ] [unsigned char] [line 0, size 8, align 8, offset 0, enc DW_ATE_unsigned_char]
!15 = metadata !{i32 786454, metadata !5, null, metadata !"u_int", i32 52, i64 0, i64 0, i64 0, i32 0, metadata !16} ; [ DW_TAG_typedef ] [u_int] [line 52, size 0, align 0, offset 0] [from unsigned int]
!16 = metadata !{i32 786468, null, null, metadata !"unsigned int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ] [unsigned int] [line 0, size 32, align 32, offset 0, enc DW_ATE_unsigned]
!17 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !18} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from u_short]
!18 = metadata !{i32 786454, metadata !5, null, metadata !"u_short", i32 51, i64 0, i64 0, i64 0, i32 0, metadata !19} ; [ DW_TAG_typedef ] [u_short] [line 51, size 0, align 0, offset 0] [from unsigned short]
!19 = metadata !{i32 786468, null, null, metadata !"unsigned short", i32 0, i64 16, i64 16, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ] [unsigned short] [line 0, size 16, align 16, offset 0, enc DW_ATE_unsigned]
!20 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"snap_print", metadata !"snap_print", metadata !"", i32 382, metadata !21, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i8 addrspace(200)*, i32, i32, i32)* @snap_print, null, null, metadata !2, i32 383} ; [ DW_TAG_subprogram ] [line 382] [def] [scope 383] [snap_print]
!21 = metadata !{i32 786453, i32 0, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !22, i32 0, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!22 = metadata !{metadata !9, metadata !10, metadata !15, metadata !15, metadata !15}
!23 = metadata !{i32 786478, metadata !24, metadata !25, metadata !"EXTRACT_16BITS", metadata !"EXTRACT_16BITS", metadata !"", i32 57, metadata !26, i1 true, i1 true, i32 0, i32 0, null, i32 256, i1 false, i16 (i8 addrspace(200)*)* @EXTRACT_16BITS, null, null, metadata !2, i32 58} ; [ DW_TAG_subprogram ] [line 57] [local] [def] [scope 58] [EXTRACT_16BITS]
!24 = metadata !{metadata !"/home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/extract.h", metadata !"/Users/theraven/Documents/Research/cheri"}
!25 = metadata !{i32 786473, metadata !24}        ; [ DW_TAG_file_type ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/extract.h]
!26 = metadata !{i32 786453, i32 0, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !27, i32 0, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!27 = metadata !{metadata !28, metadata !30}
!28 = metadata !{i32 786454, metadata !5, null, metadata !"u_int16_t", i32 66, i64 0, i64 0, i64 0, i32 0, metadata !29} ; [ DW_TAG_typedef ] [u_int16_t] [line 66, size 0, align 0, offset 0] [from __uint16_t]
!29 = metadata !{i32 786454, metadata !5, null, metadata !"__uint16_t", i32 53, i64 0, i64 0, i64 0, i32 0, metadata !19} ; [ DW_TAG_typedef ] [__uint16_t] [line 53, size 0, align 0, offset 0] [from unsigned short]
!30 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 256, i64 256, i64 0, i32 0, metadata !31} ; [ DW_TAG_pointer_type ] [line 0, size 256, align 256, offset 0] [from ]
!31 = metadata !{i32 786470, null, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null} ; [ DW_TAG_const_type ] [line 0, size 0, align 0, offset 0] [from ]
!32 = metadata !{metadata !33, metadata !54, metadata !58, metadata !60, metadata !64, metadata !68, metadata !72, metadata !76, metadata !80}
!33 = metadata !{i32 786484, i32 0, null, metadata !"oui_to_tok", metadata !"oui_to_tok", metadata !"", metadata !6, i32 139, metadata !34, i32 1, i32 1, [6 x %struct.oui_tok]* @oui_to_tok, null} ; [ DW_TAG_variable ] [oui_to_tok] [line 139] [local] [def]
!34 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 768, i64 64, i32 0, i32 0, metadata !35, metadata !52, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 768, align 64, offset 0] [from ]
!35 = metadata !{i32 786470, null, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, metadata !36} ; [ DW_TAG_const_type ] [line 0, size 0, align 0, offset 0] [from oui_tok]
!36 = metadata !{i32 786451, metadata !5, null, metadata !"oui_tok", i32 134, i64 128, i64 64, i32 0, i32 0, null, metadata !37, i32 0, null, null, null} ; [ DW_TAG_structure_type ] [oui_tok] [line 134, size 128, align 64, offset 0] [def] [from ]
!37 = metadata !{metadata !38, metadata !41}
!38 = metadata !{i32 786445, metadata !5, metadata !36, metadata !"oui", i32 135, i64 32, i64 32, i64 0, i32 0, metadata !39} ; [ DW_TAG_member ] [oui] [line 135, size 32, align 32, offset 0] [from u_int32_t]
!39 = metadata !{i32 786454, metadata !5, null, metadata !"u_int32_t", i32 67, i64 0, i64 0, i64 0, i32 0, metadata !40} ; [ DW_TAG_typedef ] [u_int32_t] [line 67, size 0, align 0, offset 0] [from __uint32_t]
!40 = metadata !{i32 786454, metadata !5, null, metadata !"__uint32_t", i32 55, i64 0, i64 0, i64 0, i32 0, metadata !16} ; [ DW_TAG_typedef ] [__uint32_t] [line 55, size 0, align 0, offset 0] [from unsigned int]
!41 = metadata !{i32 786445, metadata !5, metadata !36, metadata !"tok", i32 136, i64 64, i64 64, i64 64, i32 0, metadata !42} ; [ DW_TAG_member ] [tok] [line 136, size 64, align 64, offset 64] [from ]
!42 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !43} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!43 = metadata !{i32 786470, null, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, metadata !44} ; [ DW_TAG_const_type ] [line 0, size 0, align 0, offset 0] [from tok]
!44 = metadata !{i32 786451, metadata !45, null, metadata !"tok", i32 71, i64 128, i64 64, i32 0, i32 0, null, metadata !46, i32 0, null, null, null} ; [ DW_TAG_structure_type ] [tok] [line 71, size 128, align 64, offset 0] [def] [from ]
!45 = metadata !{metadata !"/home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/netdissect.h", metadata !"/Users/theraven/Documents/Research/cheri"}
!46 = metadata !{metadata !47, metadata !48}
!47 = metadata !{i32 786445, metadata !45, metadata !44, metadata !"v", i32 72, i64 32, i64 32, i64 0, i32 0, metadata !9} ; [ DW_TAG_member ] [v] [line 72, size 32, align 32, offset 0] [from int]
!48 = metadata !{i32 786445, metadata !45, metadata !44, metadata !"s", i32 73, i64 64, i64 64, i64 64, i32 0, metadata !49} ; [ DW_TAG_member ] [s] [line 73, size 64, align 64, offset 64] [from ]
!49 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !50} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!50 = metadata !{i32 786470, null, null, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, metadata !51} ; [ DW_TAG_const_type ] [line 0, size 0, align 0, offset 0] [from char]
!51 = metadata !{i32 786468, null, null, metadata !"char", i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ] [char] [line 0, size 8, align 8, offset 0, enc DW_ATE_signed_char]
!52 = metadata !{metadata !53}
!53 = metadata !{i32 786465, i64 0, i64 6}        ; [ DW_TAG_subrange_type ] [0, 5]
!54 = metadata !{i32 786484, i32 0, null, metadata !"bridged_values", metadata !"bridged_values", metadata !"", metadata !6, i32 115, metadata !55, i32 1, i32 1, [12 x %struct.tok]* @bridged_values, null} ; [ DW_TAG_variable ] [bridged_values] [line 115] [local] [def]
!55 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 1536, i64 64, i32 0, i32 0, metadata !43, metadata !56, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 1536, align 64, offset 0] [from ]
!56 = metadata !{metadata !57}
!57 = metadata !{i32 786465, i64 0, i64 12}       ; [ DW_TAG_subrange_type ] [0, 11]
!58 = metadata !{i32 786484, i32 0, null, metadata !"cisco_values", metadata !"cisco_values", metadata !"", metadata !6, i32 106, metadata !59, i32 1, i32 1, [6 x %struct.tok]* @cisco_values, null} ; [ DW_TAG_variable ] [cisco_values] [line 106] [local] [def]
!59 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 768, i64 64, i32 0, i32 0, metadata !43, metadata !52, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 768, align 64, offset 0] [from ]
!60 = metadata !{i32 786484, i32 0, null, metadata !"null_values", metadata !"null_values", metadata !"", metadata !6, i32 130, metadata !61, i32 1, i32 1, [1 x %struct.tok]* @null_values, null} ; [ DW_TAG_variable ] [null_values] [line 130] [local] [def]
!61 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 128, i64 64, i32 0, i32 0, metadata !43, metadata !62, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 128, align 64, offset 0] [from ]
!62 = metadata !{metadata !63}
!63 = metadata !{i32 786465, i64 0, i64 1}        ; [ DW_TAG_subrange_type ] [0, 0]
!64 = metadata !{i32 786484, i32 0, null, metadata !"llc_supervisory_values", metadata !"llc_supervisory_values", metadata !"", metadata !6, i32 98, metadata !65, i32 1, i32 1, [4 x %struct.tok]* @llc_supervisory_values, null} ; [ DW_TAG_variable ] [llc_supervisory_values] [line 98] [local] [def]
!65 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 512, i64 64, i32 0, i32 0, metadata !43, metadata !66, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 512, align 64, offset 0] [from ]
!66 = metadata !{metadata !67}
!67 = metadata !{i32 786465, i64 0, i64 4}        ; [ DW_TAG_subrange_type ] [0, 3]
!68 = metadata !{i32 786484, i32 0, null, metadata !"llc_cmd_values", metadata !"llc_cmd_values", metadata !"", metadata !6, i32 68, metadata !69, i32 1, i32 1, [9 x %struct.tok]* @llc_cmd_values, null} ; [ DW_TAG_variable ] [llc_cmd_values] [line 68] [local] [def]
!69 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 1152, i64 64, i32 0, i32 0, metadata !44, metadata !70, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 1152, align 64, offset 0] [from tok]
!70 = metadata !{metadata !71}
!71 = metadata !{i32 786465, i64 0, i64 9}        ; [ DW_TAG_subrange_type ] [0, 8]
!72 = metadata !{i32 786484, i32 0, null, metadata !"llc_flag_values", metadata !"llc_flag_values", metadata !"", metadata !6, i32 80, metadata !73, i32 1, i32 1, [7 x %struct.tok]* @llc_flag_values, null} ; [ DW_TAG_variable ] [llc_flag_values] [line 80] [local] [def]
!73 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 896, i64 64, i32 0, i32 0, metadata !43, metadata !74, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 896, align 64, offset 0] [from ]
!74 = metadata !{metadata !75}
!75 = metadata !{i32 786465, i64 0, i64 7}        ; [ DW_TAG_subrange_type ] [0, 6]
!76 = metadata !{i32 786484, i32 0, null, metadata !"llc_ig_flag_values", metadata !"llc_ig_flag_values", metadata !"", metadata !6, i32 91, metadata !77, i32 1, i32 1, [3 x %struct.tok]* @llc_ig_flag_values, null} ; [ DW_TAG_variable ] [llc_ig_flag_values] [line 91] [local] [def]
!77 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 384, i64 64, i32 0, i32 0, metadata !43, metadata !78, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 384, align 64, offset 0] [from ]
!78 = metadata !{metadata !79}
!79 = metadata !{i32 786465, i64 0, i64 3}        ; [ DW_TAG_subrange_type ] [0, 2]
!80 = metadata !{i32 786484, i32 0, null, metadata !"llc_values", metadata !"llc_values", metadata !"", metadata !6, i32 49, metadata !81, i32 1, i32 1, [16 x %struct.tok]* @llc_values, null} ; [ DW_TAG_variable ] [llc_values] [line 49] [local] [def]
!81 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 2048, i64 64, i32 0, i32 0, metadata !44, metadata !82, i32 0, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 2048, align 64, offset 0] [from tok]
!82 = metadata !{metadata !83}
!83 = metadata !{i32 786465, i64 0, i64 16}       ; [ DW_TAG_subrange_type ] [0, 15]
!84 = metadata !{i32 2, metadata !"Dwarf Version", i32 4}
!85 = metadata !{i32 1, metadata !"Debug Info Version", i32 1}
!86 = metadata !{metadata !"clang version 3.5 "}
!87 = metadata !{i32 786689, metadata !4, metadata !"p", metadata !6, i32 16777368, metadata !10, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [p] [line 152]
!88 = metadata !{i32 152, i32 0, metadata !4, null}
!89 = metadata !{i32 786689, metadata !4, metadata !"length", metadata !6, i32 33554584, metadata !15, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [length] [line 152]
!90 = metadata !{i32 786689, metadata !4, metadata !"caplen", metadata !6, i32 50331800, metadata !15, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [caplen] [line 152]
!91 = metadata !{i32 786689, metadata !4, metadata !"esrc", metadata !6, i32 67109017, metadata !10, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [esrc] [line 153]
!92 = metadata !{i32 153, i32 0, metadata !4, null}
!93 = metadata !{i32 786689, metadata !4, metadata !"edst", metadata !6, i32 83886233, metadata !10, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [edst] [line 153]
!94 = metadata !{i32 786689, metadata !4, metadata !"extracted_ethertype", metadata !6, i32 100663449, metadata !17, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [extracted_ethertype] [line 153]
!95 = metadata !{i32 786688, metadata !4, metadata !"dsap_field", metadata !6, i32 155, metadata !96, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [dsap_field] [line 155]
!96 = metadata !{i32 786454, metadata !5, null, metadata !"u_int8_t", i32 65, i64 0, i64 0, i64 0, i32 0, metadata !97} ; [ DW_TAG_typedef ] [u_int8_t] [line 65, size 0, align 0, offset 0] [from __uint8_t]
!97 = metadata !{i32 786454, metadata !5, null, metadata !"__uint8_t", i32 51, i64 0, i64 0, i64 0, i32 0, metadata !14} ; [ DW_TAG_typedef ] [__uint8_t] [line 51, size 0, align 0, offset 0] [from unsigned char]
!98 = metadata !{i32 155, i32 0, metadata !4, null}
!99 = metadata !{i32 786688, metadata !4, metadata !"dsap", metadata !6, i32 155, metadata !96, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [dsap] [line 155]
!100 = metadata !{i32 786688, metadata !4, metadata !"ssap_field", metadata !6, i32 155, metadata !96, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [ssap_field] [line 155]
!101 = metadata !{i32 786688, metadata !4, metadata !"ssap", metadata !6, i32 155, metadata !96, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [ssap] [line 155]
!102 = metadata !{i32 786688, metadata !4, metadata !"control", metadata !6, i32 156, metadata !28, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [control] [line 156]
!103 = metadata !{i32 156, i32 0, metadata !4, null}
!104 = metadata !{i32 786688, metadata !4, metadata !"is_u", metadata !6, i32 157, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [is_u] [line 157]
!105 = metadata !{i32 157, i32 0, metadata !4, null}
!106 = metadata !{i32 786688, metadata !4, metadata !"ret", metadata !6, i32 158, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [ret] [line 158]
!107 = metadata !{i32 158, i32 0, metadata !4, null}
!108 = metadata !{i32 160, i32 0, metadata !4, null}
!109 = metadata !{i32 162, i32 0, metadata !110, null}
!110 = metadata !{i32 786443, metadata !5, metadata !4, i32 162, i32 0, i32 0} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!111 = metadata !{i32 163, i32 0, metadata !112, null}
!112 = metadata !{i32 786443, metadata !5, metadata !110, i32 162, i32 0, i32 1} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!113 = metadata !{i32 164, i32 0, metadata !112, null}
!114 = metadata !{i32 165, i32 0, metadata !112, null}
!115 = metadata !{i32 168, i32 0, metadata !4, null}
!116 = metadata !{i32 169, i32 0, metadata !4, null}
!117 = metadata !{i32 177, i32 0, metadata !4, null}
!118 = metadata !{i32 178, i32 0, metadata !119, null}
!119 = metadata !{i32 786443, metadata !5, metadata !4, i32 178, i32 0, i32 2} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!120 = metadata !{i32 182, i32 0, metadata !121, null}
!121 = metadata !{i32 786443, metadata !5, metadata !119, i32 178, i32 0, i32 3} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!122 = metadata !{i32 183, i32 0, metadata !121, null}
!123 = metadata !{i32 188, i32 0, metadata !124, null}
!124 = metadata !{i32 786443, metadata !5, metadata !125, i32 188, i32 0, i32 5} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!125 = metadata !{i32 786443, metadata !5, metadata !119, i32 183, i32 0, i32 4} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!126 = metadata !{i32 189, i32 0, metadata !127, null}
!127 = metadata !{i32 786443, metadata !5, metadata !124, i32 188, i32 0, i32 6} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!128 = metadata !{i32 190, i32 0, metadata !127, null}
!129 = metadata !{i32 191, i32 0, metadata !127, null}
!130 = metadata !{i32 197, i32 0, metadata !125, null}
!131 = metadata !{i32 198, i32 0, metadata !125, null}
!132 = metadata !{i32 201, i32 0, metadata !133, null}
!133 = metadata !{i32 786443, metadata !5, metadata !4, i32 201, i32 0, i32 7} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!134 = metadata !{i32 217, i32 0, metadata !135, null}
!135 = metadata !{i32 786443, metadata !5, metadata !136, i32 217, i32 0, i32 9} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!136 = metadata !{i32 786443, metadata !5, metadata !133, i32 201, i32 0, i32 8} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!137 = metadata !{i32 218, i32 0, metadata !135, null}
!138 = metadata !{i32 220, i32 0, metadata !136, null}
!139 = metadata !{i32 221, i32 0, metadata !136, null}
!140 = metadata !{i32 224, i32 0, metadata !4, null}
!141 = metadata !{i32 225, i32 0, metadata !4, null}
!142 = metadata !{i32 227, i32 0, metadata !143, null}
!143 = metadata !{i32 786443, metadata !5, metadata !4, i32 227, i32 0, i32 10} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!144 = metadata !{i32 229, i32 0, metadata !145, null}
!145 = metadata !{i32 786443, metadata !5, metadata !143, i32 227, i32 0, i32 11} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!146 = metadata !{i32 228, i32 0, metadata !145, null}
!147 = metadata !{i32 231, i32 0, metadata !145, null}
!148 = metadata !{i32 232, i32 0, metadata !145, null}
!149 = metadata !{i32 234, i32 0, metadata !145, null}
!150 = metadata !{i32 236, i32 0, metadata !151, null}
!151 = metadata !{i32 786443, metadata !5, metadata !145, i32 236, i32 0, i32 12} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!152 = metadata !{i32 237, i32 0, metadata !153, null}
!153 = metadata !{i32 786443, metadata !5, metadata !151, i32 236, i32 0, i32 13} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!154 = metadata !{i32 238, i32 0, metadata !153, null}
!155 = metadata !{i32 239, i32 0, metadata !156, null}
!156 = metadata !{i32 786443, metadata !5, metadata !151, i32 238, i32 0, i32 14} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!157 = metadata !{i32 241, i32 0, metadata !145, null}
!158 = metadata !{i32 243, i32 0, metadata !159, null}
!159 = metadata !{i32 786443, metadata !5, metadata !4, i32 243, i32 0, i32 15} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!160 = metadata !{i32 245, i32 0, metadata !161, null}
!161 = metadata !{i32 786443, metadata !5, metadata !159, i32 244, i32 0, i32 16} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!162 = metadata !{i32 246, i32 0, metadata !161, null}
!163 = metadata !{i32 249, i32 0, metadata !164, null}
!164 = metadata !{i32 786443, metadata !5, metadata !4, i32 249, i32 0, i32 17} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!165 = metadata !{i32 251, i32 0, metadata !166, null}
!166 = metadata !{i32 786443, metadata !5, metadata !164, i32 250, i32 0, i32 18} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!167 = metadata !{i32 252, i32 0, metadata !166, null}
!168 = metadata !{i32 255, i32 0, metadata !169, null}
!169 = metadata !{i32 786443, metadata !5, metadata !4, i32 255, i32 0, i32 19} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!170 = metadata !{i32 264, i32 0, metadata !171, null}
!171 = metadata !{i32 786443, metadata !5, metadata !172, i32 264, i32 0, i32 21} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!172 = metadata !{i32 786443, metadata !5, metadata !169, i32 256, i32 0, i32 20} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!173 = metadata !{i32 265, i32 0, metadata !171, null}
!174 = metadata !{i32 267, i32 0, metadata !172, null}
!175 = metadata !{i32 268, i32 0, metadata !172, null}
!176 = metadata !{i32 272, i32 0, metadata !177, null}
!177 = metadata !{i32 786443, metadata !5, metadata !4, i32 272, i32 0, i32 22} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!178 = metadata !{i32 288, i32 0, metadata !179, null}
!179 = metadata !{i32 786443, metadata !5, metadata !180, i32 288, i32 0, i32 24} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!180 = metadata !{i32 786443, metadata !5, metadata !177, i32 273, i32 0, i32 23} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!181 = metadata !{i32 289, i32 0, metadata !182, null}
!182 = metadata !{i32 786443, metadata !5, metadata !179, i32 288, i32 0, i32 25} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!183 = metadata !{i32 290, i32 0, metadata !182, null}
!184 = metadata !{i32 291, i32 0, metadata !182, null}
!185 = metadata !{i32 292, i32 0, metadata !182, null}
!186 = metadata !{i32 293, i32 0, metadata !187, null}
!187 = metadata !{i32 786443, metadata !5, metadata !179, i32 292, i32 0, i32 26} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!188 = metadata !{i32 294, i32 0, metadata !187, null}
!189 = metadata !{i32 295, i32 0, metadata !187, null}
!190 = metadata !{i32 297, i32 0, metadata !180, null}
!191 = metadata !{i32 298, i32 0, metadata !180, null}
!192 = metadata !{i32 301, i32 0, metadata !193, null}
!193 = metadata !{i32 786443, metadata !5, metadata !4, i32 301, i32 0, i32 27} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!194 = metadata !{i32 303, i32 0, metadata !195, null}
!195 = metadata !{i32 786443, metadata !5, metadata !193, i32 302, i32 0, i32 28} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!196 = metadata !{i32 304, i32 0, metadata !195, null}
!197 = metadata !{i32 307, i32 0, metadata !198, null}
!198 = metadata !{i32 786443, metadata !5, metadata !4, i32 307, i32 0, i32 29} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!199 = metadata !{i32 314, i32 0, metadata !200, null}
!200 = metadata !{i32 786443, metadata !5, metadata !198, i32 308, i32 0, i32 30} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!201 = metadata !{i32 315, i32 0, metadata !202, null}
!202 = metadata !{i32 786443, metadata !5, metadata !200, i32 315, i32 0, i32 31} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!203 = metadata !{i32 316, i32 0, metadata !202, null}
!204 = metadata !{i32 317, i32 0, metadata !200, null}
!205 = metadata !{i32 319, i32 0, metadata !206, null}
!206 = metadata !{i32 786443, metadata !5, metadata !4, i32 319, i32 0, i32 32} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!207 = metadata !{i32 320, i32 0, metadata !208, null}
!208 = metadata !{i32 786443, metadata !5, metadata !209, i32 320, i32 0, i32 34} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!209 = metadata !{i32 786443, metadata !5, metadata !206, i32 319, i32 0, i32 33} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!210 = metadata !{i32 321, i32 0, metadata !211, null}
!211 = metadata !{i32 786443, metadata !5, metadata !212, i32 321, i32 0, i32 36} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!212 = metadata !{i32 786443, metadata !5, metadata !208, i32 320, i32 0, i32 35} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!213 = metadata !{i32 322, i32 0, metadata !211, null}
!214 = metadata !{i32 325, i32 0, metadata !211, null}
!215 = metadata !{i32 326, i32 0, metadata !211, null}
!216 = metadata !{i32 327, i32 0, metadata !211, null}
!217 = metadata !{i32 324, i32 0, metadata !211, null}
!218 = metadata !{i32 328, i32 0, metadata !212, null}
!219 = metadata !{i32 329, i32 0, metadata !220, null}
!220 = metadata !{i32 786443, metadata !5, metadata !221, i32 329, i32 0, i32 38} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!221 = metadata !{i32 786443, metadata !5, metadata !208, i32 328, i32 0, i32 37} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!222 = metadata !{i32 331, i32 0, metadata !220, null}
!223 = metadata !{i32 332, i32 0, metadata !220, null}
!224 = metadata !{i32 330, i32 0, metadata !220, null}
!225 = metadata !{i32 335, i32 0, metadata !220, null}
!226 = metadata !{i32 336, i32 0, metadata !220, null}
!227 = metadata !{i32 337, i32 0, metadata !220, null}
!228 = metadata !{i32 338, i32 0, metadata !220, null}
!229 = metadata !{i32 334, i32 0, metadata !220, null}
!230 = metadata !{i32 340, i32 0, metadata !209, null}
!231 = metadata !{i32 342, i32 0, metadata !232, null}
!232 = metadata !{i32 786443, metadata !5, metadata !4, i32 342, i32 0, i32 39} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!233 = metadata !{i32 344, i32 0, metadata !234, null}
!234 = metadata !{i32 786443, metadata !5, metadata !232, i32 342, i32 0, i32 40} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!235 = metadata !{i32 345, i32 0, metadata !234, null}
!236 = metadata !{i32 343, i32 0, metadata !234, null}
!237 = metadata !{i32 348, i32 0, metadata !234, null}
!238 = metadata !{i32 349, i32 0, metadata !234, null}
!239 = metadata !{i32 350, i32 0, metadata !234, null}
!240 = metadata !{i32 352, i32 0, metadata !241, null}
!241 = metadata !{i32 786443, metadata !5, metadata !234, i32 352, i32 0, i32 41} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!242 = metadata !{i32 353, i32 0, metadata !243, null}
!243 = metadata !{i32 786443, metadata !5, metadata !244, i32 353, i32 0, i32 43} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!244 = metadata !{i32 786443, metadata !5, metadata !241, i32 352, i32 0, i32 42} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!245 = metadata !{i32 354, i32 0, metadata !246, null}
!246 = metadata !{i32 786443, metadata !5, metadata !243, i32 353, i32 0, i32 44} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!247 = metadata !{i32 355, i32 0, metadata !246, null}
!248 = metadata !{i32 356, i32 0, metadata !246, null}
!249 = metadata !{i32 357, i32 0, metadata !246, null}
!250 = metadata !{i32 358, i32 0, metadata !246, null}
!251 = metadata !{i32 359, i32 0, metadata !244, null}
!252 = metadata !{i32 360, i32 0, metadata !234, null}
!253 = metadata !{i32 361, i32 0, metadata !254, null}
!254 = metadata !{i32 786443, metadata !5, metadata !255, i32 361, i32 0, i32 46} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!255 = metadata !{i32 786443, metadata !5, metadata !232, i32 360, i32 0, i32 45} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!256 = metadata !{i32 363, i32 0, metadata !257, null}
!257 = metadata !{i32 786443, metadata !5, metadata !254, i32 361, i32 0, i32 47} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!258 = metadata !{i32 362, i32 0, metadata !257, null}
!259 = metadata !{i32 365, i32 0, metadata !257, null}
!260 = metadata !{i32 367, i32 0, metadata !257, null}
!261 = metadata !{i32 368, i32 0, metadata !262, null}
!262 = metadata !{i32 786443, metadata !5, metadata !254, i32 367, i32 0, i32 48} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!263 = metadata !{i32 371, i32 0, metadata !262, null}
!264 = metadata !{i32 374, i32 0, metadata !255, null}
!265 = metadata !{i32 375, i32 0, metadata !255, null}
!266 = metadata !{i32 376, i32 0, metadata !255, null}
!267 = metadata !{i32 378, i32 0, metadata !4, null}
!268 = metadata !{i32 379, i32 0, metadata !4, null}
!269 = metadata !{i32 786689, metadata !20, metadata !"p", metadata !6, i32 16777598, metadata !10, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [p] [line 382]
!270 = metadata !{i32 382, i32 0, metadata !20, null}
!271 = metadata !{i32 786689, metadata !20, metadata !"length", metadata !6, i32 33554814, metadata !15, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [length] [line 382]
!272 = metadata !{i32 786689, metadata !20, metadata !"caplen", metadata !6, i32 50332030, metadata !15, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [caplen] [line 382]
!273 = metadata !{i32 786689, metadata !20, metadata !"bridge_pad", metadata !6, i32 67109246, metadata !15, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [bridge_pad] [line 382]
!274 = metadata !{i32 786688, metadata !20, metadata !"orgcode", metadata !6, i32 384, metadata !39, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [orgcode] [line 384]
!275 = metadata !{i32 384, i32 0, metadata !20, null}
!276 = metadata !{i32 786688, metadata !20, metadata !"et", metadata !6, i32 385, metadata !18, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [et] [line 385]
!277 = metadata !{i32 385, i32 0, metadata !20, null}
!278 = metadata !{i32 786688, metadata !20, metadata !"ret", metadata !6, i32 386, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [ret] [line 386]
!279 = metadata !{i32 386, i32 0, metadata !20, null}
!280 = metadata !{i32 388, i32 0, metadata !281, null}
!281 = metadata !{i32 786443, metadata !5, metadata !20, i32 388, i32 0, i32 49} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!282 = metadata !{i32 389, i32 0, metadata !20, null}
!283 = metadata !{i32 390, i32 7, metadata !20, null}
!284 = metadata !{i32 392, i32 0, metadata !285, null}
!285 = metadata !{i32 786443, metadata !5, metadata !20, i32 392, i32 0, i32 50} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!286 = metadata !{i32 786688, metadata !287, metadata !"tok", metadata !6, i32 393, metadata !42, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [tok] [line 393]
!287 = metadata !{i32 786443, metadata !5, metadata !285, i32 392, i32 0, i32 51} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!288 = metadata !{i32 393, i32 0, metadata !287, null}
!289 = metadata !{i32 786688, metadata !287, metadata !"otp", metadata !6, i32 394, metadata !290, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [otp] [line 394]
!290 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !35} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!291 = metadata !{i32 394, i32 0, metadata !287, null}
!292 = metadata !{i32 396, i32 0, metadata !293, null}
!293 = metadata !{i32 786443, metadata !5, metadata !287, i32 396, i32 0, i32 52} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!294 = metadata !{i32 397, i32 0, metadata !295, null}
!295 = metadata !{i32 786443, metadata !5, metadata !296, i32 397, i32 0, i32 54} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!296 = metadata !{i32 786443, metadata !5, metadata !293, i32 396, i32 0, i32 53} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!297 = metadata !{i32 398, i32 0, metadata !298, null}
!298 = metadata !{i32 786443, metadata !5, metadata !295, i32 397, i32 0, i32 55} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!299 = metadata !{i32 399, i32 0, metadata !298, null}
!300 = metadata !{i32 401, i32 0, metadata !296, null}
!301 = metadata !{i32 403, i32 0, metadata !287, null}
!302 = metadata !{i32 402, i32 0, metadata !287, null}
!303 = metadata !{i32 406, i32 0, metadata !287, null}
!304 = metadata !{i32 408, i32 0, metadata !287, null}
!305 = metadata !{i32 409, i32 0, metadata !20, null}
!306 = metadata !{i32 410, i32 0, metadata !20, null}
!307 = metadata !{i32 411, i32 0, metadata !20, null}
!308 = metadata !{i32 413, i32 0, metadata !20, null}
!309 = metadata !{i32 422, i32 0, metadata !310, null}
!310 = metadata !{i32 786443, metadata !5, metadata !20, i32 413, i32 0, i32 56} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!311 = metadata !{i32 423, i32 0, metadata !312, null}
!312 = metadata !{i32 786443, metadata !5, metadata !310, i32 423, i32 0, i32 57} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!313 = metadata !{i32 424, i32 0, metadata !312, null}
!314 = metadata !{i32 425, i32 0, metadata !310, null}
!315 = metadata !{i32 428, i32 0, metadata !316, null}
!316 = metadata !{i32 786443, metadata !5, metadata !310, i32 428, i32 0, i32 58} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!317 = metadata !{i32 437, i32 0, metadata !318, null}
!318 = metadata !{i32 786443, metadata !5, metadata !316, i32 428, i32 0, i32 59} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!319 = metadata !{i32 438, i32 0, metadata !320, null}
!320 = metadata !{i32 786443, metadata !5, metadata !318, i32 438, i32 0, i32 60} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!321 = metadata !{i32 439, i32 0, metadata !320, null}
!322 = metadata !{i32 440, i32 0, metadata !318, null}
!323 = metadata !{i32 441, i32 0, metadata !310, null}
!324 = metadata !{i32 444, i32 0, metadata !310, null}
!325 = metadata !{i32 446, i32 0, metadata !326, null}
!326 = metadata !{i32 786443, metadata !5, metadata !310, i32 444, i32 0, i32 61} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!327 = metadata !{i32 447, i32 0, metadata !326, null}
!328 = metadata !{i32 449, i32 0, metadata !326, null}
!329 = metadata !{i32 450, i32 0, metadata !326, null}
!330 = metadata !{i32 452, i32 0, metadata !326, null}
!331 = metadata !{i32 453, i32 0, metadata !326, null}
!332 = metadata !{i32 455, i32 0, metadata !326, null}
!333 = metadata !{i32 456, i32 0, metadata !326, null}
!334 = metadata !{i32 458, i32 0, metadata !326, null}
!335 = metadata !{i32 459, i32 0, metadata !326, null}
!336 = metadata !{i32 461, i32 0, metadata !326, null}
!337 = metadata !{i32 462, i32 0, metadata !326, null}
!338 = metadata !{i32 465, i32 0, metadata !310, null}
!339 = metadata !{i32 476, i32 0, metadata !340, null}
!340 = metadata !{i32 786443, metadata !5, metadata !341, i32 476, i32 0, i32 63} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!341 = metadata !{i32 786443, metadata !5, metadata !310, i32 465, i32 0, i32 62} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!342 = metadata !{i32 477, i32 0, metadata !341, null}
!343 = metadata !{i32 478, i32 0, metadata !341, null}
!344 = metadata !{i32 479, i32 0, metadata !341, null}
!345 = metadata !{i32 484, i32 0, metadata !341, null}
!346 = metadata !{i32 485, i32 0, metadata !341, null}
!347 = metadata !{i32 497, i32 0, metadata !348, null}
!348 = metadata !{i32 786443, metadata !5, metadata !341, i32 497, i32 0, i32 64} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!349 = metadata !{i32 498, i32 0, metadata !341, null}
!350 = metadata !{i32 499, i32 0, metadata !341, null}
!351 = metadata !{i32 500, i32 0, metadata !341, null}
!352 = metadata !{i32 506, i32 0, metadata !341, null}
!353 = metadata !{i32 507, i32 0, metadata !341, null}
!354 = metadata !{i32 518, i32 0, metadata !355, null}
!355 = metadata !{i32 786443, metadata !5, metadata !341, i32 518, i32 0, i32 65} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/print-llc.c]
!356 = metadata !{i32 519, i32 0, metadata !341, null}
!357 = metadata !{i32 520, i32 0, metadata !341, null}
!358 = metadata !{i32 521, i32 0, metadata !341, null}
!359 = metadata !{i32 526, i32 0, metadata !341, null}
!360 = metadata !{i32 527, i32 0, metadata !341, null}
!361 = metadata !{i32 530, i32 0, metadata !341, null}
!362 = metadata !{i32 531, i32 0, metadata !341, null}
!363 = metadata !{i32 533, i32 0, metadata !310, null}
!364 = metadata !{i32 534, i32 0, metadata !20, null}
!365 = metadata !{i32 537, i32 0, metadata !20, null}
!366 = metadata !{i32 538, i32 0, metadata !20, null}
!367 = metadata !{i32 539, i32 0, metadata !20, null}
!368 = metadata !{i32 786689, metadata !23, metadata !"p", metadata !25, i32 16777273, metadata !30, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [p] [line 57]
!369 = metadata !{i32 57, i32 0, metadata !23, null}
!370 = metadata !{i32 59, i32 0, metadata !371, null}
!371 = metadata !{i32 786443, metadata !24, metadata !23} ; [ DW_TAG_lexical_block ] [/Users/theraven/Documents/Research/cheri//home/rwatson/git/cheribsd/libexec/tcpdump-helper/../../contrib/tcpdump/extract.h]
