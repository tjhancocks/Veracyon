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

; Interrupt Descriptor Table internal constants
_IDT_GATE_COUNT	equ 128

; Setup the Interrupt Descriptor Table (IDT) and install it into the CPU.
;
;	void _idt.init(void)
;
_idt.init:
	.prologue:
		push ebp
		mov ebp, esp
	.clear_table:
		mov edi, IDT_ADDR				; The location of the IDT in memory.
		mov ecx, 8 * _IDT_GATE_COUNT	; 256 Entries, multiplied by 8 bytes
		shr ecx, 2						; Divide by 4 to get the DWORD count
		xor eax, eax
		rep stosd						; Fill the table with NUL bytes.
	.configure_idtptr:
		mov edi, IDTPTR_ADDR			; Get the address for the IDT Pointer
		mov dword[edi], _IDT_GATE_COUNT * 8; We want 256 potential entries in it
		mov dword[edi+2], IDT_ADDR		; The location of the IDT in memory.
	.install:
		mov eax, IDTPTR_ADDR			; Install the IDT. Must still not enable
		lidt [eax]						; interrupts yet!
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

; Install all CPU Exceptions into the Interrupt Descriptor Table.
;
;	void _idt.isrs.init(void)
;
_idt.isrs.init:
	.proglogue:
		push ebp
		mov ebp, esp
	.prepare:
		mov edi, IDT_ADDR				; Get the base address of the IDT.
		mov ecx, 0x20					; There are 32 ISRs
		mov edx, _idt.isr0				; A pointer to the first ISR handler.
	.@@:
		mov ebx, edx					; Copy EDX into EBX
		mov word[edi], bx				; Write the lo-word of the handler base
		shr ebx, 16						; EBX >> 16, shift the upper half down
		mov word[edi + 6], bx			; Write the hi-word of the handler base
		mov word[edi + 2], 0x08			; The CS selector value to use
		mov byte[edi + 4], 0x00			; The zero value
		mov byte[edi + 5], 0x8E			; Flags
	.next:
		add edi, 8						; Move to the next gate...
		add edx, _idt.isr1 - _idt.isr0	; ...and the next handler pointer.
		loop .@@
	.epilogue:
		sti
		int 0x8
		xchg bx, bx
		mov esp, ebp
		pop ebp
		ret

