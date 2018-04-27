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

	[bits	16]

start:
	.basic_setup:
		cli
		call init_serial
	.handshake:
		mov si, strings16.welcome_serial
		call send_serial_bytes
	.detect_memory:
		mov si, strings16.detecting_lower_memory
		call send_serial_bytes
		call detect_lower_memory
		mov si, strings16.reading_memory_map
		call send_serial_bytes
		call read_memory_map
		jc .mmap_failed
		jmp .mmap_done
	.mmap_failed:
		mov si, strings16.e820_failed
		call send_serial_bytes
		call dbg_put_string
		cli
		hlt
		jmp $
	.mmap_done:
		mov si, strings16.done
		call send_serial_bytes
		mov si, strings16.calculating_memory
		call send_serial_bytes
		call calculate_total_memory
		call send_serial_number
		mov si, strings16.mib
		call send_serial_bytes
	.boot_config:
		mov si, strings16.building_boot_configuration
		call send_serial_bytes
		call prepare_boot_configuration_defaults
		mov si, strings16.done
		call send_serial_bytes
		mov si, strings16.reading_boot_configuration
		call send_serial_bytes
		call load_boot_configuration_fat12
	.vesa:
		mov si, strings16.preparing_vesa
		call send_serial_bytes
		call prepare_vesa
	.prepare_pmode:
		mov si, strings16.preparing_pmode
		call send_serial_bytes
		call gdt_install
	.enable_a20:
		mov si, strings16.a20_line
		call send_serial_bytes
		in al, 0x92
        or al, 2
        out 0x92, al
		mov si, strings16.done
		call send_serial_bytes
    .enable_32bit:
		mov si, strings16.setting_32bit
		call send_serial_bytes
    	mov eax, cr0
        or al, 1
        mov cr0, eax
		mov si, strings16.done
		call send_serial_bytes
	.enter_phase2:
		cli
		mov si, strings16.far_jump
		call send_serial_bytes
		mov eax, BOOT_CONFIG
		lgdt [eax + BootConf.gdt_size]	
		mov ax, 0x10					; Kernel Data Segment
		mov ss, ax	
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		jmp 0x08:_phase2_start
	.stack_guard:
		cli
		hlt
		jmp $

;;
;; The following are a collection of strings used by CoreLoader phase 1.
;;
strings16:
	.welcome_serial:
		db "CORELOADER VERSION 0.2", 0xA
		db "Copyright (c) 2017-2018 Tom Hancocks. MIT License.", 0xA, 0xA, 0x0
	.building_boot_configuration:
		db "Building default boot configuration... ", 0x0
	.a20_line:
		db "    Enabling A20 Line... ", 0x0
	.setting_32bit:
		db "    Setting 32-bit mode... ", 0x0
	.far_jump:
		db "    Performing far jump... ", 0x0
	.reading_boot_configuration:
		db "Reading boot configuration file... ", 0x0
	.preparing_vesa:
		db "Preparing VESA screen mode... ", 0x0
	.done:
		db "done.", 0xA, 0x0
	.skipped:
		db "skipped.", 0xA, 0x0
	.missing:
		db "missing.", 0xA, 0x0
	.unavailable:
		db "unavailable.", 0xA, 0x0
	.unavailable1:
		db "unavailable (1).", 0xA, 0x0
	.unavailable2:
		db "unavailable (2).", 0xA, 0x0
	.unavailable3:
		db "unavailable (3).", 0xA, 0x0
	.unavailable4:
		db "unavailable (4).", 0xA, 0x0
	.unavailable5:
		db "unavailable (5).", 0xA, 0x0
	.unavailable6:
		db "unavailable (6).", 0xA, 0x0
	.preparing_pmode:
		db "Preparing to setup and configure protected mode on primary CPU... ",
		db 0xA, 0x0 
	.detecting_lower_memory:
		db "Detecting lower memory... ", 0x0
	.reading_memory_map:
		db "Reading memory map... ", 0x0
	.calculating_memory:
		db "Calculating total memory... ", 0x0
	.kib:
		db "KiB", 0xA, 0x0
	.mib:
		db "MiB", 0xA, 0x0
	.e820_failed:
		db "E820 Failed!", 0xA, 0x0

;;
;; Include various external source files with required functionality.
;;
	%include "CoreLoader/phase1/defines.s"
	%include "CoreLoader/phase1/serial.s"
	%include "CoreLoader/phase1/gdt.s"
	%include "CoreLoader/phase1/config.s"
	%include "CoreLoader/phase1/disk.s"
	%include "CoreLoader/phase1/fat12.s"
	%include "CoreLoader/phase1/vesa.s"
	%include "CoreLoader/phase1/memory.s"
