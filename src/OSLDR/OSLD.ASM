[bits 16]; running in 16 bit real mode
[org 0x7c00]; set up absolute address for function stack for nasm/ld

global _start

_start:
	KERNEL_OFFSET equ 0x1000; where kernel will be loaded to

	mov [BOOT_DRIVE], dl; get boot drive from BIOS (BIOS stores it in dl) and move to BOOT_DRIVE

	;set up call stack
	mov bp, 0x9000
	mov sp, bp

	;load the kernel and prepare to switch into 32bit protected mode
	call load_kernel
	call switch_to_32bit
	jmp $

	%include "DISK.ASM"; Load disk
	%include "LGDT.ASM"; Load GDT table
	%include "PM32.ASM"; ASM code to prepare switch to 32-bit protected

[bits 16]; still in 16 bit real mode
load_kernel:
	mov bx, KERNEL_OFFSET; bx is the destination
	mov dh, 2; dh is the number of sectors
	mov dl, [BOOT_DRIVE]; dl is the disk
	call disk_load; initialize disk
	int 0x10 ; int 10h
	ret
	
[bits 32]; now in 32 bit protected mode
BEGIN_32BIT:
	call KERNEL_OFFSET; kernel gets control now
	jmp $; loop back in case kernel returns something
	
BOOT_DRIVE 			db 0 ; boot drive variable


; pad to 512 bytes
times 510 - ($-$$) db 0

; BIOS magic number
dw 0xaa55