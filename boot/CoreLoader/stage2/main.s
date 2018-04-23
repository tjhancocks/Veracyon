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

	BITS 	32

; CoreLoader Stage2 entry point. We should be entering 32bit protected mode at
; this location.
CoreLoader.Stage2.main:
	.start:
		push CoreLoader.Stage2.Strings.pmode_text
		call _rs232.send_bytes
		add esp, 4
	.interrupt_handlers:
		; Setup the Interrupt Descriptor Table (IDT) and install the relevant
		; handlers for Interrupt Service Routines (ISRs) and Interrupt Requests
		; (IRQs).
		call _idt.init
		call _idt.isrs.init
		call _idt.irq.init
		sti 							; Enable maskable interrupts
		in al, 0x70						; Also enable Non-maskable interrupts
		and al, 0x7F
		out 0x70, al
		push CoreLoader.Stage2.Strings.interrupts_enabled
		call _rs232.send_bytes
		add esp, 4
	.catch:
		cli
		hlt

; Strings used in Stage2 of CoreLoader.
CoreLoader.Stage2.Strings:
	.pmode_text:	
		db "CoreLoader is now running in 32bit Protected Mode!", 0xD, 0x0
	.interrupts_enabled:
		db "Interrupts are now enabled. "
		db "ISRs & IRQs are being handled.", 0xD, 0x0

; Include all supporting source files and objects.
CoreLoader.Stage2.Supporting:
	%include "CoreLoader/Stage2/rs232.s"
	%include "CoreLoader/Stage2/idt.s"