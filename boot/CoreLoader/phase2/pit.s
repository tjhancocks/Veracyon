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

;;
;; Programmable Interrupt Timer configuration information.
;;
	align 32
$PIT:
	.phase		dd 1000
	.subticks	dd 0
	.ticks		dd 0
	.installed	dd 0
	.counter	dd 0

;;
;; Initialise the Programmable Interrupt Timer, and configure it into a known
;; state.
;;
;;	void configure_pit(void)
;;
_configure_pit:
	.prologue:
		push ebp
		mov ebp, esp
	.set_phase:
		xor edx, edx					; Ensure EDX is zero
		mov eax, 1193180				; Magic number for PIT.
		mov ebx, dword[$PIT.phase]		; Fetch the desired phase (Hz)
		div ebx 						; Calculate the value to set
		mov ebx, eax
		mov al, 0x36
		out 0x43, al
		mov eax, ebx
		out 0x40, al
		shr eax, 8
		out 0x40, al
	.install_pit_interrupt_handler:
		push _pit_interrupt_handler
		push 0
		call _register_hardware_interrupt_handler
		add esp, 8
	.epilogue:
		mov dword[$PIT.installed], 1
		mov esp, ebp
		pop ebp
		ret

;;
;; The Programmable Interrupt Timer handler function. This handles the 
;; generated interrupts that the CPU receives.
;;
;; 	void pit_interrupt_handler(void)
;;
_pit_interrupt_handler:
	.prologue:
		push ebp
		mov ebp, esp
	.subticks:
		mov eax, [$PIT.subticks]
		inc eax
		mov [$PIT.subticks], eax
		mov ebx, [$PIT.phase]
		cmp eax, ebx
		jl .counter
	.ticks:
		mov dword[$PIT.subticks], 0
		mov eax, [$PIT.ticks]
		inc eax
		mov [$PIT.ticks], eax
	.counter:
		mov eax, [$PIT.counter]
		or eax, eax
		jz .epilogue
		dec eax
		mov [$PIT.counter], eax
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Basic sleep function. This is a blocking function and designed to be simple
;; for the purposes of CoreLoader. It should be used for nothing more than the
;; waiting for a specified number of "subtick" cycles. Each subtick cycle 
;; corresponds to a millisecond.
;;
;;	void sleep(unsigned int count)
;;
_sleep:
	.prologue:
		push ebp
		mov ebp, esp
		xchg bx, bx
		pushad
	.early_exit_test:
		mov eax, [$PIT.installed]
		or eax, eax
		jz .epilogue
		mov eax, [ebp + 8]
		or eax, eax
		jz .epilogue
	.prepare:
		mov dword[$PIT.counter], eax
	.L0:
		hlt
		dec eax
		or eax, eax
		jnz .L0
	.epilogue:
		xchg bx, bx
		popad
		mov esp, ebp
		pop ebp
		ret
