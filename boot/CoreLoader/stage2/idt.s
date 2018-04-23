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

; Interrupt Descriptor Table internal constants
_IDT_GATE_COUNT	equ 128

; Setup the Interrupt Descriptor Table (IDT) and install it into the CPU.
;
;	void _idt.init()
;
_idt.init:
	.prologue:
		push ebp
		mov ebp, esp
	.clear_table:
		mov edi, IDT_ADDR				; The location of the IDT in memory.
		mov ecx, 8 * _IDT_GATE_COUNT	; 256 Entries, multiplied by 8 bytes
		shr ecx, 2						; Divide by 4 to get the DWORD count
		xor eax, eax
		rep stosd						; Fill the table with NUL bytes.
	.configure_idtptr:
		mov edi, IDTPTR_ADDR			; Get the address for the IDT Pointer
		mov dword[edi], _IDT_GATE_COUNT * 8; We want 256 potential entries in it
		mov dword[edi+2], IDT_ADDR		; The location of the IDT in memory.
	.install:
		mov eax, IDTPTR_ADDR			; Install the IDT. Must still not enable
		lidt [eax]						; interrupts yet!
	.epilogue:
		xchg bx, bx
		mov esp, ebp
		pop ebp
		ret