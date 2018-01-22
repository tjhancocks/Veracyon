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


	global mmx_memcpy

;;
;; Optimised memcpy function.
;;
;; 	void *memcpy(void *restrict dst, void *restrict src, uint32_t n)
;;
mmx_memcpy:
	.prologue:
		push ebp
		mov ebp, esp
	.check_mmx:
		mov eax, 1
		cpuid
		test eax, 1 << 23					; Check for MMX capabilities
		jz .no_mmx
	.mmx:
		mov ecx, [esp + 16]
		cmp ecx, 0x40
		jl .no_mmx

		mov esi, [esp + 12]
		mov edi, [esp + 8]

		; TODO: Fix to work correct on unaligned data.
		movq mm1, [esi + 0x00]
		movq mm2, [esi + 0x08]
		movq mm3, [esi + 0x10]
		movq mm4, [esi + 0x18]
		movq mm5, [esi + 0x20]
		movq mm6, [esi + 0x28]
		movq mm7, [esi + 0x30]
		movq mm0, [esi + 0x38]
		movntq [edi + 0x00], mm1
		movntq [edi + 0x08], mm2
		movntq [edi + 0x10], mm3
		movntq [edi + 0x18], mm4
		movntq [edi + 0x20], mm5
		movntq [edi + 0x28], mm6
		movntq [edi + 0x30], mm7
		movntq [edi + 0x38], mm0

		add esi, 0x40
		add edi, 0x40
		dec ecx
		jnz .mmx
	.mmx_done:
		jmp .epilogue
	.no_mmx:
		; We do not have any SIMD capabilities so we're going to default to a
		; standard memset function.
		mov ecx, [esp + 16]
		shr ecx, 2
		mov esi, [esp + 12]
		mov edi, [esp + 8]
		rep movsd
	.epilogue:
		mov eax, [esp + 8]					; Return dst to the caller
		mov esp, ebp
		pop ebp
		ret
