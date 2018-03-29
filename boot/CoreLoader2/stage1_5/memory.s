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

; Queries the machine's BIOS using the E820 function to determine the memory
; map used by the BIOS. This memory map is used to ensure correct use of memory
; by CoreLoader, and will ultimately be forwarded on to the Kernel.
memory.detect:
	.prologue:
		pusha
		push bp
		mov bp, sp
		push bp
		push MM_SEG			; Make sure the extra segment is for the memory map.
		pop es
		xor edi, edi
	.e820:
		xor ebx, ebx
		xor bp, bp
		mov edx, MMAP_TOKEN	; The token is used to check mmap entry validity
		mov ax, 0xE820
		mov [es:di + 20], dword 1
		mov ecx, 24
		int 0x15
		mov edx, MMAP_TOKEN
		cmp eax, edx
		jne .failed
		jmp .jmpin
	.e820lp:
		mov eax, 0xE820
		mov [es:di + 20], dword 1
		mov ecx, 24
		int 0x15
		jc .e820f
		mov edx, MMAP_TOKEN
	.jmpin:
		jcxz .skipent
		cmp cl, 20
		jbe .notext
		test byte[es:di + 20], 1
		je .skipent
	.notext:
		mov ecx, [es:di + 8]
		or ecx, [es:di + 12]
		jz .skipent
		inc bp
		add di, 24
	.skipent:
		test ebx, ebx
		jne .e820lp
	.e820f:
		push ds
		push BC_SEG
		pop ds
		mov si, BC_OFFSET
		mov [ds:si + BootConf.mmap_count], bp
		mov dword[ds:si + BootConf.mmap_addr], MM_ADDR
		pop ds
		clc
		jmp .finish
	.failed:
		stc
	.finish:
		pop bp
		mov sp, bp
		pop bp
		call memory.map.display
	.epilogue:
		popa
		ret

; Write the memory map out to the RS232 COM1 port for debugging purposes.
; This will help confirm/validate/verify the integrity of information being
; retrieved by CoreLoader early on the setup and configuration process.
memory.map.display:
	.prologue:
		pusha
	.header:
		mov si, .header_str
		call rs232.send_bytes
	.next_item:

	.footer:

	.epilogue:
		popa
		ret
	.header_str:
		db "System Memory Map:", 0xD
		db " START       END         RESERVED? ", 0xD
		db "===================================", 0xD, 0x0

; Internal memory detection constants
MMAP_TOKEN equ 0x534d4150