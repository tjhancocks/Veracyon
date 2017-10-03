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

;;
;;
;; Write a single specified character to the COM1 Serial Port.
;; This will not clobber registers.
;;
;;	void send_serial_byte(unsigned char value)
;;
_send_serial_byte:
	.prepare:
		push ebp
		mov ebp, esp
		pusha
	.test_can_transmit:
		mov dx, 0x03fd
		in al, dx
		and al, 0x20
		jz .test_can_transmit
	.transmit:
		mov eax, [ebp + 8]
		mov dx, 0x03f8
		out dx, al
	.finish:
		popa
		mov esp, ebp
		pop ebp
		ret

;;
;; Keep sending bytes over the COM1 serial port, starting from the specified
;; memory address, until a NULL byte is encountered.
;; This will not clobber registers.
;;
;;  void send_serial_bytes(unsigned char *bytes)
;;
_send_serial_bytes:
	.prepare:
		push ebp
		mov ebp, esp
		pusha
		mov esi, [ebp + 8]
	.next:
		lodsb
		or al, al
		jz .finish
		push eax
		call _send_serial_byte
		add esp, 4
		jmp .next
	.finish:
		popa
		mov esp, ebp
		pop ebp
		ret

;;
;; Write the specified value to the serial port as a decimal string
;;
_send_serial_number:
	.prologue:
		push ebp
		mov ebp, esp
	.prepare:
		mov edi, .buffer
		mov ecx, 31
		xor eax, eax
		rep stosb
		mov eax, [ebp + 8]
		mov edi, .buffer + 30
		mov ecx, 10
	.next_digit:
		xor edx, edx
		idiv ecx
		add edx, 0x30
		dec edi
		mov byte[edi], dl
		cmp eax, 0
		jnz .next_digit
	.done:
		push edi
		call _send_serial_bytes
		add esp, 4
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.buffer:
		times 31 db 0