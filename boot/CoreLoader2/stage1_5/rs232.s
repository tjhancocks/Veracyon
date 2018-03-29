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

; Serial Port definitions used on the x86 platform.
%define COM1_PORT			0x3F8
%define COM2_PORT			0x2F8
%define COM3_PORT			0x3E8
%define COM4_PORT			0x2E8

; Initialise the COM1 RS232 serial port for use. This will ensure we are using
; the correct baud rate and appropriate interrupts are configured.
rs232.init:
	.prologue:
		pusha
	.main:
		mov al, 0x00
		mov dx, COM1_PORT + 1
		out dx, al				; Disable all interrupts
		mov al, 0x80
		mov dx, COM1_PORT + 3
		out dx, al				; Enable DLAB (Baud rate divisor)
		mov al, 0x03
		mov dx, COM1_PORT
		out dx, al				; Set divisor to 3 (Low byte) 38400
		mov al, 0x00
		mov dx, COM1_PORT + 1
		out dx, al				; 				   (High byte)
		mov al, 0x03
		mov dx, COM1_PORT + 3
		out dx, al				; 8 bits, no parity, one stop bit
		mov al, 0xc7
		mov dx, COM1_PORT + 2
		out dx, al				; Enable FIFO, clear them, with 14B threshold
		mov al, 0x0b
		mov dx, COM1_PORT + 4
		out dx, al				; IRQs enabled, RTS/DSR set
	.epilogue:
		popa
		ret

; Write a single byte to the COM1 RS232 serial port.
;	IN: al -> byte to send
rs232.send_byte:
	.prologue:
		pusha
		push ax
	.test_transmit:
		mov dx, COM1_PORT + 5
		in al, dx
		and al, 0x20
		jz .test_transmit
	.transmit:
		pop ax
		mov dx, COM1_PORT
		out dx, al
	.epilogue:
		popa
		ret

; Write a stream of bytes to the COM1 RS232 serial port, until a NUL byte is
; encountered.
; 	IN: si -> address of the first byte to send
rs232.send_bytes:
	.prologue:
		pusha
	.next_byte:
		lodsb
		or al, al
		jz .epilogue
		call rs232.send_byte
		jmp .next_byte
	.epilogue:
		popa
		ret

