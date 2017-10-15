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
;; Detect the amount of lower memory.
;; Note: This will clobber registers.
;;
detect_lower_memory:
	.prologue:
		push bp
		mov bp, sp
	.main:
		clc
		int 0x12
		jc .failed
		nop
		mov si, BOOT_CONFIG
		mov word[si + BootConf.lower_memory], ax
		and eax, 0x0000FFFF
		call send_serial_number
		mov si, strings16.kib
		call send_serial_bytes
		jmp .epilogue
	.failed:
		mov si, strings16.unavailable
		call send_serial_bytes
	.epilogue:
		mov sp, bp
		pop bp
		ret

;;
;; This procedure will build a memory map of the entire system, and in the
;; process determine how much memory is present. The memory map can be used
;; by the kernel at a later time to find out exactly what memory is available
;; and safe to use (memory that hasn't been mapped to hardware, or bad).
;;
read_memory_map:
	.prologue:
		push bp
		mov bp, sp
		push bp
		push 0x7000
		pop es
		xor edi, edi
	.do_e820:
		xor ebx, ebx
		xor bp, bp
		mov edx, 0x534d4150
		mov eax, 0xe820
		mov [es:di + 20], dword 1
		mov ecx, 24
		int 0x15
		jc .failed
		mov edx, 0x534d4150
		cmp eax, edx
		jne .failed
		test ebx, ebx
		je .failed
		jmp .jmpin
	.e820lp:
		mov eax, 0xe820
		mov [es:di + 20], dword 1
		mov ecx, 24
		int 0x15
		jc .e820f
		mov edx, 0x534d4150
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
		mov si, BOOT_CONFIG
		mov [ds:si + BootConf.mmap_count], bp
		mov dword[ds:si + BootConf.mmap], MEMORY_MAP
		clc
		jmp .epilogue
	.failed:
		stc
	.epilogue:
		pop bp
		mov sp, bp
		pop bp
		ret

;;
;; Calculate the total amount of RAM based on the memory map.
;;
calculate_total_memory:
	.prologue:
		push bp
		mov bp, sp
	.main:
		push 0x7000
		pop es
		xor edi, edi
		xor ebx, ebx
		xor edx, edx
		mov esi, BOOT_CONFIG
		movzx ecx, word[ds:si + BootConf.mmap_count]
		jmp .check_entry
	.next_entry:
		sub ecx, 1
		test ecx, ecx
		jz .done
		add edi, 24
	.check_entry:
		mov eax, [es:di + 0]
		cmp eax, ebx
		jl .next_entry
		mov ebx, eax
		mov edx, eax
		add edx, [es:di + 8]
		jmp .next_entry
	.done:
		shr edx, 10
		mov dword[ds:si + BootConf.upper_memory], edx
		shr edx, 10
		mov eax, edx
	.epilogue:
		mov sp, bp
		pop bp
		ret
