;;
;; NASM 32bit assembler
;;
 
[bits 32]
[section .text]
 
jmp is_A20_on?

enable_A20:
        cli
 
        call    a20wait
        mov     al,0xAD
        out     0x64,al
 
        call    a20wait
        mov     al,0xD0
        out     0x64,al
 
        call    a20wait2
        in      al,0x60
        push    eax
 
        call    a20wait
        mov     al,0xD1
        out     0x64,al
 
        call    a20wait
        pop     eax
        or      al,2
        out     0x60,al
 
        call    a20wait
        mov     al,0xAE
        out     0x64,al
 
        call    is_A20_on?
        sti
        ret
 
a20wait:
        in      al,0x64
        test    al,2
        jnz     a20wait
        ret
 
 
a20wait2:
        in      al,0x64
        test    al,1
        jz      a20wait2
        ret

 
; Check A20 line
; Returns to caller if A20 gate is cleared.
; Continues to A20_on if A20 line is set.
; Written by Elad Ashkcenazi 
 
is_A20_on?:   
 
	pushad
	mov edi,0x112345  ;odd megabyte address.
	mov esi,0x012345  ;even megabyte address.
	mov [esi],esi     ;making sure that both addresses contain diffrent values.
	mov [edi],edi     ;(if A20 line is cleared the two pointers would point to the address 0x012345 that would contain 0x112345 (edi)) 
	cmpsd             ;compare addresses to see if the're equivalent.
	popad
	jne A20_on        ;if not equivalent , A20 line is set.
	jmp A20_off       ;if equivalent , the A20 line is cleared.
	call a20_error	  ; print a20 error


a20_error:
	mov si, a20_error_label ; prepare disk error text
	call print_string
	
a20_success:
	mov si, a20_success_label ; prepare disk error text
	call print_string
 
A20_on:
	[bits 32]
	[extern main]
	call main
	call a20_success
	cli
	hlt
	
A20_off:
	call a20_error
	jmp enable_A20
	
print_string:
	pusha
	mov ah, 0Eh ; int 10h BIOS interrupt to write text

a20_loop:
	jmp $
	
a20_error_label		db "OSLDR: A20 line not enabled! Joldzic OS-CORE cannot continue loading! STOP 0xfffe002", 0
a20_success_label	db "OSLDR: A20 line successfully enabled. Loading kernel...", 0
