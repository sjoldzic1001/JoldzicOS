# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: boot.vdi

KRNL.BIN: KRNE.O KRNL.O
	ld -m elf_i386 -o KERN.O -Ttext 0x1000 $^ --entry main
	objcopy -O binary -j .text -j .rdata KERN.O $@

KRNE.O: KRNE.ASM
	nasm $< -f elf32 -o $@

KRNL.O: c/KRNL.C
	gcc -m32 -ffreestanding -c $< -o $@ -fno-pie

OSLD.BIN: OSLD.ASM
	nasm $< -f bin -o $@

BOOT.BIN: OSLD.BIN KRNL.BIN
	cat $^ > $@

boot.vdi: BOOT.BIN
	qemu-img convert -f raw -O vdi $< $@

#run: BOOT.BIN	
	#qemu-system-i386 -drive format=raw,file=$<

clean:
	$(RM) *.BIN *.O *.DIS
