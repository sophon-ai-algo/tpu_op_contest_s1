OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
MEMORY
{
  itcm (xr)	: ORIGIN = 0x00000000, LENGTH = 0x00080000
  dtcm (xrw)	: ORIGIN = 0x02000000, LENGTH = 0x0001BE80
  ddr  (xrw)	: ORIGIN = 0x80000000, LENGTH = 0x01000000
}

STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : 0x4000;
IRQ_STACK_SIZE = DEFINED(IRQ_STACK_SIZE) ? IRQ_STACK_SIZE : 0x400;
FIQ_STACK_SIZE = DEFINED(FIQ_STACK_SIZE) ? FIQ_STACK_SIZE : 0x400;
EXPT_STACK_SIZE = DEFINED(EXPT_STACK_SIZE) ? EXPT_STACK_SIZE : 0x100;
ENTRY(start)
SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        __rom_start = .;
        *(.text.start)
        *(.text)
        *(.text.*ssd_detect_out*)
        *(.text.firmware_main)

        *(.text.gde_gather_data_gen_cmd)
        *(.text.atomic_md_sfu_gen_cmd)
        *(.text.atomic_fc_gen_cmd)
        *(.text.atomic_vec_correlation_gen_cmd)
        *(.text.atomic_tensor_arithmetic_gen_cmd)
        *(.text.atomic_table_lookup_gen_cmd)
        *(.text.atomic_mdscalar_operation_gen_cmd)
        *(.text.atomic_md_linear_gen_cmd)
        *(.text.general_gdma_gen_cmd)
        *(.text.tensor_general_move_gen_cmd)
        *(.text.tensor_stride_move_gen_cmd)
        *(.text.tensor_align_move_gen_cmd)
        *(.text.atomic_tensor_arithmetic_fix8b_gen_cmd)
        *(.text.atomic_conv_neuron_kernel_stride_fix8b_gen_cmd)
        *(.text.atomic_conv_neuron_kernel_stride_bias_stride_fix8b_gen_cmd)
        *(.text.atomic_conv_neuron_unalign_kstride_gen_cmd)
        *(.text.atomic_conv_neuron_kernel_stride_gen_cmd)
        *(.text.atomic_conv_neuron_gen_cmd)
        *(.text.atomic_conv_kernel_stride_gen_cmd)
        *nodechip_topk.o(.text*)
        *nodechip_fc_fix8b_parallel.o(.text*)
        *gdma_store_mode_transfer.o(.text*)
        *nodechip_multi_fullnet.o(.text*)

        *nodechip_cv_resize.o(.text*)
        *nodechip_cv_convert_to.o(.text*)
        *nodechip_cv_storage_convert.o(.text*)

        *nodechip_cv_yuv2rgb.o(.text*)
        *nodechip_cv_width_align.o(.text*)

        *nodechip_topk.c.obj(.text*)
        *nodechip_fc_fix8b_parallel.c.obj(.text*)
        *gdma_store_mode_transfer.c.obj(.text*)
        *nodechip_multi_fullnet.c.obj(.text*)
        *nodechip_cv_resize.c.obj(.text*)
        *nodechip_cv_convert_to.c.obj(.text*)
        *nodechip_cv_storage_convert.c.obj(.text*)
        *nodechip_cv_yuv2rgb.c.obj(.text*)
        *nodechip_cv_width_align.c.obj(.text*)

        KEEP (*(.init))
        KEEP (*(.fini))
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(8);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
        . = ALIGN(8);
    } > itcm

    __exidx_start = .;
      .ARM.exidx   : {
          *(.ARM.exidx*)
      } > itcm
    __exidx_end = .;

    __rom_end = LOADADDR(.data);
    __bin_end = LOADADDR(.data) + SIZEOF(.data);

    .data : {
        . = ALIGN(8);
        __data_start = .;
        *(.ramfunc .ramfunc.*)
        *(.data .data.*)
        *(*.got*)
        *(*.plt*)
        . = ALIGN(8);
        __data_end = .;
    } > dtcm AT > itcm


    .bss (NOLOAD) : {
        . = ALIGN(0x1000);
        __bss_start = .;
        *(.bss.heap)
        . = ALIGN(8);
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } > dtcm

    .stack (NOLOAD) : {
        . = ALIGN(8);
        __expt_stack_end = . ;
        . = . + EXPT_STACK_SIZE;
        . = ALIGN(8);
        __expt_stack_start = . ;

        . = ALIGN(8);
        __fiq_stack_end = . ;
        . = . + FIQ_STACK_SIZE;
        . = ALIGN(8);
        __fiq_stack_start = . ;

        . = ALIGN(8);
        __irq_stack_end = . ;
        . = . + IRQ_STACK_SIZE;
        . = ALIGN(8);
        __irq_stack_start = . ;

        . = ALIGN(8);
        __stack_end = . ;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        __stack_start = . ;
    } > dtcm

    .text.slow : {
        . = ALIGN(4);
        /* add any function section as below */
        *(.text.__slow_*)

        *(.text.*bm_api*)
        *nodechip_*.o(.text*)
        *dynamic*.o(.text*)
        *bm_device_*.o(.text*)
        *ok_device_*.o(.text*)
        *bmkernel_*.o(.text*)

        *nodechip_*.c.obj(.text*)
        *dynamic*.c.obj(.text*)
        *bm_device_*.c.obj(.text*)
        *bmkernel_*.c.obj(.text*)

        . = ALIGN(4);
    } > ddr
}
