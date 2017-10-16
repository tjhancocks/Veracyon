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
;; Install each of the CPU Exceptions (ISRs) into the Interrupt Descriptor
;; Table, at offsets 0x00 - 0x1F.
;;
;;	void install_cpu_exceptions(void)
;;
_install_cpu_exceptions:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov edi, IDT_BASE				; Location of the IDT
		mov edx, _cpu_exp_0				; Pointer to the first handler.
		mov ecx, 0x20					; The exception handler count (32).
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
		add edx, _cpu_exp_1 - _cpu_exp_0; Move to the next handler.
		loop .L0
	.prepare_default_panic_handler:
        mov edi, BOOT_CONFIG
        mov dword[edi + BootConf.panic_handler], PANIC_FN_PTR
		mov edi, PANIC_FN_PTR			; Location of the panic handler pointer.
		xor eax, eax
		stosd 							; Set the pointer to NULL.
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; CPU Exception Handler. This is responsible for determining the appropriate
;; action to take in the event of an exception/trap being raised. If no custom
;; handler is present, or the custom handler returns a bad result then the 
;; system will be halted.
;; WARNING: This is a naked function and it should not be called directly.
;;
_handle_exception:
		pushad
	.unsafe_finish:
		call _panic 					; We're in an unsafe state! Panic!
	.safe_finish:
		popad
		add esp, 8
		iret

;;
;; This is the default panic handler. It is responsible for reporting that an
;; error has occured and describing the error to the user, both visually and
;; over the COM1 Serial Port. It should then hang then the system and prevent
;; control returning the previous point of execution.
;;
;;	void panic(registers_t reg)
;;
_panic:
	.find_panic_handler:
		mov esi, PANIC_FN_PTR			; Location of panic handler pointer.
		mov eax, [esi]					; Load the panic handler pointer.
		or eax, eax						; Check to see if it is NULL.
		jz .report						; If NULL, proceed to basic reporter.
		jmp eax							; It's not NULL so jump to the handler.
	.report:
		push .message
		call _send_serial_bytes
	.hang:
		cli
		hlt
		jmp .hang
	.message:
		db "System Panic!", 0xA,
		db "To prevent damage or data corruption, the system is halting now.",
		db 0x0

;;
;; The following section contains each of the CPU Exception Handlers.
;;
_cpu_exp_0:
    	cli
    	nop
    	nop
    	push byte 0
    	jmp _handle_exception
_cpu_exp_1:
    	cli
    	nop
    	nop
    	push byte 1
    	jmp _handle_exception
_cpu_exp_2:
    	cli
    	nop
    	nop
    	push byte 2
    	jmp _handle_exception
_cpu_exp_3:
    	cli
    	nop
    	nop
    	push byte 3
    	jmp _handle_exception
_cpu_exp_4:
    	cli
    	nop
    	nop
    	push byte 4
    	jmp _handle_exception
_cpu_exp_5:
    	cli
    	nop
    	nop
    	push byte 5
    	jmp _handle_exception
_cpu_exp_6:
    	cli
    	nop
    	nop
    	push byte 6
    	jmp _handle_exception
_cpu_exp_7:
    	cli
    	nop
    	nop
    	push byte 7
    	jmp _handle_exception
_cpu_exp_8:
    	cli
    	push byte 0
    	push byte 8
    	jmp _handle_exception
_cpu_exp_9:
    	cli
    	nop
    	nop
    	push byte 9
    	jmp _handle_exception
_cpu_exp_10:
    	cli
    	push byte 0
    	push byte 10
    	jmp _handle_exception
_cpu_exp_11:
    	cli
    	push byte 0
    	push byte 11
    	jmp _handle_exception
_cpu_exp_12:
    	cli
    	push byte 0
    	push byte 12
    	jmp _handle_exception
_cpu_exp_13:
    	cli
    	push byte 0
    	push byte 13
    	jmp _handle_exception
_cpu_exp_14:
    	cli
    	push byte 0
    	push byte 14
    	jmp _handle_exception
_cpu_exp_15:
    	cli
    	nop
    	nop
    	push byte 15
    	jmp _handle_exception
_cpu_exp_16:
    	cli
    	nop
    	nop
    	push byte 16
    	jmp _handle_exception
_cpu_exp_17:
    	cli
    	push byte 0
    	push byte 17
    	jmp _handle_exception
_cpu_exp_18:
    	cli
    	nop
    	nop
    	push byte 18
    	jmp _handle_exception
_cpu_exp_19:
    	cli
    	nop
    	nop
    	push byte 19
    	jmp _handle_exception
_cpu_exp_20:
    	cli
    	nop
    	nop
    	push byte 20
    	jmp _handle_exception
_cpu_exp_21:
    	cli
    	nop
    	nop
    	push byte 21
    	jmp _handle_exception
_cpu_exp_22:
    	cli
    	nop
    	nop
    	push byte 22
    	jmp _handle_exception
_cpu_exp_23:
    	cli
    	nop
    	nop
    	push byte 23
    	jmp _handle_exception
_cpu_exp_24:
    	cli
    	nop
    	nop
    	push byte 24
    	jmp _handle_exception
_cpu_exp_26:
    	cli
    	nop
    	nop
    	push byte 26
    	jmp _handle_exception
_cpu_exp_27:
    	cli
    	nop
    	nop
    	push byte 27
    	jmp _handle_exception
_cpu_exp_28:
    	cli
    	nop
    	nop
    	push byte 28
    	jmp _handle_exception
_cpu_exp_29:
    	cli
    	nop
    	nop
    	push byte 29
    	jmp _handle_exception
_cpu_exp_30:
    	cli
    	nop
    	nop
    	push byte 30
    	jmp _handle_exception
_cpu_exp_31:
    	cli
    	nop
    	nop
    	push byte 31
    	jmp _handle_exception
