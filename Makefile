CFLAGS = -ffreestanding -g -m32

all: asm link run

asm:
	nasm -felf32 "Source/Loader/boot.s" -f bin -o "Binaries/boot.bin"
	nasm -felf32 "Source/Loader/extboot.s" -f bin -o "Binaries/extboot.bin"

	cat "Binaries/boot.bin" "Binaries/extboot.bin" > "Binaries/primary.bin"


	/usr/local/i386elfgcc/bin/i386-elf-gcc $(CFLAGS) -c Source/kernel.cpp -o Binaries/kernel.o
	nasm "Source/Loader/kernel_entry.s" -f elf -o "Binaries/kernel_entry.o"


link:
	/usr/local/i386elfgcc/bin/i386-elf-ld -o "Binaries/full.bin" -Ttext 0x8000 "Binaries/kernel_entry.o" "Binaries/kernel.o" --oformat binary

run:
	dd if=Binaries/primary.bin of=main.img bs=512 count=63
	dd if=Binaries/full.bin of=main.img bs=512 seek=2

	# Super important to append font, used for debugging!
	dd if=Assets/font.bin of=main.img bs=512 seek=63

	dd if=/dev/zero bs=1 count=99999 >> main.img

	qemu-system-x86_64 \
	-device sb16 -audio id=snd0,driver=alsa \
    -drive file=main.img,format=raw,index=0,if=ide \
	-cpu qemu64 \
    -m 128M \
    -d int \
    -no-reboot

# sudo dd if=main.img of=/dev/sda bs=4M status=progress && sync