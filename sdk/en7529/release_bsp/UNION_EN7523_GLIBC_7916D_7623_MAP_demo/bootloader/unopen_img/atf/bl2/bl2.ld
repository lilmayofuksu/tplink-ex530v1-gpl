OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(bl2_entrypoint)
MEMORY {
    ROM (rx): ORIGIN = (((0x8000000))), LENGTH = ((((0x8000000)) + ((0x20000)))) - (((0x8000000)))
    RAM (rwx): ORIGIN = ((((0x10000000) + 0xE800000) + (0x20000))), LENGTH = ((((0x10000000) + 0xE800000) + (0x20000)) + ((0x20000))) - ((((0x10000000) + 0xE800000) + (0x20000)))
    RAM2 (rwx): ORIGIN = (((0x10000000) + 0xFA40000)), LENGTH = ((0x8000))
}
SECTIONS
{
    . = (((0x8000000)));
    ASSERT(. == ALIGN(((1) << (12))),
           "BL2_RO_BASE address is not aligned on a page boundary.")
    ro . : {
        __RO_START__ = .;
 __TEXT_RESIDENT_START__ = .;
 *bl2_el3_entrypoint.o(.text*)
 *(.text.asm.*)
 __TEXT_RESIDENT_END__ = .;
        *(SORT_BY_ALIGNMENT(.text*))
        *(SORT_BY_ALIGNMENT(.rodata*))
 . = ALIGN(4); __RT_SVC_DESCS_START__ = .; KEEP(*(rt_svc_descs)) __RT_SVC_DESCS_END__ = .; . = ALIGN(4); __FCONF_POPULATOR_START__ = .; KEEP(*(.fconf_populator)) __FCONF_POPULATOR_END__ = .; . = ALIGN(4); __PMF_SVC_DESCS_START__ = .; KEEP(*(pmf_svc_descs)) __PMF_SVC_DESCS_END__ = .; . = ALIGN(4); __PARSER_LIB_DESCS_START__ = .; KEEP(*(.img_parser_lib_descs)) __PARSER_LIB_DESCS_END__ = .; . = ALIGN(4); __CPU_OPS_START__ = .; KEEP(*(cpu_ops)) __CPU_OPS_END__ = .; . = ALIGN(4); __GOT_START__ = .; *(.got) __GOT_END__ = .;
        *(.vectors)
        __RO_END_UNALIGNED__ = .;
        . = ALIGN(((1) << (12)));
        __RO_END__ = .;
    } >ROM
    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
          "cpu_ops not defined for this platform.")
    . = ((((0x10000000) + 0xE800000) + (0x20000)));
    ASSERT(((((0x10000000) + 0xE800000) + (0x20000))) == ALIGN(((1) << (12))),
           "BL2_RW_BASE address is not aligned on a page boundary.")
    __RW_START__ = . ;
    .data . : {
        __DATA_RAM_START__ = .;
        *(SORT_BY_ALIGNMENT(.data*))
        __DATA_RAM_END__ = .;
    } >RAM AT>ROM
    . = ALIGN(8);
    __RELA_START__ = .;
    .rela.dyn . : {
    } >RAM
    __RELA_END__ = .;
    stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >RAM
    .bss (NOLOAD) : ALIGN(8) { __BSS_START__ = .; *(SORT_BY_ALIGNMENT(.bss*)) *(COMMON) . = ALIGN(((1) << 6)); __PMF_TIMESTAMP_START__ = .; KEEP(*(pmf_timestamp_array)) . = ALIGN(((1) << 6)); __PMF_PERCPU_TIMESTAMP_END__ = .; __PERCPU_TIMESTAMP_SIZE__ = ABSOLUTE(. - __PMF_TIMESTAMP_START__); . = . + (__PERCPU_TIMESTAMP_SIZE__ * ((2) - 1)); __PMF_TIMESTAMP_END__ = .; . = ALIGN(16); *(base_xlat_table) __BSS_END__ = .; } >RAM
    xlat_table (NOLOAD) : { *(xlat_table) } >RAM2
    coherent_ram (NOLOAD) : ALIGN(((1) << (12))) {
        __COHERENT_RAM_START__ = .;
        *(tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;
        . = ALIGN(((1) << (12)));
        __COHERENT_RAM_END__ = .;
    } >RAM
    __RW_END__ = .;
    __BL2_END__ = .;
    /DISCARD/ : {
        *(.dynsym .dynstr .hash .gnu.hash)
    }
    __BL2_RAM_START__ = ADDR(.data);
    __BL2_RAM_END__ = .;
    __DATA_ROM_START__ = LOADADDR(.data);
    __DATA_SIZE__ = SIZEOF(.data);
    __BL2_ROM_END__ = __DATA_ROM_START__ + __DATA_SIZE__;
    ASSERT(__BL2_ROM_END__ <= ((((0x8000000)) + ((0x20000)))),
           "BL2's RO content has exceeded its limit.")
    __BSS_SIZE__ = SIZEOF(.bss);
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
    ASSERT(. <= ((((0x10000000) + 0xE800000) + (0x20000)) + ((0x20000))), "BL2's RW content has exceeded its limit.")
}
