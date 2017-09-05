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
;; The following defines the layout of the VESA VBE Info structure.
;;
STRUC VBEInfo
	.signature 			resb 4
	.version 			resw 1
	.oem				resd 1
	.capabilities		resd 1
	.video_modes_off	resw 1
	.video_modes_seg	resw 1
	.video_memory		resw 1
	.software_revision	resw 1
	.vendor				resd 1
	.product_name		resd 1
	.product_revision	resd 1
	.reserved			resb 222
	.oem_data			resb 256
ENDSTRUC

;;
;; The following defines the layout of the EDID structure. EDID is used by the
;; hardware to instruct the software of its preferred/native resolution.
;;
STRUC EDID
	.padding 			resb 8
	.manufacturer_id	resb 2
	.edid_code			resb 2
	.serial_code		resb 4
	.week_number		resb 1
	.manufacturer_year	resb 1
	.edid_version		resb 1
	.edid_revision		resb 1
	.video_input		resb 1
	.width_cm			resb 1
	.height_cm			resb 1
	.gamma_factor		resb 1
	.dpms_flags			resb 1
	.chroma				resb 10
	.timings1			resb 1
	.timings2			resb 1
	.reserved_timing	resb 0
	.standard_timings	resb 16
	.timing_desc1		resb 18
	.timing_desc2		resb 18
	.timing_desc3		resb 18
	.timing_desc4		resb 18
	.reserved			resb 1
	.checksum			resb 1
ENDSTRUC

;;
;; The following defines the layout of the VBE Mode Info structure. This
;; structure describes the configuration and how to use the mode.
;;
STRUC VBEMode
	.attributes			resb 2
	.window_a			resb 1
	.window_b 			resb 1
	.granularity		resb 2
	.window_size		resb 2
	.segment_a			resb 2
	.segment_b			resb 2
	.win_func_ptr		resb 4
	.pitch				resb 2
	.width				resb 2
	.height				resb 2
	.w_char				resb 1
	.h_char				resb 1
	.planes				resb 1
	.bpp				resb 1
	.banks				resb 1
	.memory_model		resb 1
	.bank_size			resb 1
	.image_pages		resb 1
	.reserved0			resb 1
	.red_mask			resb 1
	.red_position		resb 1
	.green_mask			resb 1
	.green_position		resb 1
	.blue_mask			resb 1
	.blue_position		resb 1
	.reserved_mask		resb 1
	.reserved_position	resb 1
	.direct_color_attr	resb 1
	.frame_buffer		resb 4
	.off_screen_mem_off	resb 4
	.off_screen_mem_sz	resb 2
	.reserved1			resb 206
ENDSTRUC

;;
;; The following defines the layout of the screen configuration structure.
;; This is used to keep track of the VESA VBE configuration being used.
;;
STRUC ScreenConf
	.width				resb 2
	.height				resb 2
	.depth				resb 2
ENDSTRUC

