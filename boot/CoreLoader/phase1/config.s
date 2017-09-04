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
;; Produce the default boot configuration structure in memory. This may be
;; overridden by user specified information later.
;;
prepare_boot_configuration_defaults:
	.prologue:
		push bp
		mov bp, sp
		pusha
		push es
	.clear:
		mov di, 0xFE00
		mov cx, 0x100
		xor ax, ax
		rep stosb
	.fill:
		mov di, 0xFE00
		mov byte[di], 0x00				; VGA Text Mode
		mov byte[di + 0x01], 'v'		; Kernel Name
		mov byte[di + 0x02], 'k'
		mov byte[di + 0x03], 'e'
		mov byte[di + 0x04], 'r'
		mov byte[di + 0x05], 'n'
		mov byte[di + 0x06], 'e'
		mov byte[di + 0x07], 'l'
		mov dword[di + 0x20], 0x100000	; Kernel Address (1MiB)
		mov word[di + 0x24], 80			; Resolution Width
		mov word[di + 0x26], 25			; Resolution Height
		mov word[di + 0x28], 4			; Resolution Depth
		mov dword[di + 0x2A], 0xB8000	; Linear Frame Buffer / VGA Buffer
	.epilogue:
		pop es
		popa
		mov sp, bp
		pop bp
		ret
		