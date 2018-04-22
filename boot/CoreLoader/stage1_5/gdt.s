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

	BITS 	16

; 
; Initialise the Global Descriptor Table. This will setup 3 base segments, and
; assume a flat memory model. It will be possible for the kernel to adopt a
; different memory model if it so chooses.
;
gdt.init:
	.prologue:
		push bp
		mov bp, sp
	.prepare_segments:
		mov ax, CPU_SEG
		mov es, ax
	.null_gate:
		push 0
		push 0
		push dword 0x00000
		push dword 0x00000000
		push 0
		call gdt.set_gate
		add sp, 14
	.kernel_code:
		push 0x0C
		push 0x99
		push dword 0xFFFFF
		push dword 0x00000000
		push 1
		call gdt.set_gate
		add sp, 14
	.kernel_data:
		push 0x0C
		push 0x93
		push dword 0xFFFFF
		push dword 0x00000000
		push 2
		call gdt.set_gate
		add sp, 14
	.setup_pointer:
		mov ax, BC_SEG
		mov es, ax
		mov di, BC_OFFSET
		mov ax, (8 * 3) - 1				; Size of the GDT in bytes, minus 1
		mov word[es:di + BootConf.gdt_size], ax
		mov eax, GDT_ADDR
		mov dword[es:di + BootConf.gdt_base], eax
	.epilogue:
		mov sp, bp
		pop bp
		ret

;
; Add an entry to the GDT. This _may_ clobber registers.
;
gdt.set_gate:
	.prologue:
		push bp
		mov bp, sp
		push es
	.adjust_segments:
		mov ax, CPU_SEG					; CPU data structure segment is required
		mov es, ax
		mov di, GDT_OFFSET				; The base offset of the GDT in CPU_SEG
	.calculations:
		mov ax, 8						; The size of a single gate
		mul word[bp + 4]				; Multiplied by the gate number
		add di, ax						; DI + AX
	.write_gate:
		mov eax, dword[bp + 10]			; Fetch the limit of the segment
		mov [es:di], ax					; Save the low limit
		shr eax, 16						; eax >>= 16
		and al, 0x0F					; Keep only the lower 4 bits
		mov bx, [bp + 16]				; Fetch the flags
		shl bx, 4						; bx <<= 4
		and bl, 0xF0					; Keep only the upper 4 bits
		or al, bl						; al |= bl
		mov [es:di + 6], al				; Save the flags|high_limit
		mov eax, dword[bp + 6]			; Fetch the offset of the segment
		mov [es:di + 2],  ax			; Save the lower base
		shr eax, 16						; eax >>= 16
		mov byte[es:di + 4], al			; Save the next byte of the offset
		mov byte[es:di + 7], ah			; Save the upper byte of the offset
		mov ax, [bp + 14]				; Fetch the access value
		mov byte[es:di + 5], al			; Save the access byte of the segment. 
	.epilogue:
		pop es
		mov sp, bp
		pop bp
		ret