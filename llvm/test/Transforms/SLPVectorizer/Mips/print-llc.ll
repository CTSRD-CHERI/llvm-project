; Check that this doesn't crash the vectoriser...
; RUN: %cheri_opt -O2 %s -o /dev/null
; ModuleID = 'print-llc.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
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
@llc_values = internal global [16 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str56, i32 0, i32 0) }, %struct.tok { i32 255, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str57, i32 0, i32 0) }, %struct.tok { i32 2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str58, i32 0, i32 0) }, %struct.tok { i32 3, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str59, i32 0, i32 0) }, %struct.tok { i32 6, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str60, i32 0, i32 0) }, %struct.tok { i32 4, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str61, i32 0, i32 0) }, %struct.tok { i32 14, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str62, i32 0, i32 0) }, %struct.tok { i32 66, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str63, i32 0, i32 0) }, %struct.tok { i32 78, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str64, i32 0, i32 0) }, %struct.tok { i32 126, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str65, i32 0, i32 0) }, %struct.tok { i32 142, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str66, i32 0, i32 0) }, %struct.tok { i32 170, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str67, i32 0, i32 0) }, %struct.tok { i32 224, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str68, i32 0, i32 0) }, %struct.tok { i32 240, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str69, i32 0, i32 0) }, %struct.tok { i32 254, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str70, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str3 = private unnamed_addr constant [8 x i8] c"Unknown\00", align 1
@llc_ig_flag_values = internal constant [3 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str54, i32 0, i32 0) }, %struct.tok { i32 1, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str55, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@llc_flag_values = internal constant [7 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str50, i32 0, i32 0) }, %struct.tok { i32 1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str51, i32 0, i32 0) }, %struct.tok { i32 16, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str52, i32 0, i32 0) }, %struct.tok { i32 17, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str53, i32 0, i32 0) }, %struct.tok { i32 256, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str52, i32 0, i32 0) }, %struct.tok { i32 257, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str53, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
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
@llc_cmd_values = internal global [9 x %struct.tok] [%struct.tok { i32 3, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str42, i32 0, i32 0) }, %struct.tok { i32 227, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str43, i32 0, i32 0) }, %struct.tok { i32 175, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str44, i32 0, i32 0) }, %struct.tok { i32 99, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str45, i32 0, i32 0) }, %struct.tok { i32 67, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str46, i32 0, i32 0) }, %struct.tok { i32 15, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str47, i32 0, i32 0) }, %struct.tok { i32 111, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str48, i32 0, i32 0) }, %struct.tok { i32 135, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str49, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str14 = private unnamed_addr constant [5 x i8] c"%02x\00", align 1
@.str15 = private unnamed_addr constant [2 x i8] c"?\00", align 1
@.str16 = private unnamed_addr constant [12 x i8] c": %02x %02x\00", align 1
@.str17 = private unnamed_addr constant [51 x i8] c"Supervisory, %s, rcv seq %u, Flags [%s], length %u\00", align 1
@llc_supervisory_values = internal constant [4 x %struct.tok] [%struct.tok { i32 0, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str39, i32 0, i32 0) }, %struct.tok { i32 1, i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str40, i32 0, i32 0) }, %struct.tok { i32 2, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str41, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@.str18 = private unnamed_addr constant [60 x i8] c"Information, send seq %u, rcv seq %u, Flags [%s], length %u\00", align 1
@null_values = internal constant [1 x %struct.tok] zeroinitializer, align 8
@oui_to_tok = internal constant [6 x %struct.oui_tok] [%struct.oui_tok { i32 0, %struct.tok* getelementptr inbounds ([0 x %struct.tok], [0 x %struct.tok]* @ethertype_values, i32 0, i32 0) }, %struct.oui_tok { i32 248, %struct.tok* getelementptr inbounds ([0 x %struct.tok], [0 x %struct.tok]* @ethertype_values, i32 0, i32 0) }, %struct.oui_tok { i32 524295, %struct.tok* getelementptr inbounds ([0 x %struct.tok], [0 x %struct.tok]* @ethertype_values, i32 0, i32 0) }, %struct.oui_tok { i32 12, %struct.tok* getelementptr inbounds ([6 x %struct.tok], [6 x %struct.tok]* @cisco_values, i32 0, i32 0) }, %struct.oui_tok { i32 32962, %struct.tok* getelementptr inbounds ([12 x %struct.tok], [12 x %struct.tok]* @bridged_values, i32 0, i32 0) }, %struct.oui_tok zeroinitializer], align 8
@.str19 = private unnamed_addr constant [34 x i8] c"oui %s (0x%06x), %s %s (0x%04x): \00", align 1
@oui_values = external constant [0 x %struct.tok]
@.str20 = private unnamed_addr constant [10 x i8] c"ethertype\00", align 1
@.str21 = private unnamed_addr constant [4 x i8] c"pid\00", align 1
@.str22 = private unnamed_addr constant [8 x i8] c"[|snap]\00", align 1
@ethertype_values = external constant [0 x %struct.tok]
@cisco_values = internal constant [6 x %struct.tok] [%struct.tok { i32 8192, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str34, i32 0, i32 0) }, %struct.tok { i32 8195, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str35, i32 0, i32 0) }, %struct.tok { i32 8196, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str36, i32 0, i32 0) }, %struct.tok { i32 273, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str37, i32 0, i32 0) }, %struct.tok { i32 267, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str38, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
@bridged_values = internal constant [12 x %struct.tok] [%struct.tok { i32 1, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str23, i32 0, i32 0) }, %struct.tok { i32 7, i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str24, i32 0, i32 0) }, %struct.tok { i32 2, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str25, i32 0, i32 0) }, %struct.tok { i32 8, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str26, i32 0, i32 0) }, %struct.tok { i32 3, i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str27, i32 0, i32 0) }, %struct.tok { i32 9, i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str28, i32 0, i32 0) }, %struct.tok { i32 4, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str29, i32 0, i32 0) }, %struct.tok { i32 10, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str30, i32 0, i32 0) }, %struct.tok { i32 5, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str31, i32 0, i32 0) }, %struct.tok { i32 11, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str32, i32 0, i32 0) }, %struct.tok { i32 14, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str33, i32 0, i32 0) }, %struct.tok zeroinitializer], align 8
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
  store i32 %length, i32* %length.addr, align 4
  store i32 %caplen, i32* %caplen.addr, align 4
  store i8 addrspace(200)* %esrc, i8 addrspace(200)** %esrc.addr, align 32
  store i8 addrspace(200)* %edst, i8 addrspace(200)** %edst.addr, align 32
  store i16* %extracted_ethertype, i16** %extracted_ethertype.addr, align 8
  %0 = load i16*, i16** %extracted_ethertype.addr, align 8
  store i16 0, i16* %0, align 2
  %1 = load i32, i32* %caplen.addr, align 4
  %cmp = icmp ult i32 %1, 3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0))
  %2 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %3 = load i32, i32* %caplen.addr, align 4
  call void @default_print(i8 addrspace(200)* %2, i32 %3)
  store i32 0, i32* %retval
  br label %return

