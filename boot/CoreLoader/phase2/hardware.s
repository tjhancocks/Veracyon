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
;;	void install_hardware_interrupts(void)
;;
_install_hardware_interrupts:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov edi, IDT_BASE + (0x20 * 8)	; Location of the first entry in IDT.
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
		movzx ebx, byte[esp + 32]
		mov eax, ebx
	.handle_spurious_irq7:
		cmp bl, 0x07
		jne .handle_irq
		mov al, 0x08
		out 0x20, al
		in al, 0x20
		and al, 0x80
		jnz .handle_irq
		jmp .finish
	.handle_irq:
		cmp bl, 0x01
		je .handle_keyboard_irq
		jmp .find_irq_handler
	.handle_keyboard_irq:
		in al, 0x64
		and al, 0x02
		jz .finish_keyboard_irq
		nop
		jmp .handle_keyboard_irq
	.finish_keyboard_irq:
		in al, 0x60
		jmp .acknowledge_irq
	.find_irq_handler:
		movzx eax, byte[esp + 32]
		mov ebx, 4
		mul ebx
		add eax, (0x20 * 4)
		add eax, INT_HANDLERS
		mov esi, eax
		mov eax, [esi]
	.check_handler:
		or eax, eax
		jz .acknowledge_irq
		call eax
	.acknowledge_irq:
		movzx ebx, byte[esp + 32]
		cmp bl, 0x80
		jl .acknowledge_master_irq
		mov al, 0x20
		out 0xa0, al
	.acknowledge_master_irq:
		mov al, 0x20
		out 0x20, al
	.finish:
		popad
		add esp, 4
		iret

;;
;; Register a hardware interrupt handler for the specified IRQ.
;;
;;	void register_hardware_interrupt_handler(int irq, void *fn)
;;
_register_hardware_interrupt_handler:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		cli
	.calculate_offset:
		mov edi, INT_HANDLERS			; Base of the interrupt handlers.
		mov eax, [ebp + 8]				; Fetch the IRQ number
		mov ebx, [ebp + 12]				; Fetch the function pointer.
		mov [edi + (eax * 4)], ebx		; Save the function pointer to the table
	.epilogue:
		sti
		mov esp, ebp
		pop ebp
		ret

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