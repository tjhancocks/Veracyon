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
;; Initialize the COM1 Serial Port. This is useful for debugging purposes.
;;
init_serial:
	.configure:
		mov al, 0x00
		mov dx, 0x03f9
		out dx, al			; Disable all interrupts
		mov al, 0x80
		mov dx, 0x03fb
		out dx, al			; Enable DLAB (Baud rate divisor)
		mov al, 0x03
		mov dx, 0x03f8
		out dx, al			; Set divisor to 3 (Low byte) 38400
		mov al, 0x00
		mov dx, 0x03f9
		out dx, al			; 				   (High byte)
		mov al, 0x03
		mov dx, 0x03fb
		out dx, al			; 8 bits, no parity, one stop bit
		mov al, 0xc7
		mov dx, 0x03fa
		out dx, al			; Enable FIFO, clear them, with 14B threshold
		mov al, 0x0b
		mov dx, 0x03fc
		out dx, al			; IRQs enabled, RTS/DSR set
	.finish:
		ret

;;
;; Write a single specified character to the COM1 Serial Port.
;; This will not clobber registers.
;;
;;	IN => AL - The byte to send.
;;
send_serial_byte:
	.prepare:
		pusha
		push ax
	.test_can_transmit:
		mov dx, 0x03fd
		in al, dx
		and al, 0x20
		jz .test_can_transmit
	.transmit:
		pop ax
		mov dx, 0x03f8
		out dx, al
	.finish:
		popa
		ret

;;
;; Keep sending bytes over the COM1 serial port, starting from the specified
;; memory address, until a NULL byte is encountered.
;; This will not clobber registers.
;;
;; 	IN => DS:SI - The memory location of the first byte to send.
;;
send_serial_bytes:
	.prepare:
		pusha
	.next:
		lodsb
		or al, al
		jz .finish
		call send_serial_byte
		jmp .next
	.finish:
		popa
		ret
