global _start

    SYS_EXIT equ 60
    SYS_OPEN equ 2
    SYS_WRITE equ 1
    SYS_CLOSE equ 3

    O_WRONLY equ 0x0001             ; Flag for opening a file only to write to it.
    O_CREAT equ 0x00040             ; Flag for creating a file with SYS_OPEN.
    O_EXCL equ 0x0080               ; Flag that produces an error if the file to be created already exists.
    O_RDONLY equ 0x0000             ; Flag for read only.

section .bss
    READ_BUFFER: resb 4096         ; Reserve the space for 4096 characters (bytes) to minimize the number of syscalls related to reading in from the in_file.
    WRITE_BUFFER: resb 6160        ; 4096 * 3/2 + 16 to account for the worst-case scenario (which is a sequence "sasa...sa").

section .rodata:
    READ_BUFFER_SIZE equ 4096      
    PERMISSIONS equ 0o644           ; Permission code for -rw-r--r--.
    NUMBER_OF_PARAMETERS equ 3      ; 0th parameter is the program name, 1st is for the in_file and 2nd is for out_file.

section .text

_start:
    mov rcx, [rsp]                  ; Loading the number of arguments into rcx.            
    
    cmp rcx, NUMBER_OF_PARAMETERS   ; If the number of parameters is different than three, end the program with the error code of 1.
    mov r8b, 1
    jne exit_final  
   
    mov rax, SYS_OPEN               ; Tries to open the file with the name given as the first parameter.
    mov rdi, [rsp + 16]             ; Moves the address of the first parameter (in_file) to rdi.
    mov rsi, O_RDONLY        
    syscall 
   
    mov r8b, 1
    cmp eax, 0                      ; Checks for errors. An error is a number [-4095, -1], which means it is stored in eax.
    jl exit_final        

    mov r9, rax                     ; r9 will hold the pointer to in_file.

    mov rax, SYS_OPEN               ; Creates a new file with the name provided as the second parameter which is passed into rdi.
    mov rdi, [rsp + 24]
    mov rsi, O_WRONLY               ; Loads the necessary flags into rsi with OR (write only, create file, checks if a file with the given name already exists).
    OR rsi, O_CREAT
    OR rsi, O_EXCL
    mov rdx, PERMISSIONS            
    syscall
   
    cmp eax, 0                  
    jl part_error_exit

    mov r10, rax                    ; r10 will hold the pointer to out_file.

    xor r13, r13                    ; r13w will hold the number of non-"s" and non-"S" characters in a continuous sequence in modulo.
    xor r12b, r12b                  ; Holds the information of whether there was a sequence of non-"s" and non-"S" characters in between "s" or "S".

loop_through_file:                  ; Loop through the characters in in_file.
    xor r15, r15                    ; The iterator that counts in an increasing manner so that each 8 iterations, the next 64 bits are loaded from the READ_BUFFER.
    xor r14, r14                    ; r14 will hold the offset to the WRITE_BUFFER when moving appropriate values into it. It also holds the number of chars to write using the syscall.

                                    ; xor rax, rax is equivalent to mov rax, SYS_READ (equ 0).
    xor rax, rax                    ; Reads from in_file 2048 characters (each character is the size of one byte) at most, otherwise it reads the number of bytes left in the file.
    mov rdi, r9
    mov rsi, READ_BUFFER        
    mov rdx, READ_BUFFER_SIZE       ; Read READ_BUFFER_SIZE chars into read READ_BUFFER.  
    syscall

    cmp eax, 0  
                 
    jl error_exit
    mov r8b, 0
    je write_exit_non_s             ; Exit the program if eax = 0, ie. there are no more bytes to read in the file. This is the only "error-free" (thus far) exit function.

buffer_loop:
    mov rcx, r15           
    and ecx, 7                      ; ecx (ie. the iterator) = ecx % 8.
    cmp ecx, 0
                                    ; Move the contents of READ_BUFFER into r8 as it is not modified by syscalls. r8b contains the ASCII code of the youngest char.
    cmove r8, [READ_BUFFER + r15]   ; Move the next 64-bit "batch" of characters into r8.

    cmp r8b, 's'
    je write_non_s

    cmp r8b, 'S'
    jne count_non_s

write_non_s:                        ; Checks if r12b is non-zero, ie. that there was a continuous sequence of non-"s" and non-"S" characters, then writes the value of r13w to the WRITE_BUFFER.
    cmp r12b, 0
    je write_s                      ; Jump if there was no sequence of non-"s" and non-"S" characters.

    mov [WRITE_BUFFER + r14], r13w
    add r14, 2                      ; This is the offset to the pointer to WRITE_BUFFER because two new chars (16 bits) were moved into WRITE_BUFFER.
   
    xor r13, r13
    xor r12b, r12b
   
write_s:                            ; Writes "s" or "S" to the out_file.
    mov [WRITE_BUFFER + r14], r8b
    inc r14

    jmp continue_loop

count_non_s:                        ; Counts non-"s" and non-"S" characters in a continuous sequence.
    inc r13                         ; The modulo is always kept in r13w (the value of the rest of the register can be ignored).
    mov r12b, 1                     ; Set r12b to 1 meaning that there was a sequence non-"s" and non-"S" characters.

continue_loop:
    shr r8, 8                       ; Shift right by 8 bits in order to have the next byte on the last position so that it can be separated again.

    inc r15
    cmp r15, rax                    ; rax holds the number of bytes read into READ_BUFFER. 
    jl buffer_loop
    
    jmp prep_write

write_exit_non_s:                   ; Checks if r12b is non-zero, ie. that there was a continuous sequence of non-"s" and non-"S" characters before the file ends, then writes the value of r13w to WRITE_BUFFER.
    cmp r12b, 0
    je full_exit                    ; Jump if there was no sequence of non-"s" and non-"S" characters.

    mov [WRITE_BUFFER + r14], r13w
    add r14, 2
   
    xor r12b, r12b

prep_write:
    xor r15, r15

write_to_file:                      ; Writes the values of the WRITE_BUFFER into out_file.
    mov rax, SYS_WRITE
    mov rdi, r10
    lea rsi, [WRITE_BUFFER + r15]   ; Start writing from WRITE_BUFFER + r15 (which can be zero or more depending on whether SYS_WRITE will successfully write all the bytes that should be written).         
    mov rdx, r14                    
   
    syscall
    cmp eax, 0                      
    jl error_exit

    add r15, rax                    ; If SYS_WRITE did not write all of the bytes that it should have, it will write the remaining ones to the out_file.
    sub r14, rax
    cmp r14, 0
    jg write_to_file

    jmp loop_through_file

part_error_exit:                    ; Places the error code of 1 into r8b (as it will not be modified by the subsequent syscalls).
    mov r8b, 1      
    jmp continue_exit               ; This ensures that the out_file that there is no attempt to close the out_file if it had not been created.

error_exit:                         ; Closes both the in_file and out_file and ends the program with the error code of 1.
    mov r8b, 1

full_exit:                          ; Ends the program by closing the out_file if it had been created successfully in the first place.
    mov rax, SYS_CLOSE
    mov rdi, r10
    syscall

    cmp eax, 0
    jge continue_exit
    mov r8b, 1                      ; If SYS_CLOSE fails, the appropriate error code is placed into r8b.

continue_exit:                      ; Continues the process of closing/ending the program (only closes the in_file).
    mov rax, SYS_CLOSE
    mov rdi, r9
    syscall

    cmp eax, 0                     
    jge exit_final
    mov r8b, 1
   
exit_final:
    mov dil, r8b
    mov rax, SYS_EXIT
    syscall 