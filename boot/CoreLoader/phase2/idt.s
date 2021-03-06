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
;; Prepare the region of memory (0x11000 - 0x1107FF) to be the interrupt
;; descriptor table. For this we just need to ensure it is zero'd.
;; Interrupts _must_ remain off after this!
;;
;;	void prepare_idt(void)
;;
_prepare_idt:
	.prologue:
		push ebp
		mov ebp, esp
	.clear_memory:
		mov edi, IDT_BASE
		mov ecx, 0x200
		xor eax, eax
		rep stosd
	.prepare_idt_pointer:
		mov edi, IDT_PTR
		mov ax, 0x7ff
		stosw
		mov eax, IDT_BASE
		stosd
	.load_idt:
		mov eax, IDT_PTR
		lidt [eax]
	.update_configuration:
		mov edi, BOOT_CONFIG
		mov dword[edi + BootConf.interrupt_stubs], INT_HANDLERS
		mov word[edi + BootConf.idt_size], 0x200
		mov dword[edi + BootConf.idt_base], IDT_BASE
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
