disk_load:
	pusha ; push all registers on to the stack
	push dx ; push number of sectors to read onto stack
	
	mov ah, 0x02 ; set BIOS to read mode
	mov al, dh ; set sectors to dh
	mov cl, 0x02 ; start from sector (sector 1 is MBR)
	mov ch, 0x00 ; cylinder is 0
	mov dh, 0x00 ; head is 0
	
	; dl is the drive number set by bios as input to disk_load
	; es:bx is the buffer pointer, which is set to input as well
	
	int 0x13 ; send interrupt 13h to BIOS to do its work
	jc disk_error ; BIOS will set carry bit if there is an error
	
	pop dx ; put original number of sectors to read back into dx
	cmp al, dh ; al is number of sectors to actually read, if not
			   ; equal to dh which is the disk head, jump to sectors
			   ; error
			   
	jne sectors_error
	popa ; restore all registers to original values
	ret
	
disk_error:
	mov si, disk_error_label ; prepare disk error text
	call print_string
	jmp disk_loop
	
sectors_error:
	mov si, sectors_error_label ; prepare disk error text
	call print_string
	jmp disk_loop
	
print_string:
	pusha
	mov ah, 0Eh ; int 10h BIOS interrupt to write text
	
disk_loop:
	jmp $
	
disk_error_label	db "OSLDR: Disk read error occured! Joldzic OS-CORE cannot continue loading! STOP 0xfffe001", 0
sectors_error_label	db "OSLDR: Disk sector error occured! Joldzic OS-CORE cannot continue loading! STOP 0xfffe002", 0