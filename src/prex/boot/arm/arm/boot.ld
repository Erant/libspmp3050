/*
 * Linker script for boot loader
 */
INCLUDE config.ld

SECTIONS {
	. = 0x24000000 ;
	.bootldr . : {
		*(.bootldr)
	}

	. = LOADER_BASE ;

	.text . : {
		*(.text)
	}
	.rodata . : {
		*(.rodata)
	}
	.data . : {
		*(.data)
	}
	.bss . : {
		*(.bss)
		*(COMMON)
	}
	. = LOADER_BASE + 0x3fff;
	.tail : {
		*(.tail)
	}
}
