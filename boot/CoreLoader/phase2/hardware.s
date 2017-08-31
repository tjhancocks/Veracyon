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
;; Install each of the CPU Hardware Interrupts (IRQs) into the Interrupt
;; Descriptor Table, at offsets 0x20 - 0x2F.
;;
;;	void _install_hardware_interrupts(void)
;;
_install_hardware_interrupts:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov edi, 0x11000 + (0x20 * 8)	; Location of the first entry in IDT.
		mov edx, _hdw_int_0				; Pointer to the first handler.
		mov ecx, 0x10					; The exception handler count (16).
	.L0:
		mov ebx, edx
		mov word[edi], bx				; Write the lower word of offset 
		shr bx, 16						; Fetch upper word of offset
		mov word[edi+6], bx				; Write the upper word of offset
		mov word[edi+2], 0x0008			; Write the selector
		mov byte[edi+4], 0x00			; Write zero. This is ALWAYS zero.
		mov byte[edi+5], 0x8E			; Set appropriate flags.
	.next_entry:
		add edi, 0x08					; Move to the next entry of IDT
		add edx, _hdw_int_1 - _hdw_int_0; Move to the next handler.
		loop .L0
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; CPU Hardware Interrupt Handler (IRQ). This will route handled IRQs,
;; acknowledge IRQs and correctly handle spurious interrupts. This is a naked
;; function and should not be called directly.
;;
_handle_hardware_interrupt:
		pushad
	.finish:
		popad
		add esp, 4
		iret

;;
;; The following section contains each of the CPU Hardware Interrupt Handlers.
;;
_hdw_int_0:
        cli
        push byte 0
        jmp _handle_hardware_interrupt
_hdw_int_1:
        cli
        push byte 1
        jmp _handle_hardware_interrupt
_hdw_int_2:
        cli
        push byte 2
        jmp _handle_hardware_interrupt
_hdw_int_3:
        cli
        push byte 3
        jmp _handle_hardware_interrupt
_hdw_int_4:
        cli
        push byte 4
        jmp _handle_hardware_interrupt
_hdw_int_5:
        cli
        push byte 5
        jmp _handle_hardware_interrupt
_hdw_int_6:
        cli
        push byte 6
        jmp _handle_hardware_interrupt
_hdw_int_7:
        cli
        push byte 7
        jmp _handle_hardware_interrupt
_hdw_int_8:
        cli
        push byte 8
        jmp _handle_hardware_interrupt
_hdw_int_9:
        cli
        push byte 9
        jmp _handle_hardware_interrupt
_hdw_int_10:
        cli
        push byte 10
        jmp _handle_hardware_interrupt
_hdw_int_11:
        cli
        push byte 11
        jmp _handle_hardware_interrupt
_hdw_int_12:
        cli
        push byte 12
        jmp _handle_hardware_interrupt
_hdw_int_13:
        cli
        push byte 13
        jmp _handle_hardware_interrupt
_hdw_int_14:
        cli
        push byte 14
        jmp _handle_hardware_interrupt
_hdw_int_15:
        cli
        push byte 15
        jmp _handle_hardware_interrupt