if.end:                                           ; preds = %entry
  %4 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %5 = load i8, i8 addrspace(200)* %4, align 1
  store i8 %5, i8* %dsap_field, align 1
  %6 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %7 = ptrtoint i8 addrspace(200)* %6 to i64
  %add = add i64 %7, 1
  %8 = inttoptr i64 %add to i8 addrspace(200)*
  %9 = load i8, i8 addrspace(200)* %8, align 1
  store i8 %9, i8* %ssap_field, align 1
  %10 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %11 = ptrtoint i8 addrspace(200)* %10 to i64
  %add1 = add i64 %11, 2
  %12 = inttoptr i64 %add1 to i8 addrspace(200)*
  %13 = load i8, i8 addrspace(200)* %12, align 1
  %conv = zext i8 %13 to i16
  store i16 %conv, i16* %control, align 2
  %14 = load i16, i16* %control, align 2
  %conv2 = zext i16 %14 to i32
  %and = and i32 %conv2, 3
  %cmp3 = icmp eq i32 %and, 3
  br i1 %cmp3, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.end
  store i32 1, i32* %is_u, align 4
  br label %if.end19

if.else:                                          ; preds = %if.end
  %15 = load i32, i32* %caplen.addr, align 4
  %cmp6 = icmp ult i32 %15, 4
  br i1 %cmp6, label %if.then8, label %if.end10

if.then8:                                         ; preds = %if.else
  %call9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0))
  %16 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %17 = load i32, i32* %caplen.addr, align 4
  call void @default_print(i8 addrspace(200)* %16, i32 %17)
  store i32 0, i32* %retval
  br label %return

if.end10:                                         ; preds = %if.else
  %18 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %19 = ptrtoint i8 addrspace(200)* %18 to i64
  %add11 = add i64 %19, 2
  %20 = inttoptr i64 %add11 to i8 addrspace(200)*
  %21 = addrspacecast i8 addrspace(200)* %20 to i8*
  %add.ptr = getelementptr inbounds i8, i8* %21, i64 1
  %22 = load i8, i8* %add.ptr, align 1
  %conv12 = zext i8 %22 to i16
  %conv13 = zext i16 %conv12 to i32
  %shl = shl i32 %conv13, 8
  %23 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %24 = ptrtoint i8 addrspace(200)* %23 to i64
  %add14 = add i64 %24, 2
  %25 = inttoptr i64 %add14 to i8 addrspace(200)*
  %26 = addrspacecast i8 addrspace(200)* %25 to i8*
  %add.ptr15 = getelementptr inbounds i8, i8* %26, i64 0
  %27 = load i8, i8* %add.ptr15, align 1
  %conv16 = zext i8 %27 to i16
  %conv17 = zext i16 %conv16 to i32
  %or = or i32 %shl, %conv17
  %conv18 = trunc i32 %or to i16
  store i16 %conv18, i16* %control, align 2
  store i32 0, i32* %is_u, align 4
  br label %if.end19

if.end19:                                         ; preds = %if.end10, %if.then5
  %28 = load i8, i8* %ssap_field, align 1
  %conv20 = zext i8 %28 to i32
  %cmp21 = icmp eq i32 %conv20, 255
  br i1 %cmp21, label %land.lhs.true, label %if.end30

land.lhs.true:                                    ; preds = %if.end19
  %29 = load i8, i8* %dsap_field, align 1
  %conv23 = zext i8 %29 to i32
  %cmp24 = icmp eq i32 %conv23, 255
  br i1 %cmp24, label %if.then26, label %if.end30

if.then26:                                        ; preds = %land.lhs.true
  %30 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %ndo_eflag = getelementptr inbounds %struct.netdissect_options, %struct.netdissect_options* %30, i32 0, i32 2
  %31 = load i32, i32* %ndo_eflag, align 4
  %tobool = icmp ne i32 %31, 0
  br i1 %tobool, label %if.then27, label %if.end29

if.then27:                                        ; preds = %if.then26
  %call28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str1, i32 0, i32 0))
  br label %if.end29

if.end29:                                         ; preds = %if.then27, %if.then26
  %32 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %33 = load i32, i32* %length.addr, align 4
  call void @ipx_print(i8 addrspace(200)* %32, i32 %33)
  store i32 1, i32* %retval
  br label %return

if.end30:                                         ; preds = %land.lhs.true, %if.end19
  %34 = load i8, i8* %dsap_field, align 1
  %conv31 = zext i8 %34 to i32
  %and32 = and i32 %conv31, -2
  %conv33 = trunc i32 %and32 to i8
  store i8 %conv33, i8* %dsap, align 1
  %35 = load i8, i8* %ssap_field, align 1
  %conv34 = zext i8 %35 to i32
  %and35 = and i32 %conv34, -2
  %conv36 = trunc i32 %and35 to i8
  store i8 %conv36, i8* %ssap, align 1
  %36 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %ndo_eflag37 = getelementptr inbounds %struct.netdissect_options, %struct.netdissect_options* %36, i32 0, i32 2
  %37 = load i32, i32* %ndo_eflag37, align 4
  %tobool38 = icmp ne i32 %37, 0
  br i1 %tobool38, label %if.then39, label %if.end61

if.then39:                                        ; preds = %if.end30
  %38 = load i8, i8* %dsap, align 1
  %conv40 = zext i8 %38 to i32
  %call41 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str3, i32 0, i32 0), i32 %conv40)
  %39 = load i8, i8* %dsap, align 1
  %conv42 = zext i8 %39 to i32
  %40 = load i8, i8* %dsap_field, align 1
  %conv43 = zext i8 %40 to i32
  %and44 = and i32 %conv43, 1
  %call45 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([3 x %struct.tok], [3 x %struct.tok]* @llc_ig_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str3, i32 0, i32 0), i32 %and44)
  %41 = load i8, i8* %ssap, align 1
  %conv46 = zext i8 %41 to i32
  %call47 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str3, i32 0, i32 0), i32 %conv46)
  %42 = load i8, i8* %ssap, align 1
  %conv48 = zext i8 %42 to i32
  %43 = load i8, i8* %ssap_field, align 1
  %conv49 = zext i8 %43 to i32
  %and50 = and i32 %conv49, 1
  %call51 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok], [7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str3, i32 0, i32 0), i32 %and50)
  %call52 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([46 x i8], [46 x i8]* @.str2, i32 0, i32 0), i8* %call41, i32 %conv42, i8* %call45, i8* %call47, i32 %conv48, i8* %call51)
  %44 = load i32, i32* %is_u, align 4
  %tobool53 = icmp ne i32 %44, 0
  br i1 %tobool53, label %if.then54, label %if.else57

