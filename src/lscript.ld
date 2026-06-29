/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/* Tamano de stack y heap: usa el valor definido externamente o el valor por defecto */
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
_HEAP_SIZE  = DEFINED(_HEAP_SIZE)  ? _HEAP_SIZE  : 0x800;

/* Memorias del sistema:
   mig_0      - DDR2 externo, 128 MB, usado para datos de imagen y buffers grandes
   lmb_bram_0 - BRAM local, 16 KB, usado para codigo y datos del programa */
MEMORY
{
    mig_0      : ORIGIN = 0x80000000, LENGTH = 0x8000000
    lmb_bram_0 : ORIGIN = 0x0,        LENGTH = 0x4000
}

/* Punto de entrada del programa */
ENTRY(_boot)

SECTIONS
{
    /* Codigo de arranque (boot), debe quedar al inicio de BRAM */
    .text.init : {
        __text.init_start = .;
        *(.text.init)
        __text.init_end = .;
    } > lmb_bram_0

    /* Codigo del programa; alineado a 2048 bytes por requisito del vector de interrupciones */
    .text : {
        . = ALIGN(2048);
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    } > lmb_bram_0

    .note.gnu.build-id : {
        KEEP (*(.note.gnu.build-id))
    } > lmb_bram_0

    /* Rutinas de inicializacion y finalizacion del runtime de C */
    .init : { KEEP (*(.init)) } > lmb_bram_0
    .fini : { KEEP (*(.fini)) } > lmb_bram_0

    /* Tablas de constructores y destructores globales de C++ */
    .ctors : {
        __CTOR_LIST__ = .;
        ___CTORS_LIST___ = .;
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
        __CTOR_END__ = .;
        ___CTORS_END___ = .;
    } > lmb_bram_0

    .dtors : {
        __DTOR_LIST__ = .;
        ___DTORS_LIST___ = .;
        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
        __DTOR_END__ = .;
        ___DTORS_END___ = .;
    } > lmb_bram_0

    /* Datos de solo lectura */
    .rodata : {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        __rodata_end = .;
    } > lmb_bram_0

    /* Datos de solo lectura accesibles via puntero global pequeño (gp-relative) */
    .srodata : {
        __srodata_start = .;
        *(.srodata)
        __srodata_end = .;
    } > lmb_bram_0

    .sdata2 : {
        __sdata2_start = .;
        *(.sdata2)
        *(.sdata2.*)
        *(.gnu.linkonce.s2.*)
        __sdata2_end = .;
    } > lmb_bram_0

    /* Datos inicializados */
    .data : {
        __data_start = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
        _edata = .;
    } > lmb_bram_0

    /* Tablas GOT para direccionamiento relativo al PC (PIC) */
    .got  : { *(.got)  } > lmb_bram_0
    .got1 : { *(.got1) } > lmb_bram_0
    .got2 : { *(.got2) } > lmb_bram_0

    .jcr            : { *(.jcr)            } > lmb_bram_0
    .gcc_except_table : { *(.gcc_except_table) } > lmb_bram_0

    /* Arreglos de funciones de inicializacion y finalizacion (ejecutados por el runtime) */
    .init_array : {
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
    } > lmb_bram_0

    .fini_array : {
        __fini_array_start = .;
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array))
        __fini_array_end = .;
    } > lmb_bram_0

    /* Datos inicializados accesibles via gp (small data area) */
    .sdata : {
        __sdata_start = .;
        *(.sdata)
        *(.sdata.*)
        *(.gnu.linkonce.s.*)
        __sdata_end = .;
    } > lmb_bram_0

    /* Datos de almacenamiento local por hilo (TLS) */
    .tdata : {
        __tdata_start = .;
        *(.tdata)
        *(.tdata.*)
        *(.gnu.linkonce.td.*)
        __tdata_end = .;
    } > lmb_bram_0

    .tbss : {
        __tbss_start = .;
        *(.tbss)
        *(.tbss.*)
        *(.gnu.linkonce.tb.*)
        __tbss_end = .;
    } > lmb_bram_0

    /* Datos no inicializados (se ponen a cero en tiempo de arranque) */
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    } > lmb_bram_0

    .sbss (NOLOAD) : {
        __sbss_start = .;
        *(.sbss)
        *(.sbss.*)
        *(.gnu.linkonce.sb.*)
        __sbss_end = .;
    } > lmb_bram_0

    .sbss2 : {
        __sbss2_start = .;
        *(.sbss2)
        *(.sbss2.*)
        *(.gnu.linkonce.sb2.*)
        __sbss2_end = .;
    } > lmb_bram_0
