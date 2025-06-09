section .data
    hello db 'Hello, World!', 0    ; Null-terminated string

section .text
    global _start                   ; Entry point for the program

_start:
    ; Write "Hello, World!" to stdout
    mov eax, 4                      ; sys_write system call number
    mov ebx, 1                      ; File descriptor (stdout)
    mov ecx, hello                  ; Pointer to the string
    mov edx, 13                     ; Length of the string
    int 0x80                        ; Call kernel

    ; Exit the program
    mov eax, 1                      ; sys_exit system call number
    xor ebx, ebx                    ; Exit code 0
    int 0x80                        ; Call kernel


;nasm -f elf32 hello.asm   # Assemble the source code
;ld -m elf_i386 -o hello hello.o  # Link the object file
;./hello                   # Run the executable 