;;
;; Prepare VBE Information and settings. This will check to ensure that VGA
;; Text Mode has not been requested in the configuration, and then try to set
;; the optimal resolution for the system, unless a different resolution has
;; been requested.
;;
prepare_vesa:
	.prologue:
		push bp
		mov bp, sp
		push 0							; [bp - 2] VBE Mode Number
		push 0							; [bp - 4] VBE Mode Offset
	.check_vga_text_mode:
		mov di, 0xfe00					; Location of the Boot Configuration
		cmp byte[di], 0		
		jne .get_vesa_info
	.no_vesa:
		mov si, strings16.skipped
		call send_serial_bytes
		jmp .epilogue
	.get_vesa_info:
		push es
		mov di, 0xf000					; Location of the VESA VBE Info
		mov dword[di + VBEInfo.signature], "VBE2"
		mov ax, 0x4f00					; BIOS Function to get VESA VBE Info
		int 0x10						; Call BIOS
		pop es
		cmp ax, 0x004f					; Check for success.
		je .L01
		nop
		mov si, strings16.unavailable
		jmp vesa_vbe_error
	.L01:
		mov di, 0xf000					; Location of the VESA VBE Info
		mov eax, dword[di + VBEInfo.signature]
		cmp eax, "VESA"					; Have we got the correct signature?
		je .L02
		nop
		mov si, strings16.unavailable
		jmp vesa_vbe_error
	.L02:
		movzx eax, word[di + VBEInfo.version]
		cmp ax, 0x0200					; Is the VESA version too old?
		jge .read_edid
		nop
		mov si, strings16.unavailable
		jmp vesa_vbe_error
	.read_edid:
		push es
		mov ax, 0x4f15					; BIOS function to get EDID info
		mov bx, 0x1
		xor cx, cx
		xor dx, dx
		mov di, 0xf400					; Location of the VESA EDID Info
		int 0x10						; Call out to BIOS
		pop es
		cmp ax, 0x004f					; Check for success.
		jne .L03
		nop
		jmp .determine_preferred_vbe_mode
	.L03:
		jmp .use_default_vbe_mode
	.determine_preferred_vbe_mode:
		mov di, 0xf400					; Location of the VESA EDID Info
		mov si, 0xf600					; Location of the Screen Configuration.
		movzx eax, byte[di + EDID.timing_desc1]
		or al, al
		jz .L03							; Bad data. Use default mode instead!
		movzx eax, byte[di + EDID.timing_desc1 + 2] ; Low byte of width
		mov ebx, eax
		movzx eax, byte[di + EDID.timing_desc1 + 4]
		and eax, 0xf0
		shl eax, 4
		or eax, ebx
		mov word[si + ScreenConf.width], ax
		movzx eax, byte[di + EDID.timing_desc1 + 5] ; Low byte of height
		mov ebx, eax
		movzx eax, byte[di + EDID.timing_desc1 + 7]
		and eax, 0xf0
		shl eax, 4
		or eax, ebx
		mov word[si + ScreenConf.height], ax
	.validate_mode:
		movzx eax, word[si + ScreenConf.width]
		or ax, ax
		jz .use_default_vbe_mode
		movzx eax, word[si + ScreenConf.height]
		or ax, ax
		jz .use_default_vbe_mode
		nop
		jmp .set_vbe_mode
	.use_default_vbe_mode:
		mov di, 0xfe00					; Location of the Boot Configuration
		mov si, 0xf600					; Location of the Screen Configuration.
		movzx eax, word[di + 0x24]		; Fetch the desired/default width
		movzx ebx, word[di + 0x26]		; Fetch the desired/default height
		mov word[si + ScreenConf.width], ax
		mov word[si + ScreenConf.height], bx
	.set_vbe_mode:
		mov word[si + ScreenConf.depth], 32
		mov di, 0xf000					; Location of the VBE VESA Info
		movzx esi, word[di + VBEInfo.video_modes_off]
		mov [bp - 4], si				; Keep the offset in local memory
	.find_vbe_mode:
		push fs
		mov di, 0xf000					; Location of the VBE VESA Info
		movzx eax, word[di + VBEInfo.video_modes_seg]
		mov fs, ax
		movzx esi, word[bp - 4]			; Fetch the offset
		movzx edx, word[fs:si]
		add si, 2
		mov word[bp - 4], si			; Update the offset
		mov word[bp - 2], dx			; Update the current mode
		pop fs
		movzx eax, word[bp - 2]			; Fetch the current mode
		cmp ax, 0xFFFF					; Is this the end of the list?
		je .vbe_mode_not_found
		nop
		jmp .get_vbe_mode_info
	.vbe_mode_not_found:
		mov si, strings16.unavailable
		jmp vesa_vbe_error
	.get_vbe_mode_info:
		push es
		mov ax, 0x4f01					; BIOS function to get VBE Mode Info
		movzx ecx, word[bp - 2]			; Fetch the current mode
		mov di, 0xf200					; Location of the VBE Mode Info
		int 0x10						; Call out to BIOS
		pop es
		cmp ax, 0x004f					; Check for success.
		jne .L04
		nop
		jmp .check_vbe_mode
	.L04:
		mov si, strings16.unavailable
		jmp vesa_vbe_error
	.check_vbe_mode:
		mov si, 0xf600					; Location of the Screen Configuration.
		mov di, 0xf200					; Location of the VBE Mode Info
		movzx eax, word[si + ScreenConf.width]
		movzx ebx, word[di + VBEMode.width]
		cmp ax, bx
		jne .next_vbe_mode				; Incorrect width
		movzx eax, word[si + ScreenConf.height]
		movzx ebx, word[di + VBEMode.height]
		cmp ax, bx
		jne .next_vbe_mode				; Incorrect height
		movzx eax, word[si + ScreenConf.depth]
		movzx ebx, byte[di + VBEMode.bpp]
		cmp ax, bx
		jne .next_vbe_mode				; Incorrect depth (Bits per pixel)
		movzx eax, word[di + VBEMode.attributes]
		cmp ax, 0x0081
		jz .next_vbe_mode				; Incorrect attributes (LFB Missing)
		nop
		jmp .found_vbe_mode
	.next_vbe_mode:
		nop
		jmp .find_vbe_mode
	.found_vbe_mode:
		push es
		mov ax, 0x4f02					; BIOS function to set the VESA mode
		movzx ebx, word[bp - 2]			; Fetch the current VBE mode
		or ebx, 0x4000
		xor cx, cx
		xor dx, dx
		xor di, di
		int 0x10						; Invoke BIOS
		pop es
		cmp ax, 0x004f					; Check for success.
		jne .L05
		nop
		jmp .save_vbe_mode_info
	.L05:
		nop
		mov si, strings16.unavailable
		jmp vesa_vbe_error
	.save_vbe_mode_info:

	.finish:
		mov si, strings16.done
		call send_serial_bytes
		jmp .epilogue
	.epilogue:
		mov sp, bp
		pop bp
		ret

;;
;; A collection of VESA error reasons.
;;
vesa_error_string:

;;
;; This is a naked function hanlder for systems that do not have VESA VBE
;; functions available.
;; WARNING: Do not call directly.
;;
vesa_vbe_error:
	.main:
		call send_serial_bytes
	.epilogue:
		mov sp, bp
		pop bp
		ret
