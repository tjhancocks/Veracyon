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

	[bits	16]

;;
;; The following defines the layout of the Boot Configuration structure.
;;
STRUC BootConf
	.vesa_mode			resb 1
	.kernel_name		resb 31
	.kernel_base		resd 1
	.width				resw 1
	.height				resw 1
	.depth				resb 1
	.lfb				resd 1
	.background_color	resd 1
ENDSTRUC

;;
;; Produce the default boot configuration structure in memory. This may be
;; overridden by user specified information later.
;;
prepare_boot_configuration_defaults:
	.prologue:
		push bp
		mov bp, sp
		pusha
		push es
	.clear:
		mov di, 0xFE00
		mov cx, 0x100
		xor ax, ax
		rep stosb
	.fill:
		mov di, 0xFE00
		mov byte[di], 0x01				; VESA Mode
		mov di, 0xFE01
		mov si, .fat_kernel_name
		mov cx, 11
		rep movsb						; Write the kernel name (assume FAT)
		mov di, 0xFE00
		mov dword[di + 0x20], 0x100000	; Kernel Address (1MiB)
		mov word[di + 0x24], 800		; Resolution Width
		mov word[di + 0x26], 600		; Resolution Height
		mov word[di + 0x28], 32			; Resolution Depth
		mov dword[di + 0x2A], 0x00000	; Linear Frame Buffer / VGA Buffer
	.epilogue:
		pop es
		popa
		mov sp, bp
		pop bp
		ret
	.fat_kernel_name:
		db "VKERNEL    "

;;
;; Load the boot configuration file from a FAT12 boot disk.
;;
load_boot_configuration_fat12:
	.prologue:
		push bp
		mov bp, sp
	.read_file:
		mov si, .config_name
		call fat12_read_file
	.check_for_error:
		jc .epilogue
		call parse_boot_config
		jc .epilogue
		mov si, strings16.done
		call send_serial_bytes
	.epilogue:
		clc
		mov sp, bp
		pop bp
		ret
	.config_name:
		db "BOOT    CLI"

;;
;; Parse the current file buffer as a boot configuration file.
;;
parse_boot_config:
	.prologue:
		push bp
		mov bp, sp
		push ds
		push 0x3000
		pop ds
		push 0x0000
		pop es
	.main:
		xor si, si						; Start at the beginning of the file.
	.next_line:
		cmp byte[si], '#'				; Do a check for a comment
		je .L0							; Consume the comment
		cmp byte[si], 0xA				; Is the character a carriage return?
		jne .null_check_1
		inc si
		jmp .next_line
	.null_check_1:
		cmp byte[si], 0x0				; Is the character a NULL character?
		je .eof1
		jmp .read_key
	.L0:
		cmp byte[si], 0xA				; Is the character a carriage return?
		jne .null_check_2
		inc si
		jmp .next_line
	.null_check_2:
		cmp byte[si], 0x0				; Is this a null character?
		je .eof1
		inc si
		jmp .L0							; Check the next character!
	.eof1:
		jmp .epilogue
	.read_key:
		mov di, .key_buffer				; We're going to be writing to the key
		push si							; Save the location in file
		push ds							; Save the DS segment
		push 0x0000
		pop ds							; Reset DS to normal
		mov si, di						; Switch to position in key buffer
		xor ax, ax
		mov cx, 16
		rep stosb						; Clear the key buffer
		pop ds							; Restore DS
		pop si							; Restore location in file
		mov di, .key_buffer				; We're going to be writing to the key
	.next_key_char:
		cmp byte[si], ':'				; Is this the key value delimiter?
		je .read_value
		movsb 							; Copy char from the file to key buffer
		jmp .next_key_char				; Next key char
	.read_value:
		mov di, .value_buffer			; We're going to be writing to the value
		push si							; Save the location in file
		push ds							; Save the DS segment
		push 0x0000
		pop ds							; Reset DS to normal
		mov si, di						; Switch to position in value buffer
		xor ax, ax
		mov cx, 257
		rep stosb						; Clear the value buffer
		pop ds							; Restore DS
		pop si							; Restore location in file
		inc si							; Skip over the ':'
		mov di, .value_buffer			; We're going to be writing to the value
	.next_value_char:
		cmp byte[si], 0xA				; End of the value by new line
		je .handle_key_value
		cmp byte[si], 0x0				; End of the value by null character
		je .handle_key_value
		movsb 							; Copy char from file to value buffer
		jmp .next_value_char
	.handle_key_value:
		pop ds
		push si
	.message:
		call check_debug_message
		jc .vga_text
		nop
		jmp .finished_key_value
	.vga_text:
		call check_vga_text_mode
		jc .screen_width
		nop
		jmp .finished_key_value
	.screen_width:
		call check_screen_width
		jc .screen_height
		nop
		jmp .finished_key_value
	.screen_height:
		call check_screen_height
		jc .default_background
		nop
		jmp .finished_key_value
	.default_background:
		call check_default_background
		jc .finished_key_value
		nop
		jmp .finished_key_value
	.finished_key_value:
		pop si
		inc si
		push ds
		push 0x3000
		pop ds
		push 0x0000
		pop es
		jmp .next_line
	.epilogue:
		pop ds
		mov sp, bp
		pop bp
		ret
	.bad_conf:
		db "bad configuration file.", 0xA, 0x0
	.key_buffer:
		times 16 db 0
	.value_buffer:
		times 257 db 0

