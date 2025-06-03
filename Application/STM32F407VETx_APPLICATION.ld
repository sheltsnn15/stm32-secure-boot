/* Application firmware linker script - starts at 0x08010000 (after bootloader) */

ENTRY(Reset_Handler)

/* Define memory layout */
MEMORY
{
  FLASH   (rx)  : ORIGIN = 0x08010000, LENGTH = 448K  /* after 64KB bootloader */
  RAM     (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  CCMRAM  (xrw) : ORIGIN = 0x10000000, LENGTH = 64K
}

/* Heap and stack size (adjust if needed) */
_Min_Heap_Size  = 0x200;  /* 512 bytes */
_Min_Stack_Size = 0x400;  /* 1 KB */

__app_start = ORIGIN(FLASH);  /* Optional: app base address */

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Vector table */
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)           /* Code */
    *(.text*)          /* Additional code */
    *(.rodata)         /* Read-only data */
    *(.rodata*)        /* Const data */
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  /* Initialized data section - runs from RAM but loaded from FLASH */
  _sidata = LOADADDR(.data);
  .data : AT (_sidata)
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM

  /* Uninitialized data section (zero-initialized at startup) */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM

  /* CCMRAM section (for fast-access variables, optional) */
  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;
    *(.ccmram)
    *(.ccmram*)
    . = ALIGN(4);
    _eccmram = .;
  } >CCMRAM

  /* Heap and stack area */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Remove unwanted standard library sections */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
