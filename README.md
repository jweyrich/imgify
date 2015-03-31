# imgify - convert any file to png and back

## bin2png

	usage: bin2png -i <in_filename> -o <out_filename> [-p <pad_byte>]

Example:

	./bin2png -i /mach_kernel -o mach_kernel.png
	Input file => /mach_kernel
	  size => 8394688 bytes
	Output file => mach_kernel.png
	  size => 4524970 bytes
	  image => 1449x1449 px, 32 bpp, 929px padding

## png2bin

	usage: png2bin -i <in_filename> -o <out_filename> [-p <pad_byte>

Example:

	./png2bin -i mach_kernel.png -o mach_kernel.bin
	Input file => mach_kernel.png
	  size => 4524970 bytes
	  image => 1449x1449 px, 32 bpp, 929px padding
	Output file => mach_kernel.bin
	  size => 8394688 bytes
	  image => 1449x1449 px, 32 bpp