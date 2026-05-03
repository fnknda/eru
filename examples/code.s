bits 64

; This code is just to use for the examples.
; The instructions start after the "Hello, world!\n" string (14 bytes in).
; - Compile with: `nasm -fbin -o code code.s`
; - Export bytes to C format with: `xxd -i -ncode code`

text:
	db "Hello, world!", 0xa
text.end:

_start:
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel text]
	mov rdx, text.end - text
	syscall

	mov rax, 60
	mov rdi, 0
	syscall
