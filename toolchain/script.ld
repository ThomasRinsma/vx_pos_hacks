ENTRY(main)
OUTPUT_FORMAT(elf32-bigarm)

SECTIONS
{
  . = 0x70420074;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
}