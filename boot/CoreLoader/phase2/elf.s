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

	[bits 	32]

;;
;; ELF Ident Structure.
;;
STRUC ELFIdent
	.magic			resd 1
	.class 			resb 1
	.data			resb 1
	.version		resb 1
	.os_abi			resb 1
	.abi_version	resb 1
	.pad			resb 7
ENDSTRUC

;;
;; ELF Header Structure.
;;
STRUC ELFHeader
	.ident			resb 16
	.type			resw 1
	.machine		resw 1
	.version		resd 1
	.entry			resd 1
	.phoff			resd 1
	.shoff			resd 1
	.flags			resd 1
	.ehsize			resw 1
	.phentsize		resw 1
	.phnum			resw 1
	.shentsize		resw 1
	.shnum			resw 1
	.shstrndx		resw 1
ENDSTRUC

;;
;; Start the process of loading an ELF file. It will attempt to parse the
;; contents of memory at the specified memory location.
;;
;;	void elf_load(void *elf_data)
;;
_elf_load:
	.prologue:
		push ebp
		mov ebp, esp
	.check_is_elf:
		push dword[ebp + 8]
		call _elf_check_file
		call _elf_check_32
		call _elf_check_endianess
		call _elf_check_machine_isa
		call _elf_check_version
		call _elf_check_type
	.determine_type:
	.load_relocatable_elf:
	.load_executable_elf:
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; ELF Related Panic. This will halt the boot process and output an error 
;; message.
;;
;;	void elf_panic(const char *message)
;;
_elf_panic:
	.prologue:
		push ebp
		mov esp, ebp
	.main:
		push .panic
		call _send_serial_bytes
		add esp, 4
		push dword[ebp + 8]
		call _send_serial_bytes
	.halt:
		cli
		hlt
		jmp $
	.panic:
		db 0xA, "ELF Panic!", 0xA, 0x0

;;
;; Check the specified data to ensure that it is an ELF file.
;;
;;	void elf_check_file(void *elf_data)
;;
_elf_check_file:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .message
		call _send_serial_bytes
		mov esi, .magic					; Fetch the ELF data and compare the 1st
		mov edi, [ebp + 8] 				; 4 bytes with the magic number and
		add edi, ELFHeader.ident		; ensure that they match. If they do
		add edi, ELFIdent.magic			; then we have a valid ELF file. 
		mov ecx, 4
		rep cmpsb
		jne .error						; If not equal then error!
		push strings.done
		call _send_serial_bytes
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.error:
		push .err_message
		call _elf_panic
	.magic:
		db 0x7f, "ELF"
	.err_message:
		db "The provided file was not a compatible ELF file.", 0xA, 0x0
	.message:
		db "Checking ELF magic number... ", 0x0

;;
;; Check the specified data to ensure that it is a 32-bit ELF file.
;;
;;	void elf_check_32(void *elf_data)
;;
_elf_check_32:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .message
		call _send_serial_bytes
		mov edi, [ebp + 8]				; Fetch the ELF data.
		movzx eax, byte[edi + ELFHeader.ident + ELFIdent.class]
		cmp eax, 1						; Ensure we're looking at class 1
		jne .error
		push strings.done
		call _send_serial_bytes
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.error:
		push .err_message
		call _elf_panic
	.err_message:
		db "The provided file was not a 32-bit compatible ELF file.", 0xA, 0x0
	.message:
		db "Checking for 32-bit compatible ELF... ", 0x0


;;
;; Check the specified data to ensure that it is in a little-endian format.
;;
;;	void elf_check_endianess(void *elf_data)
;;
_elf_check_endianess:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .message
		call _send_serial_bytes
		mov edi, [ebp + 8]				; Fetch the ELF data.
		movzx eax, byte[edi + ELFHeader.ident + ELFIdent.data]
		cmp eax, 1						; Ensure we're looking at class 1
		jne .error
		push strings.done
		call _send_serial_bytes
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.error:
		push .err_message
		call _elf_panic
	.err_message:
		db "The provided file was not a little endian encoded ELF file."
		db 0xA, 0x0
	.message:
		db "Checking for a little endian encoded ELF file... ", 0x0

;;
;; Check the specified machine instruction set used by the ELF file.
;;
;;	void elf_check_machine_isa(void *elf_data)
;;
_elf_check_machine_isa:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .message
		call _send_serial_bytes
		mov edi, [ebp + 8]				; Fetch the ELF data.
		movzx eax, byte[edi + ELFHeader.machine]
		cmp eax, 3						; Ensure we're looking at x86 (3)
		jne .error
		push strings.done
		call _send_serial_bytes
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.error:
		push .err_message
		call _elf_panic
	.err_message:
		db "The provided file was an x86 compatible ELF."
		db 0xA, 0x0
	.message:
		db "Checking for an x86 compatible ELF file... ", 0x0

;;
;; Check the specified version of the ELF file.
;;
;;	void elf_check_version(void *elf_data)
;;
_elf_check_version:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .message
		call _send_serial_bytes
		mov edi, [ebp + 8]				; Fetch the ELF data.
		movzx eax, byte[edi + ELFHeader.version]
		cmp eax, 1						; Ensure we're looking version 1
		jne .error
		push strings.done
		call _send_serial_bytes
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.error:
		push .err_message
		call _elf_panic
	.err_message:
		db "The provided file was an unsupport version of ELF."
		db 0xA, 0x0
	.message:
		db "Checking the ELF file version is compatible... ", 0x0

;;
;; Check the specified ELF file is in fact an executable or relocatable ELF 
;; file.
;;
;;	void elf_check_type(void *elf_data)
;;
_elf_check_type:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .message
		call _send_serial_bytes
		mov edi, [ebp + 8]				; Fetch the ELF data.
		movzx eax, byte[edi + ELFHeader.type]
		cmp eax, 2						; Executable?
		jne .error
		push strings.done
		call _send_serial_bytes
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.error:
		push .err_message
		call _elf_panic
	.err_message:
		db "The provided file was an unsupport type of ELF file."
		db 0xA, 0x0
	.message:
		db "Checking the ELF type is compatible... ", 0x0