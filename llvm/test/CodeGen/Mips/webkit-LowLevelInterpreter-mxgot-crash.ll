; RUN: llc -target-abi n64 -relocation-model=pic -mxgot -o /dev/null %s
; Crash when compiling WebKit LowLevelInterpreter.cpp

; https://github.com/CTSRD-CHERI/llvm/issues/225

; ModuleID = 'new-webkit2.creduce.test.creduce.ll-reduced-simplified.bc'
source_filename = "new-webkit2.creduce.test.creduce.ll-output-33efd2f.bc"
target triple = "mips64-unknownfreebsd12"

%"class.JSC::VM.622.1251.1880.620.1244.1868.0.2.4" = type {}
%"struct.JSC::ProtoCallFrame.623.1252.1881.621.1245.1869.1.3.5" = type {}

define { i64 } @a(i32, i8, %"class.JSC::VM.622.1251.1880.620.1244.1868.0.2.4", %"struct.JSC::ProtoCallFrame.623.1252.1881.621.1245.1869.1.3.5", i1) {
  br i1 undef, label %6, label %8

; <label>:6:                                      ; preds = %5
  %7 = call i8* @b(i8* blockaddress(@a, %8))
  ret { i64 } undef

; <label>:8:                                      ; preds = %8, %5
  br label %8
}

declare i8* @b(i8*)
