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

	[bits	32]

_phase2_start:
	.confirm_pmode:
		push strings.done
		call _send_serial_bytes
		add esp, 4
	.setup_interrupts:
		push strings.preparing_interrupts
		call _send_serial_bytes
		add esp, 4
		call _prepare_idt
	.setup_cpu_exception_handlers:
		push strings.exceptions
		call _send_serial_bytes
		add esp, 4
		call _install_cpu_exceptions
		push strings.done
		call _send_serial_bytes
		add esp, 4
	.setup_hardware_handlers:
		push strings.hardware
		call _send_serial_bytes
		add esp, 4
		call _install_hardware_interrupts
		push strings.done
		call _send_serial_bytes
		add esp, 4
	.setup_pic:
		push strings.configuring_pic
		call _send_serial_bytes
		add esp, 4
		call _prepare_pic
		push strings.done
		call _send_serial_bytes
		add esp, 4
	.complete_interrupts:
		; Todo
	.setup_pit:
		; Todo
	.setup_basic_paging:
		; Todo
	.detect_boot_media:
		; Todo
	.load_boot_media_driver:
		; Todo
	.detect_boot_filesystem:
		; Todo
	.load_filesystem_driver:
		; Todo
	.build_system_configuration_structure:
		; Todo
	.load_kernel:
		; Todo
	.main:
		cli
		hlt
		jmp $

;;
;; The following are a collection of strings used by CoreLoader phase 2.
;;
strings:
	.done:
		db "done!", 0xA, 0x0
	.preparing_interrupts:
		db "Preparing CPU interrupt handlers:", 0xA, 0x0
	.exceptions:
		db "    Exception handlers... ", 0x0
	.hardware:
		db "    Hardware handlers... ", 0x0
	.configuring_pic:
		db "    Configuring PIC... ", 0x0

;;
;; Include various external source files with required functionality.
;;
	%include "CoreLoader/phase2/serial.s"
	%include "CoreLoader/phase2/idt.s"
	%include "CoreLoader/phase2/exceptions.s"
	%include "CoreLoader/phase2/hardware.s"
	%include "CoreLoader/phase2/pic.s"