if.then54:                                        ; preds = %if.then39
  %45 = load i16, i16* %control, align 2
  %conv55 = zext i16 %45 to i32
  %call56 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str4, i32 0, i32 0), i32 %conv55)
  br label %if.end60

if.else57:                                        ; preds = %if.then39
  %46 = load i16, i16* %control, align 2
  %conv58 = zext i16 %46 to i32
  %call59 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str5, i32 0, i32 0), i32 %conv58)
  br label %if.end60

if.end60:                                         ; preds = %if.else57, %if.then54
  br label %if.end61

if.end61:                                         ; preds = %if.end60, %if.end30
  %47 = load i8, i8* %ssap, align 1
  %conv62 = zext i8 %47 to i32
  %cmp63 = icmp eq i32 %conv62, 66
  br i1 %cmp63, label %land.lhs.true65, label %if.end75

land.lhs.true65:                                  ; preds = %if.end61
  %48 = load i8, i8* %dsap, align 1
  %conv66 = zext i8 %48 to i32
  %cmp67 = icmp eq i32 %conv66, 66
  br i1 %cmp67, label %land.lhs.true69, label %if.end75

land.lhs.true69:                                  ; preds = %land.lhs.true65
  %49 = load i16, i16* %control, align 2
  %conv70 = zext i16 %49 to i32
  %cmp71 = icmp eq i32 %conv70, 3
  br i1 %cmp71, label %if.then73, label %if.end75

if.then73:                                        ; preds = %land.lhs.true69
  %50 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %51 = ptrtoint i8 addrspace(200)* %50 to i64
  %add74 = add i64 %51, 3
  %52 = inttoptr i64 %add74 to i8 addrspace(200)*
  %53 = load i32, i32* %length.addr, align 4
  %sub = sub i32 %53, 3
  call void @stp_print(i8 addrspace(200)* %52, i32 %sub)
  store i32 1, i32* %retval
  br label %return

if.end75:                                         ; preds = %land.lhs.true69, %land.lhs.true65, %if.end61
  %54 = load i8, i8* %ssap, align 1
  %conv76 = zext i8 %54 to i32
  %cmp77 = icmp eq i32 %conv76, 6
  br i1 %cmp77, label %land.lhs.true79, label %if.end90

land.lhs.true79:                                  ; preds = %if.end75
  %55 = load i8, i8* %dsap, align 1
  %conv80 = zext i8 %55 to i32
  %cmp81 = icmp eq i32 %conv80, 6
  br i1 %cmp81, label %land.lhs.true83, label %if.end90

land.lhs.true83:                                  ; preds = %land.lhs.true79
  %56 = load i16, i16* %control, align 2
  %conv84 = zext i16 %56 to i32
  %cmp85 = icmp eq i32 %conv84, 3
  br i1 %cmp85, label %if.then87, label %if.end90

if.then87:                                        ; preds = %land.lhs.true83
  %57 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %58 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %59 = ptrtoint i8 addrspace(200)* %58 to i64
  %add88 = add i64 %59, 4
  %60 = inttoptr i64 %add88 to i8 addrspace(200)*
  %61 = load i32, i32* %length.addr, align 4
  %sub89 = sub i32 %61, 4
  call void @ip_print(%struct.netdissect_options* %57, i64 undef, i8 addrspace(200)* %60, i32 %sub89)
  store i32 1, i32* %retval
  br label %return

if.end90:                                         ; preds = %land.lhs.true83, %land.lhs.true79, %if.end75
  %62 = load i8, i8* %ssap, align 1
  %conv91 = zext i8 %62 to i32
  %cmp92 = icmp eq i32 %conv91, 224
  br i1 %cmp92, label %land.lhs.true94, label %if.end110

land.lhs.true94:                                  ; preds = %if.end90
  %63 = load i8, i8* %dsap, align 1
  %conv95 = zext i8 %63 to i32
  %cmp96 = icmp eq i32 %conv95, 224
  br i1 %cmp96, label %land.lhs.true98, label %if.end110

land.lhs.true98:                                  ; preds = %land.lhs.true94
  %64 = load i16, i16* %control, align 2
  %conv99 = zext i16 %64 to i32
  %cmp100 = icmp eq i32 %conv99, 3
  br i1 %cmp100, label %if.then102, label %if.end110

if.then102:                                       ; preds = %land.lhs.true98
  %65 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %ndo_eflag103 = getelementptr inbounds %struct.netdissect_options, %struct.netdissect_options* %65, i32 0, i32 2
  %66 = load i32, i32* %ndo_eflag103, align 4
  %tobool104 = icmp ne i32 %66, 0
  br i1 %tobool104, label %if.then105, label %if.end107

if.then105:                                       ; preds = %if.then102
  %call106 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str6, i32 0, i32 0))
  br label %if.end107

if.end107:                                        ; preds = %if.then105, %if.then102
  %67 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %68 = ptrtoint i8 addrspace(200)* %67 to i64
  %add108 = add i64 %68, 3
  %69 = inttoptr i64 %add108 to i8 addrspace(200)*
  %70 = load i32, i32* %length.addr, align 4
  %sub109 = sub i32 %70, 3
  call void @ipx_print(i8 addrspace(200)* %69, i32 %sub109)
  store i32 1, i32* %retval
  br label %return

if.end110:                                        ; preds = %land.lhs.true98, %land.lhs.true94, %if.end90
  %71 = load i8, i8* %ssap, align 1
  %conv111 = zext i8 %71 to i32
  %cmp112 = icmp eq i32 %conv111, 240
  br i1 %cmp112, label %land.lhs.true114, label %if.end136

land.lhs.true114:                                 ; preds = %if.end110
  %72 = load i8, i8* %dsap, align 1
  %conv115 = zext i8 %72 to i32
  %cmp116 = icmp eq i32 %conv115, 240
  br i1 %cmp116, label %land.lhs.true118, label %if.end136

land.lhs.true118:                                 ; preds = %land.lhs.true114
  %73 = load i16, i16* %control, align 2
  %conv119 = zext i16 %73 to i32
  %and120 = and i32 %conv119, 1
  %tobool121 = icmp ne i32 %and120, 0
  br i1 %tobool121, label %lor.lhs.false, label %if.then125

lor.lhs.false:                                    ; preds = %land.lhs.true118
  %74 = load i16, i16* %control, align 2
  %conv122 = zext i16 %74 to i32
  %cmp123 = icmp eq i32 %conv122, 3
  br i1 %cmp123, label %if.then125, label %if.end136

if.then125:                                       ; preds = %lor.lhs.false, %land.lhs.true118
  %75 = load i32, i32* %is_u, align 4
  %tobool126 = icmp ne i32 %75, 0
  br i1 %tobool126, label %if.then127, label %if.else131

