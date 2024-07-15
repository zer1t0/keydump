
SECTIONS
{
  /* start in 0x1000 to avoid errors with header segment */
  . = 0x1000;
  /*Align .text to page boundary. This is required to prevent errors when
  changing memory protection.*/
  .text ALIGN(0x1000) : {
        *start.asm* (.text$start)
        *.c.o*(.text*)
        *syscall_z.asm*(.text)
        *(.rodata*)
        FILL( 0x00 )

        /*Align data to page boundary to prevent errors when changing
        memory protection*/
        . = ALIGN( 0x1000 );
        __data_offset = .;
        *(.data*)
        *start.asm*(.text$end)
  }
}
