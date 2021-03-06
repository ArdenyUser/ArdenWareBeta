#assemble boot.s file
as --32 boot.s -o boot.o

#compile kernel.c file
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

gcc -m32 -c utils.c -o utils.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

gcc -m32 -c char.c -o char.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

gcc -m32 -c start.c -o start.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

#linking the kernel with kernel.o and boot.o files
ld -m elf_i386 -T linker.ld kernel.o boot.o -o Ardenware.bin -nostdlib

#check MyOS.bin file is x86 multiboot file or not
grub-file --is-x86-multiboot Ardenware.bin

#building the iso file
mkdir -p isodir/boot/grub
cp Ardenware.bin isodir/boot/Ardenware.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o ArdenWare.iso isodir

#run it in qemu
qemu-system-x86_64 -cdrom ArdenWare.iso