if.then127:                                       ; preds = %if.then125
  %76 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %add.ptr128 = getelementptr inbounds i8, i8 addrspace(200)* %76, i64 3
  store i8 addrspace(200)* %add.ptr128, i8 addrspace(200)** %p.addr, align 32
  %77 = load i32, i32* %length.addr, align 4
  %sub129 = sub i32 %77, 3
  store i32 %sub129, i32* %length.addr, align 4
  %78 = load i32, i32* %caplen.addr, align 4
  %sub130 = sub i32 %78, 3
  store i32 %sub130, i32* %caplen.addr, align 4
  br label %if.end135

if.else131:                                       ; preds = %if.then125
  %79 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %add.ptr132 = getelementptr inbounds i8, i8 addrspace(200)* %79, i64 4
  store i8 addrspace(200)* %add.ptr132, i8 addrspace(200)** %p.addr, align 32
  %80 = load i32, i32* %length.addr, align 4
  %sub133 = sub i32 %80, 4
  store i32 %sub133, i32* %length.addr, align 4
  %81 = load i32, i32* %caplen.addr, align 4
  %sub134 = sub i32 %81, 4
  store i32 %sub134, i32* %caplen.addr, align 4
  br label %if.end135

if.end135:                                        ; preds = %if.else131, %if.then127
  %82 = load i16, i16* %control, align 2
  %83 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %84 = load i32, i32* %length.addr, align 4
  call void @netbeui_print(i16 zeroext %82, i64 undef, i8 addrspace(200)* %83, i32 %84)
  store i32 1, i32* %retval
  br label %return

if.end136:                                        ; preds = %lor.lhs.false, %land.lhs.true114, %if.end110
  %85 = load i8, i8* %ssap, align 1
  %conv137 = zext i8 %85 to i32
  %cmp138 = icmp eq i32 %conv137, 254
  br i1 %cmp138, label %land.lhs.true140, label %if.end152

land.lhs.true140:                                 ; preds = %if.end136
  %86 = load i8, i8* %dsap, align 1
  %conv141 = zext i8 %86 to i32
  %cmp142 = icmp eq i32 %conv141, 254
  br i1 %cmp142, label %land.lhs.true144, label %if.end152

land.lhs.true144:                                 ; preds = %land.lhs.true140
  %87 = load i16, i16* %control, align 2
  %conv145 = zext i16 %87 to i32
  %cmp146 = icmp eq i32 %conv145, 3
  br i1 %cmp146, label %if.then148, label %if.end152

if.then148:                                       ; preds = %land.lhs.true144
  %88 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %89 = ptrtoint i8 addrspace(200)* %88 to i64
  %add149 = add i64 %89, 3
  %90 = inttoptr i64 %add149 to i8 addrspace(200)*
  %91 = load i32, i32* %length.addr, align 4
  %sub150 = sub i32 %91, 3
  %92 = load i32, i32* %caplen.addr, align 4
  %sub151 = sub i32 %92, 3
  call void @isoclns_print(i8 addrspace(200)* %90, i32 %sub150, i32 %sub151)
  store i32 1, i32* %retval
  br label %return

if.end152:                                        ; preds = %land.lhs.true144, %land.lhs.true140, %if.end136
  %93 = load i8, i8* %ssap, align 1
  %conv153 = zext i8 %93 to i32
  %cmp154 = icmp eq i32 %conv153, 170
  br i1 %cmp154, label %land.lhs.true156, label %if.end172

land.lhs.true156:                                 ; preds = %if.end152
  %94 = load i8, i8* %dsap, align 1
  %conv157 = zext i8 %94 to i32
  %cmp158 = icmp eq i32 %conv157, 170
  br i1 %cmp158, label %land.lhs.true160, label %if.end172

land.lhs.true160:                                 ; preds = %land.lhs.true156
  %95 = load i16, i16* %control, align 2
  %conv161 = zext i16 %95 to i32
  %cmp162 = icmp eq i32 %conv161, 3
  br i1 %cmp162, label %if.then164, label %if.end172

if.then164:                                       ; preds = %land.lhs.true160
  %96 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %97 = ptrtoint i8 addrspace(200)* %96 to i64
  %add165 = add i64 %97, 3
  %98 = inttoptr i64 %add165 to i8 addrspace(200)*
  %99 = load i32, i32* %length.addr, align 4
  %sub166 = sub i32 %99, 3
  %100 = load i32, i32* %caplen.addr, align 4
  %sub167 = sub i32 %100, 3
  %call168 = call i32 @snap_print(i8 addrspace(200)* %98, i32 %sub166, i32 %sub167, i32 2)
  store i32 %call168, i32* %ret, align 4
  %101 = load i32, i32* %ret, align 4
  %tobool169 = icmp ne i32 %101, 0
  br i1 %tobool169, label %if.then170, label %if.end171

if.then170:                                       ; preds = %if.then164
  %102 = load i32, i32* %ret, align 4
  store i32 %102, i32* %retval
  br label %return

if.end171:                                        ; preds = %if.then164
  br label %if.end172

if.end172:                                        ; preds = %if.end171, %land.lhs.true160, %land.lhs.true156, %if.end152
  %103 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %ndo_eflag173 = getelementptr inbounds %struct.netdissect_options, %struct.netdissect_options* %103, i32 0, i32 2
  %104 = load i32, i32* %ndo_eflag173, align 4
  %tobool174 = icmp ne i32 %104, 0
  br i1 %tobool174, label %if.end219, label %if.then175

if.then175:                                       ; preds = %if.end172
  %105 = load i8, i8* %ssap, align 1
  %conv176 = zext i8 %105 to i32
  %106 = load i8, i8* %dsap, align 1
  %conv177 = zext i8 %106 to i32
  %cmp178 = icmp eq i32 %conv176, %conv177
  br i1 %cmp178, label %if.then180, label %if.else197

if.then180:                                       ; preds = %if.then175
  %107 = load i8 addrspace(200)*, i8 addrspace(200)** %esrc.addr, align 32
  %108 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495)
  %109 = ptrtoint i8 addrspace(200)* %107 to i64
  %110 = ptrtoint i8 addrspace(200)* %108 to i64
  %cmp181 = icmp eq i64 %109, %110
  br i1 %cmp181, label %if.then186, label %lor.lhs.false183

lor.lhs.false183:                                 ; preds = %if.then180
  %111 = load i8 addrspace(200)*, i8 addrspace(200)** %edst.addr, align 32
  %112 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495)
  %113 = ptrtoint i8 addrspace(200)* %111 to i64
  %114 = ptrtoint i8 addrspace(200)* %112 to i64
  %cmp184 = icmp eq i64 %113, %114
  br i1 %cmp184, label %if.then186, label %if.else190

if.then186:                                       ; preds = %lor.lhs.false183, %if.then180
  %115 = load i8, i8* %dsap, align 1
  %conv187 = zext i8 %115 to i32
  %call188 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str8, i32 0, i32 0), i32 %conv187)
  %call189 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str7, i32 0, i32 0), i8* %call188)
  br label %if.end196

