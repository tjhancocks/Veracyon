; Copyright (c) 2017 Tom Hancocks
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

	[bits	16]

;;
;; The following definitions define the locations of various structures in
;; memory.
;;
	BIOS_PARAM_BLOCK	equ 0x7c00
	VESA_INFO			equ 0xF000
	VBE_MODE			equ 0xF200
	EDID_INFO			equ 0xF400
	SCREEN_CONFIG		equ 0xF600
	BOOT_CONFIG			equ 0xFE00
	GDT_BASE			equ 0x10000
	IDT_BASE			equ	0x11000
	IDT_PTR				equ	0x11800
	PANIC_FN_PTR		equ 0x11806
	INT_HANDLERS		equ 0x11900
	PAGE_DIR			equ 0x12000
	ID_MAP_PAGE_TABLE	equ 0x13000
	KERNEL_PAGE_TABLE_1	equ	0x14000
	VESA_PAGE_TABLE_1	equ 0x17000
	DMA_BUFFER			equ 0x20000
	FILE_BUFFER			equ 0x30000

;;
;; The following defines the layout of the BIOS Parameter Block (Bootsector)
;;
STRUC BPBlock
	.jmp_code			resb 3
	.oem_id             resb 8
	.bps                resw 1
	.spc                resb 1
	.reserved_count     resw 1
	.fat_count          resb 1
	.dir_entries        resw 1
	.total_sectors      resw 1
	.media_descriptor   resb 1
	.spf                resw 1
	.spt                resw 1
	.head_count         resw 1
	.hidden_sectors     resd 1
	.unknown            resd 1
	.drive              resb 1
	.dirty              resb 1
	.extended_sig       resb 1
	.volume_id          resd 1
	.volume_label       resb 11
	.fs_type            resb 8
ENDSTRUC

;;
;; The following defines the layout of the Boot Configuration structure.
;;
STRUC BootConf
	.vesa_mode			resb 1
	.kernel_name		resb 31
	.kernel_base		resd 1
	.width				resw 1
	.height				resw 1
	.depth				resb 1
	.lfb				resd 1
	.background_color	resd 1
	.bytes_per_pixel	resd 1
	.bytes_per_line		resd 1
	.screen_size		resd 1
	.x_max				resd 1
	.y_max				resd 1
	.x					resd 1
	.y					resd 1
	.gdt_size			resw 1
	.gdt_base			resd 1
	.lower_memory		resw 1
	.upper_memory		resd 1
ENDSTRUC

;;
;; The following defines the layout of the VESA VBE Info structure.
;;
STRUC VBEInfo
	.signature 			resb 4
	.version 			resw 1
	.oem				resd 1
	.capabilities		resd 1
	.video_modes_off	resw 1
	.video_modes_seg	resw 1
	.video_memory		resw 1
	.software_revision	resw 1
	.vendor				resd 1
	.product_name		resd 1
	.product_revision	resd 1
	.reserved			resb 222
	.oem_data			resb 256
ENDSTRUC

;;
;; The following defines the layout of the EDID structure. EDID is used by the
;; hardware to instruct the software of its preferred/native resolution.
;;
STRUC EDID
	.padding 			resb 8
	.manufacturer_id	resw 1
	.edid_code			resw 1
	.serial_code		resd 1
	.week_number		resb 1
	.manufacturer_year	resb 1
	.edid_version		resb 1
	.edid_revision		resb 1
	.video_input		resb 1
	.width_cm			resb 1
	.height_cm			resb 1
	.gamma_factor		resb 1
	.dpms_flags			resb 1
	.chroma				resb 10
	.timings1			resb 1
	.timings2			resb 1
	.reserved_timing	resb 1
	.standard_timings	resw 8
	.timing_desc1		resb 18
	.timing_desc2		resb 18
	.timing_desc3		resb 18
	.timing_desc4		resb 18
	.reserved			resb 1
	.checksum			resb 1
ENDSTRUC

;;
;; The following defines the layout of the VBE Mode Info structure. This
;; structure describes the configuration and how to use the mode.
;;
STRUC VBEMode
	.attributes			resb 2
	.window_a			resb 1
	.window_b 			resb 1
	.granularity		resb 2
	.window_size		resb 2
	.segment_a			resb 2
	.segment_b			resb 2
	.win_func_ptr		resb 4
	.pitch				resb 2
	.width				resb 2
	.height				resb 2
	.w_char				resb 1
	.h_char				resb 1
	.planes				resb 1
	.bpp				resb 1
	.banks				resb 1
	.memory_model		resb 1
	.bank_size			resb 1
	.image_pages		resb 1
	.reserved0			resb 1
	.red_mask			resb 1
	.red_position		resb 1
	.green_mask			resb 1
	.green_position		resb 1
	.blue_mask			resb 1
	.blue_position		resb 1
	.reserved_mask		resb 1
	.reserved_position	resb 1
	.direct_color_attr	resb 1
	.frame_buffer		resb 4
	.off_screen_mem_off	resb 4
	.off_screen_mem_sz	resb 2
	.reserved1			resb 206
ENDSTRUC

;;
;; The following defines the layout of the screen configuration structure.
;; This is used to keep track of the VESA VBE configuration being used.
;;
STRUC ScreenConf
	.width				resb 2
	.height				resb 2
	.depth				resb 2
ENDSTRUC