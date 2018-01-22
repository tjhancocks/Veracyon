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

	[bits 	32]


	global memsetd

;;
;; Optimised memsetd function. This operates on 32-bit values (DWORD).
;;
;; 	void *memsetd(void *restrict dst, uint32_t value, uint32_t n)
;;
memsetd:
	.prologue:
		push ebp
		mov ebp, esp
	.check_mmx:
		mov eax, 1
		cpuid
		test eax, 1 << 23					; Check for MMX capabilities
		jz .no_mmx
	.no_mmx:
		; We do not have any SIMD capabilities so we're going to default to a
		; standard memset function.
		mov ecx, [esp + 16]
		mov eax, [esp + 12]
		mov edi, [esp + 8]
		rep stosd
	.epilogue:
		mov eax, [esp + 8]					; Return dst to the caller
		mov esp, ebp
		pop ebp
		ret
