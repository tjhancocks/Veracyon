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

; Internal state storage for the configuration parser.
$cfg:
	.orig_gs: 	dw 0 					; Original GS value
	.orig_es:	dw 0					; Original input file segment
	.orig_bx:	dw 0					; Original input file offset
	.orig_ds:	dw 0					; Original DS value
	.file_seg:	dw 0					; Current file segment
	.file_off:	dw 0					; Current file offset
	.kv_orig_es:dw 0					; Original Pre KV ES
	.kv_orig_ds:dw 0					; Original Pre KV DS
	.kv_orig_si:dw 0					; Original Pre KV SI

; A look-up table of both keys and key handler sub-routines.
$cfg_keys:
	.count:
		db 3
	.strings:
		dw .echo_cmd, .vid_mode_cmd, .kernel_cmd
	.handlers:
		dw config.echo, config.vid_mode, config.kernel
	.echo_cmd:
		db "echo", 0
	.vid_mode_cmd:
		db "vid-mode", 0
	.kernel_cmd:
		db "kernel", 0

; Helper macro for saving file offset/segment
%macro pushfp 2
	mov word[cs:$cfg.file_seg], %1
	mov word[cs:$cfg.file_off], %2
%endmacro

; Helper macro for restoring file offset/segment
%macro popfp 2
	srmov %1, word[cs:$cfg.file_seg]
	mov %2, word[cs:$cfg.file_off]
%endmacro


; Parses the contents of the file currently loaded into the file buffer.
;	
;	IN: ES:BX => File buffer to parse
;
; Note: This will clobber registers
config.parse_file:
	.prologue:
		push bp
		mov bp, sp
	.prepare:
		mov word[cs:$cfg.orig_ds], ds	; Save the current DS value
		mov word[cs:$cfg.orig_gs], gs	; Save the current GS value
		mov word[cs:$cfg.orig_es], es	; Save the current ES value as file seg
		mov word[cs:$cfg.orig_bx], bx	; Save the current BX value as file off
		srmov gs, ds 					; Copy DS into GS
		srmov ds, es 					; Copy ES into DS
		mov si, bx						; Copy BX into SI, ES:BX => DS:SI
	.next_line:
		lodsb 							; Load 1st byte of the current line
		pushfp ds, si 					; Update the current file position
		cmp al, '#'						; Is the line a comment?
		je .line_is_comment
		cmp al, '%'						; Is the line a key-value pair?
		je .line_is_kv_pair
		cmp al, 0xD						; Is the line empty? (CR)
		je .end_of_line
		cmp al, 0xA						; Is the line empty? (NL/LF)
		je .end_of_line
		test al, al						; Is this the end of the file? (NUL)
		jz .parsing_completed			; The parsing has completed successfully
	.unexpected_character:				; At this point we can assume an
										; unexpected character was encountered.
		stc 							; Set the carry flag...
		jmp .finalize					; and jump to the finalizer to finish.
	.line_is_comment:					; The line is a comment, skip over it.
		call config.parse_comment
		jmp .end_of_line				; We have reached the end of the line
	.line_is_kv_pair:					; The line is a key-value pair.
		call config.parse_kv_pair
		jmp .end_of_line				; We have reached the end of the line
	.end_of_line:
		jmp .next_line					; Start parsing the next line.
	.parsing_completed:					; The parsing has finished successfully
		clc
	.finalize:
		srmov ds, word[cs:$cfg.orig_ds]	; Restore original DS
		srmov es, word[cs:$cfg.orig_es]	; Restore original ES
		srmov gs, word[cs:$cfg.orig_gs]	; Restore original GS
		mov bx, word[cs:$cfg.orig_bx]
	.epilogue:
		mov sp, bp
		pop bp
		ret

; Parse a configuration comment that has been encountered in the current file.
; This involves consuming each character until the next 0xA, 0xD or NUL byte is
; found.
config.parse_comment:
	.@@:
		lodsb 							; Fetch the next character
		cmp al, 0xD						; Is the line empty? (CR)
		je .return
		cmp al, 0xA						; Is the line empty? (NL/LF)
		je .return
		test al, al						; Is this the end of the file? (NUL)
		je .return
		jmp .@@							; Continue.
	.return:
		ret

