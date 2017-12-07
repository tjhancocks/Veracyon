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
	global sse_memcpy

;;
;; Perform an SSE memory copy if available.
;;
;;	int sse_memcpy(void *dst, void *src, uint32_t count)
;;
sse_memcpy:
	.prologue:
		push ebp
		mov ebp, esp
	.sse_test:
		mov eax, 0x1
		cpuid
		test edx, 1 << 25
		jnz .prepare
		xor eax, eax
		jmp .epilogue
	.prepare:
		mov esi, [ebp + 12]
		mov edi, [ebp + 8]
		mov ecx, [ebp + 16]
		shr ecx, 4
	.copy_loop:
		movdqa xmm0, [esi]
		movdqa xmm1, [esi + 0x10]
		movdqa xmm2, [esi + 0x20]
		movdqa xmm3, [esi + 0x30]
		movdqa xmm4, [esi + 0x40]
		movdqa xmm5, [esi + 0x50]
		movdqa xmm6, [esi + 0x60]
		movdqa xmm7, [esi + 0x70]
		movdqa [edi], xmm0
		movdqa [edi + 0x10], xmm1
		movdqa [edi + 0x20], xmm2
		movdqa [edi + 0x30], xmm3
		movdqa [edi + 0x40], xmm4
		movdqa [edi + 0x50], xmm5
		movdqa [edi + 0x60], xmm6
		movdqa [edi + 0x70], xmm7
		add esi, 0x80
		add edi, 0x80
		loop .copy_loop
		mov eax, 1
	.epilogue:
		mov esp, ebp
		pop ebp
		ret