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

; Screen configuration information. This is used to determine what functionality
; needs to be employed to deliver the correct behaviour.
$SCREEN:
	.crsr_x:
		dd 0
	.crsr_y:
		dd 0
	.width:
		dd 0
	.height:
		dd 0
	.clear:
		dd 0x00000000					; Sub-routine pointer for the clear func
	.putch:
		dd 0x00000000					; Sub-routine pointer for the putch func
	.puts:
		dd 0x00000000					; Sub-routine pointer for the puts func

; Initialise the screen driver for CoreLoader. This will preconfigure the
; function lookup table.
;
;	void _screen.init(void)
;
_screen.init:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov esi, BC_ADDR
		movzx eax, byte[esi + BootConf.vmode]; Look up the video mode.
		cmp al, VID_TEXT
		je .use_text
		jmp .epilogue
	.use_text:
		mov dword[$SCREEN.width], 80
		mov dword[$SCREEN.height], 25
		mov dword[$SCREEN.clear], _screen.text.clear
		mov dword[$SCREEN.putch], _screen.text.putch
		mov dword[$SCREEN.puts], _screen.text.puts
	.finalize:
		call _screen.clear
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

; Clear the screen.
;
;	void _screen.clear(void)
;
_screen.clear:
	.lookup:
		mov eax, [$SCREEN.clear]		; Look up the internal function
		test eax, eax					; Is it NULL?
		jz .missing						; If so skip calling it. It would error
		jmp eax							; Call it, and do not return.
	.missing:
		xor eax, eax					; We failed to call it. Return 0.
		ret

; Put a single character out the screen, at the current cursor location.
;
;	void _screen.putch(char c)
;
_screen.putch:
	.lookup:
		mov eax, [$SCREEN.putch]		; Look up the internal function
		test eax, eax					; Is it NULL?
		jz .missing						; If so skip calling it. It would error
		jmp eax							; Call it, and do not return.
	.missing:
		xor eax, eax					; We failed to call it. Return 0.
		ret

; Put a series of characters out the screen, at the current cursor location.
;
;	void _screen.puts(const char *str)
;
_screen.puts:
	.lookup:
		mov eax, [$SCREEN.puts]			; Look up the internal function
		test eax, eax					; Is it NULL?
		jz .missing						; If so skip calling it. It would error
		jmp eax							; Call it, and do not return.
	.missing:
		xor eax, eax					; We failed to call it. Return 0.
		ret

; Clear the screen for VGA Text Mode. Clears with the default attribute of 
; 0x07.
;
;	void _screen.text.clear(void)
;
_screen.text.clear:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov edi, 0xB8000				; Text mode screen buffer location
		mov eax, dword[$SCREEN.width]
		mov ebx, dword[$SCREEN.height]
		mul ebx							; Width * Height
		mov ecx, eax					; Copy EAX into ECX
		shr ecx, 1						; Divide by 2 to get the dwords.
		mov eax, 0x07200720				; Two empty cells in the screen.
		rep stosd
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

; Write out a single character to the current text buffer location. This will
; update screen scrolling offsets at the same time.
;
;	void _screen.text.putch(char c)
;
_screen.text.putch:
	.prologue:
		push ebp
		mov ebp, esp
	.calculate_offset:
		mov eax, dword[$SCREEN.crsr_y]	; Get the cursor y position
		mov ebx, dword[$SCREEN.width]	; Get the width
		mul ebx							; Y * Width
		add eax, dword[$SCREEN.crsr_x]	; + X
		shl eax, 1						; * 2
		add eax, 0xb8000				; + Screen Buffer Base
		mov edi, eax					; Set as the destination
	.fetch_char:
		mov eax, [ebp + 8]				; Arg #0: 'c'
		cmp al, 0xD						; Is this a newline?
		jne .copy_printable
	.copy_new_line:
		jmp .next_line
	.copy_printable:
		and eax, 0x000000FF				; Mask out the upper 3 bytes
		movzx ebx, word[edi]			; Fetch the current cell
		and ebx, 0x0000FF00				; Mask out the upper 3 bytes
		or ebx, eax						; EBX | EAX - Overwrite the cell content
		mov word[edi], bx				; Update the cell.
	.update_cursor:
		mov eax, dword[$SCREEN.crsr_x]	; Get the current cursor x
		inc eax							; ++x
		cmp eax, dword[$SCREEN.width]
		jge .next_line
		mov dword[$SCREEN.crsr_x], eax	; Update cursor x value
		jmp .epilogue
	.next_line:
		mov eax, dword[$SCREEN.crsr_y]	; Get the current cursor y
		inc eax							; ++y
		mov dword[$SCREEN.crsr_x], 0	; Update cursor x value 
		mov dword[$SCREEN.crsr_y], eax	; Update cursor y value 
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

; Write out a series character to the current text buffer location. This will
; update screen scrolling offsets at the same time.
;
;	void _screen.text.puts(const char *str)
;
_screen.text.puts:
	.prologue:
		push ebp
		mov ebp, esp
	.prepare:
		mov esi, [ebp + 8]
		and eax, 0x000000FF
	.@@:
		lodsb
		test al, al
		jz .epilogue
		push eax
		call _screen.text.putch
		add esp, 4
		jmp .@@
	.epilogue:
		mov esp, ebp
		pop ebp
		ret