if.else190:                                       ; preds = %lor.lhs.false183
  %116 = load i8 addrspace(200)*, i8 addrspace(200)** %esrc.addr, align 32
  %call191 = call i8* @etheraddr_string(i8 addrspace(200)* %116)
  %117 = load i8 addrspace(200)*, i8 addrspace(200)** %edst.addr, align 32
  %call192 = call i8* @etheraddr_string(i8 addrspace(200)* %117)
  %118 = load i8, i8* %dsap, align 1
  %conv193 = zext i8 %118 to i32
  %call194 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str8, i32 0, i32 0), i32 %conv193)
  %call195 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str9, i32 0, i32 0), i8* %call191, i8* %call192, i8* %call194)
  br label %if.end196

if.end196:                                        ; preds = %if.else190, %if.then186
  br label %if.end218

if.else197:                                       ; preds = %if.then175
  %119 = load i8 addrspace(200)*, i8 addrspace(200)** %esrc.addr, align 32
  %120 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495)
  %121 = ptrtoint i8 addrspace(200)* %119 to i64
  %122 = ptrtoint i8 addrspace(200)* %120 to i64
  %cmp198 = icmp eq i64 %121, %122
  br i1 %cmp198, label %if.then203, label %lor.lhs.false200

lor.lhs.false200:                                 ; preds = %if.else197
  %123 = load i8 addrspace(200)*, i8 addrspace(200)** %edst.addr, align 32
  %124 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* null, i64 65495)
  %125 = ptrtoint i8 addrspace(200)* %123 to i64
  %126 = ptrtoint i8 addrspace(200)* %124 to i64
  %cmp201 = icmp eq i64 %125, %126
  br i1 %cmp201, label %if.then203, label %if.else209

if.then203:                                       ; preds = %lor.lhs.false200, %if.else197
  %127 = load i8, i8* %ssap, align 1
  %conv204 = zext i8 %127 to i32
  %call205 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str11, i32 0, i32 0), i32 %conv204)
  %128 = load i8, i8* %dsap, align 1
  %conv206 = zext i8 %128 to i32
  %call207 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str8, i32 0, i32 0), i32 %conv206)
  %call208 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str10, i32 0, i32 0), i8* %call205, i8* %call207)
  br label %if.end217

if.else209:                                       ; preds = %lor.lhs.false200
  %129 = load i8 addrspace(200)*, i8 addrspace(200)** %esrc.addr, align 32
  %call210 = call i8* @etheraddr_string(i8 addrspace(200)* %129)
  %130 = load i8, i8* %ssap, align 1
  %conv211 = zext i8 %130 to i32
  %call212 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str11, i32 0, i32 0), i32 %conv211)
  %131 = load i8 addrspace(200)*, i8 addrspace(200)** %edst.addr, align 32
  %call213 = call i8* @etheraddr_string(i8 addrspace(200)* %131)
  %132 = load i8, i8* %dsap, align 1
  %conv214 = zext i8 %132 to i32
  %call215 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([16 x %struct.tok], [16 x %struct.tok]* @llc_values, i32 0, i32 0), i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str8, i32 0, i32 0), i32 %conv214)
  %call216 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str12, i32 0, i32 0), i8* %call210, i8* %call212, i8* %call213, i8* %call215)
  br label %if.end217

if.end217:                                        ; preds = %if.else209, %if.then203
  br label %if.end218

if.end218:                                        ; preds = %if.end217, %if.end196
  br label %if.end219

if.end219:                                        ; preds = %if.end218, %if.end172
  %133 = load i32, i32* %is_u, align 4
  %tobool220 = icmp ne i32 %133, 0
  br i1 %tobool220, label %if.then221, label %if.else253

if.then221:                                       ; preds = %if.end219
  %134 = load i16, i16* %control, align 2
  %conv222 = zext i16 %134 to i32
  %and223 = and i32 %conv222, 239
  %call224 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([9 x %struct.tok], [9 x %struct.tok]* @llc_cmd_values, i32 0, i32 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str14, i32 0, i32 0), i32 %and223)
  %135 = load i8, i8* %ssap_field, align 1
  %conv225 = zext i8 %135 to i32
  %and226 = and i32 %conv225, 1
  %136 = load i16, i16* %control, align 2
  %conv227 = zext i16 %136 to i32
  %and228 = and i32 %conv227, 16
  %or229 = or i32 %and226, %and228
  %call230 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok], [7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str15, i32 0, i32 0), i32 %or229)
  %137 = load i32, i32* %length.addr, align 4
  %call231 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str13, i32 0, i32 0), i8* %call224, i8* %call230, i32 %137)
  %138 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %add.ptr232 = getelementptr inbounds i8, i8 addrspace(200)* %138, i64 3
  store i8 addrspace(200)* %add.ptr232, i8 addrspace(200)** %p.addr, align 32
  %139 = load i32, i32* %length.addr, align 4
  %sub233 = sub i32 %139, 3
  store i32 %sub233, i32* %length.addr, align 4
  %140 = load i32, i32* %caplen.addr, align 4
  %sub234 = sub i32 %140, 3
  store i32 %sub234, i32* %caplen.addr, align 4
  %141 = load i16, i16* %control, align 2
  %conv235 = zext i16 %141 to i32
  %and236 = and i32 %conv235, -17
  %cmp237 = icmp eq i32 %and236, 175
  br i1 %cmp237, label %if.then239, label %if.end252

if.then239:                                       ; preds = %if.then221
  %142 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %143 = load i8, i8 addrspace(200)* %142, align 1
  %conv240 = zext i8 %143 to i32
  %cmp241 = icmp eq i32 %conv240, 129
  br i1 %cmp241, label %if.then243, label %if.end251

if.then243:                                       ; preds = %if.then239
  %144 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %arrayidx = getelementptr inbounds i8, i8 addrspace(200)* %144, i64 1
  %145 = load i8, i8 addrspace(200)* %arrayidx, align 1
  %conv244 = zext i8 %145 to i32
  %146 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %arrayidx245 = getelementptr inbounds i8, i8 addrspace(200)* %146, i64 2
  %147 = load i8, i8 addrspace(200)* %arrayidx245, align 1
  %conv246 = zext i8 %147 to i32
  %call247 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str16, i32 0, i32 0), i32 %conv244, i32 %conv246)
  %148 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %add.ptr248 = getelementptr inbounds i8, i8 addrspace(200)* %148, i64 3
  store i8 addrspace(200)* %add.ptr248, i8 addrspace(200)** %p.addr, align 32
  %149 = load i32, i32* %length.addr, align 4
  %sub249 = sub i32 %149, 3
  store i32 %sub249, i32* %length.addr, align 4
  %150 = load i32, i32* %caplen.addr, align 4
  %sub250 = sub i32 %150, 3
  store i32 %sub250, i32* %caplen.addr, align 4
  br label %if.end251

if.end251:                                        ; preds = %if.then243, %if.then239
  br label %if.end252

