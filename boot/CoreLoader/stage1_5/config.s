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

	BITS 	16

; Parses the contents of the file currently loaded into the file buffer.
;	
;	IN: ES:BX => File buffer to parse
;
; Note: This will clobber registers
config.parse_file:
	.prologue:
		push bp
		mov bp, sp
		push es								; [bp - 2] ES: File Segment
		push bx								; [bp - 4] BX: File Offset
		push ds								; [bp - 6] DS - Restore state
	.prepare:
		mov ax, ds
		mov gs, ax							; Move DS into GS
		mov ax, [bp - 2]					; Fetch the value of ES
		mov ds, ax							; DS = ES
		mov si, [bp - 4]					; Fetch the value of BX
	.next_line:
		lodsb 								; Read the next char from file
		mov [bp - 4], si					; Update the file offset
		cmp al, '#'							; Are we looking at a comment?
		je .is_comment
		cmp al, '%'
		je .is_key_value					; If AL is '%' then its a key-value
		cmp al, 0xD							; Ignore a blank line
		je .next_line
		cmp al, 0xA							; Ignore a blank line
		je .next_line
		cmp al, 0
		je .epilogue
		jmp .syntax_error					; Unreconised character found!
	.is_key_value:
		call config.parse_key_value			; Parse out the key...
		jmp .next_line
	.is_comment:
		call config.parse_comment			; Call into the comment parser
		mov [bp - 4], si					; Update the file offset
		jmp .next_line						; Loop around to the next line
	.syntax_error:
		stc
	.epilogue:
		pop ds								; Restore the original DS value
		mov sp, bp
		pop bp
		ret
	.unexpected_char:
		db "Encountered an unexpected character in config", 0xD, 0x0

config.parse_comment:
	.check_char:
		lodsb 								; Fetch the next character...
		cmp al, 0xD							; Is it a new line character?
		je .done							; It is. We're done
		cmp al, 0xA							; Is it a new line character?
		je .done							; It is. We're done
		cmp al, 0x0							; Is it NULL/EoF?
		je .done
		jmp .check_char						; Next character.
	.done:
		ret

config.parse_key_value:
	.prepare:
		push bp
		mov bp, sp
		push si								; [bp - 2] Start of key
		push 0								; [bp - 4] Key length
		push si								; [bp - 6] Start of value
		push 0								; [bp - 8] Value length
		push 0								; [bp - 10] File offset
	.next_key_char:
		lodsb 								; Read the next char in the file.
		cmp al, ':'							; Check for either a ':'
		je .resolve_key
		cmp al, '='							; or '=' to denote end of key.
		je .resolve_key
		jmp .next_key_char					; Move to the next character.
	.resolve_key:
		mov ax, si							; Copy SI into AX
		dec ax								; --AX, ignore the delimiter
		sub ax, word[bp - 2]				; - start of key. This is the length
		mov word[bp - 4], ax				; Save length for future use
		mov word[bp - 6], si				; Update the start of value
	.next_value_char:
		lodsb
		cmp al, 0xD							; Check for a newline...
		je .resolve_value
		cmp al, 0xA							; ...different type of newline
		je .resolve_value
		cmp al, 0x0							; ...or the end of the file
		je .resolve_value
		jmp .next_value_char
	.resolve_value:
		mov ax, si							; Copy SI into AX
		dec ax								; --AX, ignore the end of statement
		sub ax, word[bp - 6]				; - start of value, for the length
		mov word[bp - 8], ax				; Save the length for future use
	.prepare_key_lookup:
		mov word[bp - 10], si				; Save the current file offset.
		movzx cx, byte[cs:.key_count]		; Get the number of keys to iterate
	.next_key:
		push cx								; Push the remaining key count
		mov bx, cx							; Transfer into BX to transform
		dec bx								; We're going to be off by one
		shl bx, 1							; Multiply by 2 for offset in list
		push ds								; Save the current DS value
		mov ax, [bp + 4]					; Recover the original DS value
		mov ds, ax							; and set it.
		mov si, word[bx + .key_list]		; Look up the pointer to the key
		mov di, word[bp - 2]				; Fetch the key from file
		mov cx, word[bp - 4]				; The number of characters to check
		rep cmpsb							; Test them for equality
		jne .keys_not_equal
		mov ax, word[bx + .key_handler]		; Get the handler for this key
		mov si, word[bp - 6]				; Fetch the value from file
		mov cx, word[bp - 8]				; Fetch the value length
		push ds
		pop gs								; Copy DS into GS for the handler
		pop ds								; Restore the parser DS value
		call ax								; Call the handler for the key
		clc
		jmp .done
	.keys_not_equal:
		pop ds								; Restore the parser DS value
		pop cx								; Recover the remaining key count
		loop .next_key						; Go to the next key if CX > 0
	.key_not_found:
		stc
	.done:
		mov si, word[bp - 10]				; Restore the file offset
		mov sp, bp
		pop bp
		ret
	.key_count:
		db 3
	.key_list:
		dw .echo_key, .vid_mode_key, .kernel_key
	.key_handler:
		dw config.echo, config.vid_mode, config.kernel
	.echo_key:
		db "echo", 0x0
	.vid_mode_key:
		db "vid-mode", 0x0
	.kernel_key:
		db "kernel", 0x0

config.echo:
	.next_char:
		lodsb 								; Read the next char from value
		call rs232.send_byte				; Write it to serial debug
		loop .next_char						; Next character...
	.done:
		mov al, 0xD
		call rs232.send_byte				; Send a newline character
		ret

config.vid_mode:
	.prepare:
		push bp
		mov bp, sp
		push si								; [bp - 2] Value pointer
		push cx								; [bp - 4] Value length
		push 0								; [bp - 6] Desired mode number
		movzx cx, byte[.mode_count]			; Get the number of known modes
	.check_mode:
		push ds								; Keep track of original DS
		push gs								; Transfer GS into DS
		pop ds
		push cx								; Save the number of remaining modes
		mov bx, cx							; Copy it into BX
		dec bx								; --BX
		shl bx, 1							; * 2
		mov si, word[bx + .mode_str_lookup] ; Look up mode string in table
		mov cx, word[bp - 4]				; Get the value length
		mov di, word[bp - 2]				; Get the value
		rep cmpsb							; Compare them for equality
		pop ds								; Restore original DS
		je .mode_found						; Did we match? If so jump.
		pop cx								; Restore the remaining mode count
		loop .check_mode
		jmp .use_mode						; We couldn't identify the requested
											; mode. Set the default mode.
	.mode_found:
		shr bx, 1							; BX / 2 to get mode number
		mov word[bp - 6], bx				; Save the desired mode number
	.use_mode:
		push BC_SEG
		pop gs
		xor si, si
		mov ax, word[bp - 6]				; Fetch the desired mode number
		mov byte[gs:si + BootConf.vmode], al; Add it to the BootConf
	.done:
		add sp, 2
		pop cx
		pop si
		mov sp, bp
		pop bp
		ret
	.mode_count:
		db 3
	.mode_str_lookup:
		dw .text_str, .native_str, .graph_str
	.text_str:
		db "text", 0x0
	.native_str:
		db "native", 0x0
	.graph_str:
		db "graph", 0x0

config.kernel:
	.main:
		ret