; Parse a key-value pair that has been encountered in the current file.
config.parse_kv_pair:
	.prologue:
		push bp
		mov bp, sp
		push 0							; [bp - 2] Start of key
		push 0							; [bp - 4] Length of key
		push 0							; [bp - 6] Start of value
		push 0							; [bp - 8] Length of value
		push 0							; [bp - 10] Index of key
	.prepare:
		srmov word[cs:$cfg.kv_orig_es], es ; Save the current ES value
		srmov word[cs:$cfg.kv_orig_ds], ds ; Save the current DS value.
		mov word[bp - 2], si			; Note the current position. (Key Start)
	.next_key_char:
		lodsb 							; Fetch the next key character
		cmp al, ':'						; ':' denotes the end of the key...
		je .resolve_key
		cmp al, 0xD						; Is the line empty? (CR)
		je .unexpected_end_of_key
		cmp al, 0xA						; Is the line empty? (NL/LF)
		je .unexpected_end_of_key					
		test al, al 					; Are we at the end of the file?
		je .unexpected_end_of_key
		jmp .next_key_char
	.unexpected_end_of_key:
		stc 							; Set CF to indicate an error occured.
		jmp .finalize
	.resolve_key:
		mov ax, si						; Copy SI into AX for calculating length
		dec ax							; We need to ignore the delimiter
		sub ax, word[bp - 2]			; Subtract the start of key for length
		mov word[bp - 4], ax			; Update the length of key value
		mov word[bp - 6], si			; Note the beginning of the value
	.next_value_char:
		lodsb 							; Fetch the next value character
		cmp al, 0xD						; Is the line empty? (CR)
		je .resolve_value
		cmp al, 0xA						; Is the line empty? (NL/LF)
		je .resolve_value					
		test al, al 					; Are we at the end of the file?
		jz .resolve_value
		jmp .next_value_char
	.resolve_value:
		mov ax, si						; Copy SI into AX for calculating length
		dec ax							; We need to ignore the delimiter
		sub ax, word[bp - 6]			; Subtract the start of value for length
		mov word[bp - 8], ax			; Update the length of "value" value
	.look_up_key:
		pushfp ds, si 					; Save the current file position
		srmov ds, word[cs:$cfg.orig_ds] ; Restore the original DS value					
		xor cx, cx						; Count up from zero
	.next_key:
		push cx							; Save the current iteration index
		shl cx, 1						; Multiply by two for word based address
		mov word[bp - 10], cx			; Save the index of the key
		add cx, $cfg_keys.strings		; In relation to the look up table.
		mov si, cx						; Move SI to the location CX refers to.
		mov si, word[ds:si]
		mov cx, [bp - 4]				; Fetch the key length for comparison
		mov di, word[bp - 2]			; Fetch the config key for comparison
		rep cmpsb						; Counted comparison of each byte.
		je .key_match					; Keys are equal, hanlder needs calling
		pop cx							; Restore the iteration count
		inc cx							; Increment the iteration index
		cmp cl, byte[$cfg_keys.count]	; Have we iterated over all the keys?
		jl .next_key
	.key_not_found:
		mov si, .key_not_found_str
		call rs232.send_bytes
		stc
		jmp .finalize
	.key_match:
		mov si, word[bp - 10]			; Fetch the index of the key
		add si, $cfg_keys.handlers		; In relation to the look up table
		mov ax, word[ds:si]				; Get the routine pointer
		test ax, ax						; Make sure the pointer isn't NULL
		jnz .call_handler
		stc
		jmp .finalize
	.call_handler:
		srmov ds, word[cs:$cfg.kv_orig_ds]; Restore the original value of DS
		srmov es, word[cs:$cfg.kv_orig_es]; Restore the original value of ES
		clc								; Clear the error flag, it didn't fail
		mov si, word[bp - 6]			; Recall the start of the value
		mov cx, word[bp - 8]			; Recall the length of the value
		call ax							; Call the handler routine for the key
	.finalize:
		popfp ds, si 					; Restore the current file position
		srmov es, word[cs:$cfg.kv_orig_es]; Restore the original value of ES
		srmov ds, word[cs:$cfg.kv_orig_ds]; Restore the original value of DS
	.epilogue:
		add sp, 10
		mov sp, bp
		pop bp
		ret
	.key_not_found_str:
		db "Unexpected config key found.", 0x0