if.end252:                                        ; preds = %if.end251, %if.then221
  br label %if.end290

if.else253:                                       ; preds = %if.end219
  %151 = load i16, i16* %control, align 2
  %conv254 = zext i16 %151 to i32
  %and255 = and i32 %conv254, 1
  %cmp256 = icmp eq i32 %and255, 1
  br i1 %cmp256, label %if.then258, label %if.else272

if.then258:                                       ; preds = %if.else253
  %152 = load i16, i16* %control, align 2
  %conv259 = zext i16 %152 to i32
  %shr = ashr i32 %conv259, 2
  %and260 = and i32 %shr, 3
  %call261 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([4 x %struct.tok], [4 x %struct.tok]* @llc_supervisory_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str15, i32 0, i32 0), i32 %and260)
  %153 = load i16, i16* %control, align 2
  %conv262 = zext i16 %153 to i32
  %shr263 = ashr i32 %conv262, 9
  %and264 = and i32 %shr263, 127
  %154 = load i8, i8* %ssap_field, align 1
  %conv265 = zext i8 %154 to i32
  %and266 = and i32 %conv265, 1
  %155 = load i16, i16* %control, align 2
  %conv267 = zext i16 %155 to i32
  %and268 = and i32 %conv267, 256
  %or269 = or i32 %and266, %and268
  %call270 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok], [7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str15, i32 0, i32 0), i32 %or269)
  %156 = load i32, i32* %length.addr, align 4
  %call271 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([51 x i8], [51 x i8]* @.str17, i32 0, i32 0), i8* %call261, i32 %and264, i8* %call270, i32 %156)
  br label %if.end286

if.else272:                                       ; preds = %if.else253
  %157 = load i16, i16* %control, align 2
  %conv273 = zext i16 %157 to i32
  %shr274 = ashr i32 %conv273, 1
  %and275 = and i32 %shr274, 127
  %158 = load i16, i16* %control, align 2
  %conv276 = zext i16 %158 to i32
  %shr277 = ashr i32 %conv276, 9
  %and278 = and i32 %shr277, 127
  %159 = load i8, i8* %ssap_field, align 1
  %conv279 = zext i8 %159 to i32
  %and280 = and i32 %conv279, 1
  %160 = load i16, i16* %control, align 2
  %conv281 = zext i16 %160 to i32
  %and282 = and i32 %conv281, 256
  %or283 = or i32 %and280, %and282
  %call284 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([7 x %struct.tok], [7 x %struct.tok]* @llc_flag_values, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str15, i32 0, i32 0), i32 %or283)
  %161 = load i32, i32* %length.addr, align 4
  %call285 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([60 x i8], [60 x i8]* @.str18, i32 0, i32 0), i32 %and275, i32 %and278, i8* %call284, i32 %161)
  br label %if.end286

if.end286:                                        ; preds = %if.else272, %if.then258
  %162 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %add.ptr287 = getelementptr inbounds i8, i8 addrspace(200)* %162, i64 4
  store i8 addrspace(200)* %add.ptr287, i8 addrspace(200)** %p.addr, align 32
  %163 = load i32, i32* %length.addr, align 4
  %sub288 = sub i32 %163, 4
  store i32 %sub288, i32* %length.addr, align 4
  %164 = load i32, i32* %caplen.addr, align 4
  %sub289 = sub i32 %164, 4
  store i32 %sub289, i32* %caplen.addr, align 4
  br label %if.end290

if.end290:                                        ; preds = %if.end286, %if.end252
  store i32 1, i32* %retval
  br label %return

return:                                           ; preds = %if.end290, %if.then170, %if.then148, %if.end135, %if.end107, %if.then87, %if.then73, %if.end29, %if.then8, %if.then
  %165 = load i32, i32* %retval
  ret i32 %165
}

; Function Attrs: nounwind readnone

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
  store i32 %length, i32* %length.addr, align 4
  store i32 %caplen, i32* %caplen.addr, align 4
  store i32 %bridge_pad, i32* %bridge_pad.addr, align 4
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %1 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %0)
  %cmp = icmp ule i64 5, %1
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  br label %trunc

if.end:                                           ; preds = %entry
  %2 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %3 = addrspacecast i8 addrspace(200)* %2 to i8*
  %add.ptr = getelementptr inbounds i8, i8* %3, i64 0
  %4 = load i8, i8* %add.ptr, align 1
  %conv = zext i8 %4 to i32
  %shl = shl i32 %conv, 16
  %5 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %6 = addrspacecast i8 addrspace(200)* %5 to i8*
  %add.ptr1 = getelementptr inbounds i8, i8* %6, i64 1
  %7 = load i8, i8* %add.ptr1, align 1
  %conv2 = zext i8 %7 to i32
  %shl3 = shl i32 %conv2, 8
  %or = or i32 %shl, %shl3
  %8 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %9 = addrspacecast i8 addrspace(200)* %8 to i8*
  %add.ptr4 = getelementptr inbounds i8, i8* %9, i64 2
  %10 = load i8, i8* %add.ptr4, align 1
  %conv5 = zext i8 %10 to i32
  %or6 = or i32 %or, %conv5
  store i32 %or6, i32* %orgcode, align 4
  %11 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %12 = ptrtoint i8 addrspace(200)* %11 to i64
  %add = add i64 %12, 3
  %13 = inttoptr i64 %add to i8 addrspace(200)*
  %call = call zeroext i16 @EXTRACT_16BITS(i8 addrspace(200)* %13)
  store i16 %call, i16* %et, align 2
  %14 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %ndo_eflag = getelementptr inbounds %struct.netdissect_options, %struct.netdissect_options* %14, i32 0, i32 2
  %15 = load i32, i32* %ndo_eflag, align 4
  %tobool = icmp ne i32 %15, 0
  br i1 %tobool, label %if.then7, label %if.end23

if.then7:                                         ; preds = %if.end
  store %struct.tok* getelementptr inbounds ([1 x %struct.tok], [1 x %struct.tok]* @null_values, i32 0, i32 0), %struct.tok** %tok, align 8
  store %struct.oui_tok* getelementptr inbounds ([6 x %struct.oui_tok], [6 x %struct.oui_tok]* @oui_to_tok, i32 0, i64 0), %struct.oui_tok** %otp, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then7
  %16 = load %struct.oui_tok*, %struct.oui_tok** %otp, align 8
  %tok8 = getelementptr inbounds %struct.oui_tok, %struct.oui_tok* %16, i32 0, i32 1
  %17 = load %struct.tok*, %struct.tok** %tok8, align 8
  %cmp9 = icmp ne %struct.tok* %17, null
  br i1 %cmp9, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %18 = load %struct.oui_tok*, %struct.oui_tok** %otp, align 8
  %oui = getelementptr inbounds %struct.oui_tok, %struct.oui_tok* %18, i32 0, i32 0
  %19 = load i32, i32* %oui, align 4
  %20 = load i32, i32* %orgcode, align 4
  %cmp11 = icmp eq i32 %19, %20
  br i1 %cmp11, label %if.then13, label %if.end15

