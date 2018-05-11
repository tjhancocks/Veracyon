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

	BITS 	32

; Serial Port definitions used on the x86 platform.
%define _COM1_PORT			0x3F8
%define _COM2_PORT			0x2F8
%define _COM3_PORT			0x3E8
%define _COM4_PORT			0x2E8

; RS232 internal constants
_BUFFER_LEN equ 16

; Write a single byte to the COM1 RS232 serial port.
;
;	void _rs232.send_byte(char c);
;
_rs232.send_byte:
	.prologue:
		push ebp
		mov ebp, esp
	.test_transmit:
		mov dx, _COM1_PORT + 5
		in al, dx
		and al, 0x20
		jz .test_transmit
	.transmit:
		mov eax, [ebp + 8]				; Fetch 'c' from the stack
		mov edx, _COM1_PORT
		out dx, al
	.epilogue:
		leave
		ret

; Write a stream of bytes to the COM1 RS232 serial port.
;
;	void _rs232.send_bytes(const char *restrict str)
;
_rs232.send_bytes:
	.prologue:
		push ebp
		mov ebp, esp
		mov esi, [ebp + 8]				; Fetch 'str' from the stack
	.@@:
		lodsb 							; Load the next byte from '*ESI'
		or al, al						; Was it a NUL byte?
		jz .epilogue					; If so finish up...
		push eax						; Arg #0: 'c'
		call _rs232.send_byte			; ...otherwise send it to the link.
		add esp, 4
		jmp .@@							; Next byte 
	.epilogue:
		leave
		ret

; Write a stream n bytes to the COM1 RS232 serial port, or until a NUL byte is
; encountered.
;
;	void _rs232.send_counted_bytes(const char *restrict str, int n)
;
_rs232.send_counted_bytes:
	.prologue:
		push ebp
		mov ebp, esp
		mov esi, [ebp + 8]				; Fetch 'str' from the stack
		mov ecx, [ebp + 12]				; Fetch 'n' from the stack
	.@@:
		push ecx						; Save the current 'n' value
		lodsb 							; Load the next byte from '*ESI'
		or al, al						; Was it a NUL byte?
		jz .epilogue					; If so finish up...
		push eax						; Arg #0: 'c'
		call _rs232.send_byte			; ...otherwise send it to the link.
		add esp, 4
		pop ecx							; Restore the current 'n' value
		loop .@@						; Loop to the next byte if any remaining
	.epilogue:
		leave
		ret

; Write the specified value to the COM1 RS232 serial port in a textual
; representation.
;
; NOTE: If the value is specified to be in base-2 then the "b" prefix will
; be applied. If the value is specified to be in base-8 then the "O" prefix
; will be applied. Finally if the value is specified to be in the base-16
; prefix then the "0x" will be applied.
;
;	void _rs232.send_value(uint32_t value, char base)
;
_rs232.send_value:
	.prologue:
		push ebp
		mov ebp, esp
	.clear_buffer:
		mov edi, .buffer
		xor al, al
		mov ecx, _BUFFER_LEN 			; The length of the internal buffer
		rep stosb						; Zero out the contents of the buffer
	.is_prefix_required:
		mov ebx, [ebp + 12]				; Fetch the value 'base'
		cmp ebx, 2						; Is it Base-2?
		je .use_bin_prefix
		cmp ebx, 8						; Is it Base-8?
		je .use_oct_prefix
		cmp ebx, 16						; Is it Base-16?
		je .use_hex_prefix
		jmp .parse_value
	.use_bin_prefix:
		push .bin_prefix				; Arg #0: 'str'
		call _rs232.send_bytes 			; Write out the binary prefix.
		add esp, 4
		jmp .parse_value
	.use_oct_prefix:
		push .bin_prefix				; Arg #0: 'str'
		call _rs232.send_bytes			; Write out the octal prefix
		add esp, 4
		jmp .parse_value
	.use_hex_prefix:
		push .hex_prefix				; Arg #0: 'str'
		call _rs232.send_bytes			; Write out the hexadecimal prefix
		add esp, 4
		mov edi, .buffer				; For hex, we need to write 0's for
		mov al, '0'						; padding.
		mov ecx, _BUFFER_LEN
		rep stosb
	.parse_value:
		mov eax, [ebp + 8]				; Fetch the value 'value'
		mov edi, .buffer + _BUFFER_LEN 	; Put EDI in the correct location
	.next_digit:
		dec edi							; Move to the next character in buffer
		mov ecx, [ebp + 12]				; Fetch the value 'base'
		xor edx, edx					; Make sure EDX is clear
		idiv ecx						; Divide by the base
		movzx ebx, dl					; The digit in this location
		mov esi, .digit_lookup
		mov dl, byte[esi + ebx]			; Look up the "alpha" char for the digit
		mov byte[edi], dl				; Write the character to the buffer
		test eax, eax					; Have we reached zero yet?
		jnz .next_digit
		cmp ecx, 16
		jne .display_value
		mov edi, .buffer + _BUFFER_LEN - 8
	.display_value:
		push edi						; Write the buffer to the serial port
		call _rs232.send_bytes
		add esp, 4
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.buffer:
		times _BUFFER_LEN + 1 db 0
	.digit_lookup:
		db "0123456789ABCDEF"
	.bin_prefix:
		db "b", 0
	.oct_prefix:
		db "O", 0
	.hex_prefix:
		db "0x", 0