;;
;; Check for and handle a debug message key-value.
;;
check_debug_message:
	.main:
		mov si, parse_boot_config.key_buffer
		mov di, .key
		mov cx, 7
		rep cmpsb
		jne .failed
		nop
	.print:
		mov si, parse_boot_config.value_buffer
		call dbg_put_string
		mov si, .nl
		call dbg_put_string
		clc
		ret
	.failed:
		stc
		ret
	.key:
		db "message"
	.nl:
		db 0xD, 0xA, 0x0

;;
;; Check for and handle a vga text mode key-value
;;
check_vga_text_mode:
	.main:
		mov si, parse_boot_config.key_buffer
		mov di, .key
		mov cx, 8
		rep cmpsb
		jne .failed
		nop
	.check_state:
		mov si, parse_boot_config.value_buffer
		mov di, .enabled
		mov cx, 7
		rep cmpsb
		je .enable_text_mode
		nop
	.enable_vesa_mode:
		mov di, 0xfe00
		mov byte[di + BootConf.vesa_mode], 1
		jmp .done
	.enable_text_mode:
		mov di, 0xfe00
		mov byte[di + BootConf.vesa_mode], 0
		mov word[di + BootConf.width], 80
		mov word[di + BootConf.height], 25
		mov dword[di + BootConf.lfb], 0xB8000
	.done:
		clc
		ret
	.failed:
		stc
		ret
	.key:
		db "vga-text"
	.enabled:
		db "enabled"

;;
;; Check for and handle a resolution width key-value
;;
check_screen_width:
	.main:
		mov si, parse_boot_config.key_buffer
		mov di, .key
		mov cx, 12
		rep cmpsb
		jne .failed
		nop
	.parse_value:
		mov si, parse_boot_config.value_buffer
		call str_to_num
		mov di, 0xfe00
		mov word[di + BootConf.width], ax
	.done:
		clc
		ret
	.failed:
		stc
		ret
	.key:
		db "screen-width"

;;
;; Check for and handle a resolution height key-value
;;
check_screen_height:
	.main:
		mov si, parse_boot_config.key_buffer
		mov di, .key
		mov cx, 13
		rep cmpsb
		jne .failed
		nop
	.parse_value:
		mov si, parse_boot_config.value_buffer
		call str_to_num
		mov di, 0xfe00
		mov word[di + BootConf.height], ax
	.done:
		clc
		ret
	.failed:
		stc
		ret
	.key:
		db "screen-height"

;;
;; Check for and handle a default background key-value
;;
check_default_background:
	.main:
		mov si, parse_boot_config.key_buffer
		mov di, .key
		mov cx, 10
		rep cmpsb
		jne .failed
		nop
	.parse_value:
		mov si, parse_boot_config.value_buffer
		call hex_to_num
		mov di, 0xfe00
		mov dword[di + BootConf.background_color], eax
	.done:
		clc
		ret
	.failed:
		stc
		ret
	.key:
		db "background"

;;
;; Convert a string into a number.
;;
;;	IN: SI => String
;; OUT: AX => Value
;;
str_to_num:
	.main:
		xor ebx, ebx
	.next_char:
		xor eax, eax
		lodsb
		or ax, ax
		jz .done
		cmp ax, '0'
		jl .error
		cmp ax, '9'
		jg .error
		sub ax, '0'
		xchg ebx, eax
		mov edx, 10
		mul dx
		add eax, ebx
		xchg ebx, eax
		jmp .next_char
	.error:
		xor eax, eax
	.done:
		mov eax, ebx
		ret

;;
;; Convert a hex string into a number.
;;
;; 	IN: SI => String
;; OUT: EAX => Value
;;
hex_to_num:
	.main:
		xor ebx, ebx
	.next_char:
		xor eax, eax
		lodsb
		or ax, ax
		jz .done
		cmp ax, '0'
		jl .lower_check
		cmp ax, '9'
		jg .lower_check
		sub ax, '0'
	.merge:
		xchg ebx, eax
		mov edx, 16
		mul dx
		add eax, ebx
		xchg ebx, eax
		jmp .next_char
	.lower_check:
		cmp ax, 'a'
		jl .upper_check
		cmp ax, 'f'
		jg .upper_check
		sub ax, 'a'
		jmp .merge
	.upper_check:
		cmp ax, 'A'
		jl .error
		cmp ax, 'F'
		jg .error
		sub ax, 'A' - 10
		jmp .merge
	.error:
		xor eax, eax
	.done:
		mov eax, ebx
		ret


;;
;; Simple debug function to write a string to the screen. This is used for 
;; debugging purposes to ensure the boot configuration is working correctly.
;;
dbg_put_string:
	.prologue:
		push bp
		mov bp, sp
	.main:
		mov ah, 0xE
	.next_char:
		lodsb
		or al, al
		jz .epilogue
		int 0x10
		jmp .next_char
	.epilogue:
		mov sp, bp
		pop bp
		ret