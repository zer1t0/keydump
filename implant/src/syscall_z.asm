SECTION .text
global  syscall_z

syscall_z:
  mov rax, rdi
  mov rdi, rsi
  mov rsi, rdx
  mov rdx, rcx
  mov r10, r8
  mov r8, r9
  mov r9, [rsp+8h]
  syscall
  ret