if.then13:                                        ; preds = %for.body
  %21 = load %struct.oui_tok*, %struct.oui_tok** %otp, align 8
  %tok14 = getelementptr inbounds %struct.oui_tok, %struct.oui_tok* %21, i32 0, i32 1
  %22 = load %struct.tok*, %struct.tok** %tok14, align 8
  store %struct.tok* %22, %struct.tok** %tok, align 8
  br label %for.end

if.end15:                                         ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end15
  %23 = load %struct.oui_tok*, %struct.oui_tok** %otp, align 8
  %incdec.ptr = getelementptr inbounds %struct.oui_tok, %struct.oui_tok* %23, i32 1
  store %struct.oui_tok* %incdec.ptr, %struct.oui_tok** %otp, align 8
  br label %for.cond

for.end:                                          ; preds = %if.then13, %for.cond
  %24 = load i32, i32* %orgcode, align 4
  %call16 = call i8* @tok2str(%struct.tok* getelementptr inbounds ([0 x %struct.tok], [0 x %struct.tok]* @oui_values, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str3, i32 0, i32 0), i32 %24)
  %25 = load i32, i32* %orgcode, align 4
  %26 = load i32, i32* %orgcode, align 4
  %cmp17 = icmp eq i32 %26, 0
  %cond = select i1 %cmp17, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str20, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str21, i32 0, i32 0)
  %27 = load %struct.tok*, %struct.tok** %tok, align 8
  %28 = load i16, i16* %et, align 2
  %conv19 = zext i16 %28 to i32
  %call20 = call i8* @tok2str(%struct.tok* %27, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str3, i32 0, i32 0), i32 %conv19)
  %29 = load i16, i16* %et, align 2
  %conv21 = zext i16 %29 to i32
  %call22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str19, i32 0, i32 0), i8* %call16, i32 %25, i8* %cond, i8* %call20, i32 %conv21)
  br label %if.end23

if.end23:                                         ; preds = %for.end, %if.end
  %30 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %add.ptr24 = getelementptr inbounds i8, i8 addrspace(200)* %30, i64 5
  store i8 addrspace(200)* %add.ptr24, i8 addrspace(200)** %p.addr, align 32
  %31 = load i32, i32* %length.addr, align 4
  %sub = sub i32 %31, 5
  store i32 %sub, i32* %length.addr, align 4
  %32 = load i32, i32* %caplen.addr, align 4
  %sub25 = sub i32 %32, 5
  store i32 %sub25, i32* %caplen.addr, align 4
  %33 = load i32, i32* %orgcode, align 4
  switch i32 %33, label %sw.epilog85 [
    i32 0, label %sw.bb
    i32 248, label %sw.bb
    i32 524295, label %sw.bb30
    i32 12, label %sw.bb40
    i32 32962, label %sw.bb47
  ]

sw.bb:                                            ; preds = %if.end23, %if.end23
  %34 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %35 = load i16, i16* %et, align 2
  %36 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %37 = load i32, i32* %length.addr, align 4
  %38 = load i32, i32* %caplen.addr, align 4
  %call26 = call i32 @ethertype_print(%struct.netdissect_options* %34, i16 zeroext %35, i8 addrspace(200)* %36, i32 %37, i32 %38)
  store i32 %call26, i32* %ret, align 4
  %39 = load i32, i32* %ret, align 4
  %tobool27 = icmp ne i32 %39, 0
  br i1 %tobool27, label %if.then28, label %if.end29

if.then28:                                        ; preds = %sw.bb
  %40 = load i32, i32* %ret, align 4
  store i32 %40, i32* %retval
  br label %return

if.end29:                                         ; preds = %sw.bb
  br label %sw.epilog85

sw.bb30:                                          ; preds = %if.end23
  %41 = load i16, i16* %et, align 2
  %conv31 = zext i16 %41 to i32
  %cmp32 = icmp eq i32 %conv31, 32923
  br i1 %cmp32, label %if.then34, label %if.end39

if.then34:                                        ; preds = %sw.bb30
  %42 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %43 = load i16, i16* %et, align 2
  %44 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %45 = load i32, i32* %length.addr, align 4
  %46 = load i32, i32* %caplen.addr, align 4
  %call35 = call i32 @ethertype_print(%struct.netdissect_options* %42, i16 zeroext %43, i8 addrspace(200)* %44, i32 %45, i32 %46)
  store i32 %call35, i32* %ret, align 4
  %47 = load i32, i32* %ret, align 4
  %tobool36 = icmp ne i32 %47, 0
  br i1 %tobool36, label %if.then37, label %if.end38

if.then37:                                        ; preds = %if.then34
  %48 = load i32, i32* %ret, align 4
  store i32 %48, i32* %retval
  br label %return

if.end38:                                         ; preds = %if.then34
  br label %if.end39

if.end39:                                         ; preds = %if.end38, %sw.bb30
  br label %sw.epilog85

sw.bb40:                                          ; preds = %if.end23
  %49 = load i16, i16* %et, align 2
  %conv41 = zext i16 %49 to i32
  switch i32 %conv41, label %sw.default [
    i32 8192, label %sw.bb42
    i32 8196, label %sw.bb43
    i32 273, label %sw.bb44
    i32 8195, label %sw.bb45
    i32 267, label %sw.bb46
  ]

sw.bb42:                                          ; preds = %sw.bb40
  %50 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %51 = load i32, i32* %length.addr, align 4
  %52 = load i32, i32* %caplen.addr, align 4
  call void @cdp_print(i8 addrspace(200)* %50, i32 %51, i32 %52)
  store i32 1, i32* %retval
  br label %return

sw.bb43:                                          ; preds = %sw.bb40
  %53 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %54 = load i32, i32* %length.addr, align 4
  call void @dtp_print(i8 addrspace(200)* %53, i32 %54)
  store i32 1, i32* %retval
  br label %return

sw.bb44:                                          ; preds = %sw.bb40
  %55 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %56 = load i32, i32* %length.addr, align 4
  call void @udld_print(i8 addrspace(200)* %55, i32 %56)
  store i32 1, i32* %retval
  br label %return

sw.bb45:                                          ; preds = %sw.bb40
  %57 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %58 = load i32, i32* %length.addr, align 4
  call void @vtp_print(i8 addrspace(200)* %57, i32 %58)
  store i32 1, i32* %retval
  br label %return

sw.bb46:                                          ; preds = %sw.bb40
  %59 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %60 = load i32, i32* %length.addr, align 4
  call void @stp_print(i8 addrspace(200)* %59, i32 %60)
  store i32 1, i32* %retval
  br label %return

sw.default:                                       ; preds = %sw.bb40
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default
  br label %sw.bb47