; Handler for the "echo" key. This will print any information passed to it back
; out to the RS232 serial port.
config.echo:
	.@@:
		lodsb 							; Fetch the next character
		cmp al, 0xD						; Is the line empty? (CR)
		je .return
		cmp al, 0xA						; Is the line empty? (NL/LF)
		je .return
		test al, al						; Is this the end of the file? (NUL)
		je .return
		call rs232.send_byte
		jmp .@@							; Continue.
	.return:
		mov al, 0xD
		call rs232.send_byte
		ret

; Handler for the "vid-mode" key. This will set the desired video mode on the
; computer for the user. The modes are "text", "native" and "graph". This
; will be used by the VESA module to appropriately choose a VESA mode for the
; screen.
config.vid_mode:
	.prepare:
		mov word[cs:.input_string], si	; Save the input string and its length
		mov word[cs:.input_string_length], cx
		xor cx, cx						; Set the counter to 0. Count upwards.
	.check_mode:
		push cx							; Save the current iteration index.
		push es							; Save the current ES.
		mov word[cs:.current_index], cx	; Save the index number for later
		shl cx, 1						; Multiply by 2 to get the array offset
		add cx, .mode_lookup			; Base it off the look up table.
		mov di, cx						; Copy CX into DI, and then load the
		mov di, word[cs:di]				; pointer for DI from itself.
		mov si, word[cs:.input_string]
		mov cx, word[cs:.input_string_length]
		srmov es, word[cs:$cfg.orig_ds]	; Load the original DS value into ES
		rep cmpsb						; Compare the strings
		pop es							; Restore the actual value of ES
		pop cx							; Restore the actual value of CX
		je .mode_found
		inc cx
		cmp cx, word[cs:.mode_count]	; Have we reached the end of the list?
		jnz .check_mode					; If not, then loop around again.
	.mode_not_found:
		mov byte[cs:.use_mode], VID_TEXT; Use the default text mode.
		jmp .set_mode
	.mode_found:
		mov cx, word[cs:.current_index]	; Restore the mode index
		mov byte[cs:.use_mode], cl		; and set the mode with it.
	.set_mode:
		srmov gs, BC_SEG				; Set the BootConfiguration segment 
		mov si, BC_OFFSET				; and offset.
		mov cl, byte[cs:.use_mode]		; Fetch the mode number
		mov byte[gs:si + BootConf.vmode], cl; and write it to the BootConfig.
		ret
	.input_string:
		dw 0x0000
	.input_string_length:
		dw 0x0000
	.current_index:
		dw 0x0000
	.use_mode:
		db VID_TEXT
	.mode_count:
		dw 3
	.mode_lookup:
		dw .text_str, .native_str, .graph_str
	.text_str:
		db "text", 0x0
	.native_str:
		db "native", 0x0
	.graph_str:
		db "graph", 0x0

; Handler for the "kernel" key. This specifies the name of the kernel image to
; locate and load into memory. Loading of the kernel will not happen until the
; second stage, once appropriate protected mode drivers have been installed.
config.kernel:
	.main:
		push es							; Save the current ES value
		push cx							; Save the length of the kernel name
		srmov es, BC_SEG				; Use ES to access the BootConfig
		mov di, BC_OFFSET				; Get to the BootConfig.
		mov cx, 64						; Field is 64 bytes.
		xor ax, ax						; Zero out AX so that we can clear the
		rep stosb						; BootConf field first
		pop cx							; Restore the kernel name length
		mov di, BC_OFFSET				; Get back to the start
		rep movsb						; Copy the name into the config.
		pop es							; Restore ES.
	.done:
		ret