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

start:
	.basic_setup:
		cli
		call init_serial
	.handshake:
		mov si, strings16.welcome_serial
		call send_serial_bytes
	.boot_config:
		mov si, strings16.building_boot_configuration
		call send_serial_bytes
		call prepare_boot_configuration_defaults
		mov si, strings16.done
		call send_serial_bytes
		mov si, strings16.reading_boot_configuration
		call send_serial_bytes
		mov si, strings16.skipped
		call send_serial_bytes
	.prepare_pmode:
		mov si, strings16.preparing_pmode
		call send_serial_bytes
		call gdt_install
	.enable_a20:
		in al, 0x92
        or al, 2
        out 0x92, al
    .enable_32bit:
    	mov eax, cr0
        or al, 1
        mov cr0, eax
	.enter_phase2:
		mov ax, 0x10					; Kernel Data Segment
		mov ds, ax
		mov eax, 0x10800				; GDT Pointer linear address
		lgdt [eax]	
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
		db "Copyright (c) 2017 Tom Hancocks. MIT License.", 0xA, 0xA, 0x0
	.building_boot_configuration:
		db "Building default boot configuration	... ", 0x0
	.reading_boot_configuration:
		db "Reading boot configuration file... ", 0x0
	.done:
		db "done.", 0xA, 0x0
	.skipped:
		db "skipped.", 0xA, 0x0
	.preparing_pmode:
		db "Preparing to setup and configure protected mode on primary CPU... ",
		db 0x0 

;;
;; Include various external source files with required functionality.
;;
	%include "CoreLoader/phase1/serial.s"
	%include "CoreLoader/phase1/gdt.s"
	%include "CoreLoader/phase1/config.s"