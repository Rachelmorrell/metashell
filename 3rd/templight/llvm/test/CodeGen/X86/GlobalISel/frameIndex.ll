; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=x86_64-linux-gnu    -global-isel -verify-machineinstrs < %s -o - | FileCheck %s --check-prefix=X64
; RUN: llc -mtriple=x86_64-linux-gnu                 -verify-machineinstrs < %s -o - | FileCheck %s --check-prefix=X64
; RUN: llc -mtriple=i386-linux-gnu      -global-isel -verify-machineinstrs < %s -o - | FileCheck %s --check-prefix=X32
; RUN: llc -mtriple=i386-linux-gnu                   -verify-machineinstrs < %s -o - | FileCheck %s --check-prefix=X32
; RUN: llc -mtriple=x86_64-linux-gnux32 -global-isel -verify-machineinstrs < %s -o - | FileCheck %s --check-prefix=X32ABI
; RUN: llc -mtriple=x86_64-linux-gnux32              -verify-machineinstrs < %s -o - | FileCheck %s --check-prefix=X32ABI

define i32* @allocai32() {
; X64-LABEL: allocai32:
; X64:       # BB#0:
; X64-NEXT:    leaq -4(%rsp), %rax
; X64-NEXT:    retq
;
; X32-LABEL: allocai32:
; X32:       # BB#0:
; X32-NEXT:    pushl %eax
; X32-NEXT:  .Lcfi0:
; X32-NEXT:    .cfi_def_cfa_offset 8
; X32-NEXT:    movl %esp, %eax
; X32-NEXT:    popl %ecx
; X32-NEXT:    retl
;
; X32ABI-LABEL: allocai32:
; X32ABI:       # BB#0:
; X32ABI-NEXT:    leal -4(%rsp), %eax
; X32ABI-NEXT:    retq
  %ptr1 = alloca i32
  ret i32* %ptr1
}
