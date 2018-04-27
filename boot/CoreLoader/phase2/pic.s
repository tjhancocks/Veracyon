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

	[bits	32]

;;
;; Setup and configure the Programmable Interrupt Controller. This will ensure
;; that the higer 8 IRQs can be handled correctly.
;;
;;  void prepare_pic(void)
;;
_prepare_pic:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov al, 0x11
		out 0x20, al
		out 0xA0, al
		mov al, 0x20
		out 0x21, al
		mov al, 0x28
		out 0xA1, al
		mov al, 0x04
		out 0x21, al
		mov al, 0x08
		out 0xA1, al
		mov al, 0x01
		out 0x21, al
		out 0xA1, al
		xor eax, eax
		out 0x21, al
		out 0xA1, al
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
