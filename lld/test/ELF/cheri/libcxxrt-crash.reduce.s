# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t.o
# RUN: llvm-readobj -r %t.o
# RUN: ld.lld --eh-frame-hdr --shared -z notext -o %t.so %t.o

.cfi_startproc
lui	$11, %hi(%neg(%gp_rel(a)))
.cfi_endproc