; The following block contains all subroutines required for handling ISRs,
; as either panics or silent exceptions.
_idt:
	;
	;	void _idt.panic_handler(uint32_t interrupt_number)
	;
	.panic_handler:
		; We currently assume that the panic _MUST_ happen. Write to the debug
		; serial link and halt.
		push .dbg_panic_msg
		call _rs232.send_bytes
		push .dbg_interrupt_str
		call _rs232.send_bytes
		add esp, 8
		mov eax, [esp + 4]				; Fetch arg#0, 'interrupt_number'
		push dword 16
		push eax
		call _rs232.send_value
		push dword 0xD
		call _rs232.send_byte
		add esp, 12
		hlt
	.dbg_panic_msg:
		db "CoreLoader encountered a fatal error and must halt!", 0xD, 0x0
	.dbg_interrupt_str:
		db "Received CPU Exception ", 0x0
	.isr_handler:
		pushad 							; Save all general purpose registers
		push ds							; Save the segment registers so they
		push es							; can be restored at the end of the
		push fs							; interrupt.
		push gs							;
		mov ax, 0x10					; We need to ensure we're in the
		mov ds, ax						; supervisor data segment. Fix it.
		mov es, ax
		mov fs, ax
		mov gs, ax
		cld
		mov eax, [esp + INTFrame.int]	; Fetch the interrupt number from stack
		push eax						; Arg#0: 'interrupt_number'
		call .panic_handler				; Call out to the panic handler
		add esp, 4
		pop gs							; Restore the segment registers.
		pop fs
		pop es
		pop ds
		popad 							; Restore all general purpose registers
		add esp, 8						; Clean up the stack
		iretd 							; Return from the interrupt safely.
	.isr0:								; #0: Divide-by-zero
		cli
		push byte 0						; Error Code
		push byte 0						; Interrupt Number
		jmp .isr_handler
	.isr1:								; #1: Debug
		cli
		push byte 0						; Error Code
		push byte 1						; Interrupt Number
		jmp .isr_handler
	.isr2:								; #2: Non-maskable Interrupt
		cli
		push byte 0						; Error Code
		push byte 2						; Interrupt Number
		jmp .isr_handler
	.isr3:								; #3: Breakpoint
		cli
		push byte 0						; Error Code
		push byte 3						; Interrupt Number
		jmp .isr_handler
	.isr4:								; #4: Overflow
		cli
		push byte 0						; Error Code
		push byte 4						; Interrupt Number
		jmp .isr_handler
	.isr5:								; #5: Bound Range Exceeded
		cli
		push byte 0						; Error Code
		push byte 5						; Interrupt Number
		jmp .isr_handler
	.isr6:								; #6: Invalid Opcode
		cli
		push byte 0						; Error Code
		push byte 6						; Interrupt Number
		jmp .isr_handler
	.isr7:								; #7: Device Not Available
		cli
		push byte 0						; Error Code
		push byte 7						; Interrupt Number
		jmp .isr_handler
	.isr8:								; #8: Double Fault
		cli
		nop
		nop
		push byte 8						; Interrupt Number
		jmp .isr_handler
	.isr9:								; #9: Coprocessor Segment Overrun
		cli
		push byte 0						; Error Code
		push byte 9						; Interrupt Number
		jmp .isr_handler
	.isr10:								; #10: Invalid TSS					
		cli
		nop
		nop
		push byte 10					; Interrupt Number
		jmp .isr_handler
	.isr11:								; #11: Segment Not Present
		cli
		nop
		nop
		push byte 11					; Interrupt Number
		jmp .isr_handler
	.isr12:								; #12: Stack-Segment Fault
		cli
		nop
		nop
		push byte 12					; Interrupt Number
		jmp .isr_handler
	.isr13:								; #13: General Protection Fault
		cli
		nop
		nop
		push byte 13					; Interrupt Number
		jmp .isr_handler
	.isr14:								; #14: Page Fault
		cli
		nop
		nop
		push byte 14					; Interrupt Number
		jmp .isr_handler
	.isr15:								; #15: Reserved
		cli
		push byte 0						; Error Code
		push byte 15					; Interrupt Number
		jmp .isr_handler
	.isr16:								; #16 x87 Floating-Point Exception
		cli
		push byte 0						; Error Code
		push byte 16					; Interrupt Number
		jmp .isr_handler
	.isr17:								; #17: Alignment Check
		cli
		nop
		nop
		push byte 17					; Interrupt Number
		jmp .isr_handler
	.isr18:								; #18: Machine Check
		cli
		push byte 0						; Error Code
		push byte 18					; Interrupt Number
		jmp .isr_handler
	.isr19:								; #19: SIMD Floating-Point Exception
		cli
		push byte 0						; Error Code
		push byte 19					; Interrupt Number
		jmp .isr_handler
	.isr20:								; #20: Virtualization Exception
		cli
		push byte 0						; Error Code
		push byte 20					; Interrupt Number
		jmp .isr_handler
	.isr21:								; #21: Reserved
		cli
		push byte 0						; Error Code
		push byte 21					; Interrupt Number
		jmp .isr_handler
	.isr22:								; #22: Reserved
		cli
		push byte 0						; Error Code
		push byte 22					; Interrupt Number
		jmp .isr_handler
	.isr23:								; #23: Reserved
		cli
		push byte 0						; Error Code
		push byte 23					; Interrupt Number
		jmp .isr_handler
	.isr24:								; #24: Reserved
		cli
		push byte 0						; Error Code
		push byte 24					; Interrupt Number
		jmp .isr_handler
	.isr25:								; #25: Reserved
		cli
		push byte 0						; Error Code
		push byte 25					; Interrupt Number
		jmp .isr_handler
	.isr26:								; #26: Reserved
		cli
		push byte 0						; Error Code
		push byte 26					; Interrupt Number
		jmp .isr_handler
	.isr27:								; #27: Reserved
		cli
		push byte 0						; Error Code
		push byte 27					; Interrupt Number
		jmp .isr_handler
	.isr28:								; #28: Reserved
		cli
		push byte 0						; Error Code
		push byte 28					; Interrupt Number
		jmp .isr_handler
	.isr29:								; #29: Reserved
		cli
		push byte 0						; Error Code
		push byte 29					; Interrupt Number
		jmp .isr_handler
	.isr30:								; #30: Security Exception
		cli
		push byte 0						; Error Code
		push byte 30					; Interrupt Number
		jmp .isr_handler
	.isr31:								; #31: Reserved
		cli
		push byte 0						; Error Code
		push byte 31					; Interrupt Number
		jmp .isr_handler