sw.bb47:                                          ; preds = %if.end23, %sw.epilog
  %61 = load i16, i16* %et, align 2
  %conv48 = zext i16 %61 to i32
  switch i32 %conv48, label %sw.epilog84 [
    i32 1, label %sw.bb49
    i32 7, label %sw.bb49
    i32 3, label %sw.bb58
    i32 9, label %sw.bb58
    i32 4, label %sw.bb69
    i32 10, label %sw.bb69
    i32 14, label %sw.bb83
  ]

sw.bb49:                                          ; preds = %sw.bb47, %sw.bb47
  %62 = load i32, i32* %bridge_pad.addr, align 4
  %conv50 = zext i32 %62 to i64
  %63 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %64 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %63)
  %cmp51 = icmp ule i64 %conv50, %64
  br i1 %cmp51, label %if.end54, label %if.then53

if.then53:                                        ; preds = %sw.bb49
  br label %trunc

if.end54:                                         ; preds = %sw.bb49
  %65 = load i32, i32* %bridge_pad.addr, align 4
  %66 = load i32, i32* %caplen.addr, align 4
  %sub55 = sub i32 %66, %65
  store i32 %sub55, i32* %caplen.addr, align 4
  %67 = load i32, i32* %bridge_pad.addr, align 4
  %68 = load i32, i32* %length.addr, align 4
  %sub56 = sub i32 %68, %67
  store i32 %sub56, i32* %length.addr, align 4
  %69 = load i32, i32* %bridge_pad.addr, align 4
  %70 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %idx.ext = zext i32 %69 to i64
  %add.ptr57 = getelementptr inbounds i8, i8 addrspace(200)* %70, i64 %idx.ext
  store i8 addrspace(200)* %add.ptr57, i8 addrspace(200)** %p.addr, align 32
  %71 = load %struct.netdissect_options*, %struct.netdissect_options** @gndo, align 8
  %72 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %73 = load i32, i32* %length.addr, align 4
  %74 = load i32, i32* %caplen.addr, align 4
  call void @ether_print(%struct.netdissect_options* %71, i64 undef, i8 addrspace(200)* %72, i32 %73, i32 %74, void (%struct.netdissect_options*, i64, i8 addrspace(200)*)* null, i64 undef, i8 addrspace(200)* null)
  store i32 1, i32* %retval
  br label %return

sw.bb58:                                          ; preds = %sw.bb47, %sw.bb47
  %75 = load i32, i32* %bridge_pad.addr, align 4
  %conv59 = zext i32 %75 to i64
  %76 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %77 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %76)
  %cmp60 = icmp ule i64 %conv59, %77
  br i1 %cmp60, label %if.end63, label %if.then62

if.then62:                                        ; preds = %sw.bb58
  br label %trunc

if.end63:                                         ; preds = %sw.bb58
  %78 = load i32, i32* %bridge_pad.addr, align 4
  %79 = load i32, i32* %caplen.addr, align 4
  %sub64 = sub i32 %79, %78
  store i32 %sub64, i32* %caplen.addr, align 4
  %80 = load i32, i32* %bridge_pad.addr, align 4
  %81 = load i32, i32* %length.addr, align 4
  %sub65 = sub i32 %81, %80
  store i32 %sub65, i32* %length.addr, align 4
  %82 = load i32, i32* %bridge_pad.addr, align 4
  %83 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %idx.ext66 = zext i32 %82 to i64
  %add.ptr67 = getelementptr inbounds i8, i8 addrspace(200)* %83, i64 %idx.ext66
  store i8 addrspace(200)* %add.ptr67, i8 addrspace(200)** %p.addr, align 32
  %84 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %85 = load i32, i32* %length.addr, align 4
  %86 = load i32, i32* %caplen.addr, align 4
  %call68 = call i32 @token_print(i8 addrspace(200)* %84, i32 %85, i32 %86)
  store i32 1, i32* %retval
  br label %return

sw.bb69:                                          ; preds = %sw.bb47, %sw.bb47
  %87 = load i32, i32* %bridge_pad.addr, align 4
  %add70 = add i32 %87, 1
  %conv71 = zext i32 %add70 to i64
  %88 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %89 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %88)
  %cmp72 = icmp ule i64 %conv71, %89
  br i1 %cmp72, label %if.end75, label %if.then74

if.then74:                                        ; preds = %sw.bb69
  br label %trunc

if.end75:                                         ; preds = %sw.bb69
  %90 = load i32, i32* %bridge_pad.addr, align 4
  %add76 = add i32 %90, 1
  %91 = load i32, i32* %caplen.addr, align 4
  %sub77 = sub i32 %91, %add76
  store i32 %sub77, i32* %caplen.addr, align 4
  %92 = load i32, i32* %bridge_pad.addr, align 4
  %add78 = add i32 %92, 1
  %93 = load i32, i32* %length.addr, align 4
  %sub79 = sub i32 %93, %add78
  store i32 %sub79, i32* %length.addr, align 4
  %94 = load i32, i32* %bridge_pad.addr, align 4
  %add80 = add i32 %94, 1
  %95 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %idx.ext81 = zext i32 %add80 to i64
  %add.ptr82 = getelementptr inbounds i8, i8 addrspace(200)* %95, i64 %idx.ext81
  store i8 addrspace(200)* %add.ptr82, i8 addrspace(200)** %p.addr, align 32
  %96 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %97 = load i32, i32* %length.addr, align 4
  %98 = load i32, i32* %caplen.addr, align 4
  call void @fddi_print(i8 addrspace(200)* %96, i32 %97, i32 %98)
  store i32 1, i32* %retval
  br label %return

sw.bb83:                                          ; preds = %sw.bb47
  %99 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %100 = load i32, i32* %length.addr, align 4
  call void @stp_print(i8 addrspace(200)* %99, i32 %100)
  store i32 1, i32* %retval
  br label %return

sw.epilog84:                                      ; preds = %sw.bb47
  br label %sw.epilog85

sw.epilog85:                                      ; preds = %sw.epilog84, %if.end23, %if.end39, %if.end29
  store i32 0, i32* %retval
  br label %return

trunc:                                            ; preds = %if.then74, %if.then62, %if.then53, %if.then
  %call86 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str22, i32 0, i32 0))
  store i32 1, i32* %retval
  br label %return

return:                                           ; preds = %trunc, %sw.epilog85, %sw.bb83, %if.end75, %if.end63, %if.end54, %sw.bb46, %sw.bb45, %sw.bb44, %sw.bb43, %sw.bb42, %if.then37, %if.then28
  %101 = load i32, i32* %retval
  ret i32 %101
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
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %p.addr, align 32
  %1 = addrspacecast i8 addrspace(200)* %0 to %struct.unaligned_u_int16_t*
  %val = getelementptr inbounds %struct.unaligned_u_int16_t, %struct.unaligned_u_int16_t* %1, i32 0, i32 0
  %2 = load i16, i16* %val, align 1
  ret i16 %2
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

