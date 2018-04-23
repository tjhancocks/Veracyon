; Copyright (c) 2017-2018 Tom Hancocks
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.

; This file contains the definitions to multiple aspects of CoreLoader, 
; including structure definitions, CoreLoader memory layout and constants.

BPB_ADDR	equ 0x7c00		; BIOS Parameter Block
GDTPTR_ADDR equ 0x7e00		; Global Descriptor Table Pointer address
IDTPTR_ADDR equ 0x7e10		; Interrupt Descriptor Table Pointer address
TSB_SEG		equ	0x1000		; Temporary Storage Buffer Segment (Real Mode)
TSB_OFFSET	equ	0x0000		; Temporary Storage Buffer Offset (Real Mode)
TSB_ADDR	equ	0x10000		; Temporary Storage Buffer (Protected Mode)
FILE_SEG	equ	0x2000		; File Buffer Segment (Real Mode)
FILE_OFFSET	equ	0x0000		; File Buffer Offset (Real Mode)
FILE_ADDR	equ	0x20000		; File Buffer (Protected Mode)
BC_SEG		equ 0x3000		; Boot Configuration Segment (Real Mode)
BC_OFFSET	equ 0x0000 		; Boot Configuration Offset (Real Mode)
BC_ADDR		equ 0x30000		; Boot Configuration Address (Protected Mode)
CPU_SEG		equ 0x4000		; CPU Data Structures Segment (Real Mode)
GDT_OFFSET	equ 0x0000 		; GDT Offset in CPU Segment (Real Mode)
GDT_ADDR	equ 0x40000		; GDT Address (Protected Mode)
IDT_ADDR	equ 0x40A00		; IDT Address (Protected Mode)
MM_SEG		equ 0x7000		; Memory Map Segment (Real Mode)
MM_OFFSET	equ 0x0000		; Memory Map Offset (Real Mode)
MM_ADDR		equ 0x70000		; Memory Map Address (Protected Mode)

FS_FAT12	equ	0x01		; FAT12 File System
FS_FAT32	equ	0x02		; FAT32 File System

VID_TEXT	equ 0x00 		; VGA Text Video Mode
VID_NATIVE	equ 0x01 		; Native VESA Video Mode, running text console
VID_GRAPH	equ 0x02 		; NAtive VESA Video Mode, running graphical ui

; The Boot Configuration Structure is provided to the Kernel upon completion
STRUC BootConf
	.kernel_image	resb 64	; The name of the kernel image to load
	.boot_media		resb 1	; The type of boot media used for the boot disk
	.vmode			resb 1	; The desired video mode for the system.
	.mmap_addr		resd 1	; Memory Map Address
	.mmap_count		resd 1	; Size of memory map in bytes
	.fs_type		resb 1	; The type of file system of boot device
ENDSTRUC

; E820 Memory Map Entry structure that is returned back from the BIOS.
STRUC MMEntry
	.base_lo		resd 1
	.base_hi		resd 1
	.len_lo			resd 1
	.len_hi 		resd 1
	.type			resd 1
	.acpi			resd 1
ENDSTRUC

; BIOS Parameter Block structure used at the start of FAT file system.s
STRUC BPB
	.jmp_code		resb 3
	.oem_id			resb 8
	.bps			resw 1
	.spc			resb 1
	.reserved		resw 1
	.fats			resb 1
	.root_ents		resw 1
	.sm_sectors		resw 1
	.media			resb 1
	.spf			resw 1
	.spt 			resw 1
	.heads			resw 1
	.hidden			resd 1
	.lg_sectors		resd 1
ENDSTRUC

STRUC EBPB
	.bpb			resb 36
	.drive			resb 1
	.nt_flags		resb 1
	.sig			resb 1
	.volume_id		resd 1
	.volume_label	resb 11
	.sys_id			resb 8
ENDSTRUC

STRUC INTFrame
	.gs				resd 1
	.fs				resd 1
	.es				resd 1
	.ds				resd 1
	.eax			resd 1
	.ecx			resd 1
	.edx			resd 1
	.ebx			resd 1
	.esp			resd 1
	.ebp			resd 1
	.esi			resd 1
	.edi			resd 1
	.int			resd 1
	.err			resd 1
	.eip			resd 1
	.cs				resd 1
	.eflags			resd 1
	.uesp			resd 1
	.ss				resd 1
ENDSTRUC