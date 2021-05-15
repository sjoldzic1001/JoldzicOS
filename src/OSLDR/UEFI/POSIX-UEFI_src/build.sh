#!/bin/bash

rm -rf ./EFI ./JoldzicOS main.efi main.o boot.img /tmp/part.img

make
  
dd if=/dev/zero of=boot.img bs=512 count=93750
parted boot.img -s -a minimal mklabel gpt
parted boot.img -s -a minimal mkpart EFI FAT16 2048s 93716s
parted boot.img -s -a minimal toggle 1 boot

dd if=/dev/zero of=/tmp/part.img bs=512 count=91669
mformat -i /tmp/part.img -h 32 -t 32 -n 64 -c 1

mkdir JoldzicOS
mkdir EFI
mkdir EFI/BOOT
mv main.efi EFI/BOOT/BOOTX64.EFI
touch EFI/BOOT/config.txt
touch JoldzicOS/joskrnl.prg
echo -e "KERNEL=\JoldzicOS\joskrnl.prg\nCMDLINE=boot=1 hd=1" > EFI/BOOT/config.txt

mcopy -s -i /tmp/part.img EFI ::
mcopy -s -i /tmp/part.img JoldzicOS ::

dd if=/tmp/part.img of=boot.img bs=512 count=91669 seek=2048 conv=notrunc

 qemu-system-x86_64 -cpu qemu64 \
  -drive if=pflash,format=raw,unit=0,file=/usr/share/OVMF/OVMF_CODE.fd,readonly=on \
  -drive if=pflash,format=raw,unit=1,file=/usr/share/OVMF/OVMF_VARS.fd \
  -net none \
  -drive file=boot.img,if=ide,format=raw
