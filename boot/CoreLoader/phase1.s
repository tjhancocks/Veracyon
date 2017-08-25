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

_start:
	.basic_setup:
		call _init_serial
	.handshake:
		mov si, strings.welcome_serial
		call _send_serial_bytes
	.prepare_pmode:
		mov si, strings.preparing_pmode
		call _send_serial_bytes
	.stack_guard:
		cli
		hlt
		jmp $

;;
;; The following are a collection of strings used by CoreLoader phase 1.
;;
strings:
	.welcome_serial:
		db "CORELOADER VERSION 0.2", 0xA
		db "Copyright (c) 2017 Tom Hancocks. MIT License.", 0xA, 0xA, 0x0
	.preparing_pmode:
		db "Preparing to setup and configure protected mode on primary CPU", 0xA
		db 0x0

;;
;; Include various external source files with required functionality.
;;
	%include "CoreLoader/phase1/serial.s"