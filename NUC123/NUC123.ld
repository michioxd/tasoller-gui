/* Linker script to configure memory regions. */
MEMORY {
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x10000   /* 64k */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x5000    /* 20k */
}

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/**
 * Used for loading .data into memory:
 *   __etext
 *   __data_start__
 *   __data_end__
 * Used for zeroing .bss:
 *   __bss_start__
 *   __bss_end__
 * Used for stack setup:
 *  __StackLimit
 *  __StackTop
 * Used for heap setup:
 *  __HeapBase
 *  __HeapLimit
 */
ENTRY(Reset_Handler)

SECTIONS {
	.text : {
		KEEP(*(.vectors))
		. = . + 16;
		/* KEEP(*(.init)) */
		/* KEEP(*(.text._entry)) */
		*(.text*)
		*(.rodata*)
	} > FLASH

	. = ALIGN(4);
	__etext = .;

	.data : AT (__etext) {
		. = ALIGN(4);
		__data_start__ = .;
		*(.data*)
		. = ALIGN(4);
		__data_end__ = .;
	} > RAM

	.bss : {
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM

	.heap (COPY): {
		__HeapBase = .;
		KEEP(*(.heap*))
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY): {
		KEEP(*(.stack*))
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
