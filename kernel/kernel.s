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

	[bits 	32]

;;
;; Public Kernel Symbols
;;
	global	start
	extern	kmain

;;
;; Kernel starting and entry point.
;;
section .text
start:
	.capture_coreloader_info:
		mov eax, [esp + 4]				; Fetch the BootConfig structure.
	.main:
		mov esp, stack + 0x4000
		push eax
		call kmain
		cli
		hlt
		jmp $

;;
;; Kernel stack. This is a reserved region of known memory that can be used by
;; the kernel for the default stack. This is 65KiB in size.
;;
section	.bss
align	4
stack:	
	resb	0x10000

;;
;; We need some space to use as working memory so that the kernel is able to to
;; initially do its job.
;;
working_memory:
	resb 	0x800000