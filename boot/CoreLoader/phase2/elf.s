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
	.e_magic			resd 1
	.e_class 			resb 1
	.e_data				resb 1
	.e_version			resb 1
	.e_os_abi			resb 1
	.e_abi_version		resb 1
	.e_pad				resb 7
ENDSTRUC

;;
;; ELF Header Structure.
;;
STRUC ELFHdr
	.e_ident			resb 16
	.e_type				resw 1
	.e_machine			resw 1
	.e_version			resd 1
	.e_entry			resd 1
	.e_phoff			resd 1
	.e_shoff			resd 1
	.e_flags			resd 1
	.e_ehsize			resw 1
	.e_phentsize		resw 1
	.e_phnum			resw 1
	.e_shentsize		resw 1
	.e_shnum			resw 1
	.e_shstrndx			resw 1
ENDSTRUC

;;
;; ELF Program Header Structure
;;
STRUC ELFPhdr
	.p_type				resd 1
	.p_offset			resd 1
	.p_vaddr			resd 1
	.p_paddr 			resd 1
	.p_filesz			resd 1
	.p_memsz			resd 1
	.p_flags			resd 1
	.p_align 			resd 1
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
	.parse_elf:
		call _elf_parse_phdr
		add esp, 4
	.launch:
		mov esi, [ebp + 8]
		mov eax, [esi + ELFHdr.e_entry]
		call eax
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
		add edi, ELFHdr.e_ident			; ensure that they match. If they do
		add edi, ELFIdent.e_magic		; then we have a valid ELF file. 
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
		movzx eax, byte[edi + ELFHdr.e_ident + ELFIdent.e_class]
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
		movzx eax, byte[edi + ELFHdr.e_ident + ELFIdent.e_data]
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
		movzx eax, byte[edi + ELFHdr.e_machine]
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
		movzx eax, byte[edi + ELFHdr.e_version]
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
		movzx eax, byte[edi + ELFHdr.e_type]
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

;;
;; Parse the program headers of the specified ELF file. This will kick off
;; relocation for those program sections accordingly.
;;
;;	void elf_parse_phdr(void *elf_data)
;;
_elf_parse_phdr:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] Program Header Offset
		push 0							; [ebp - 8] Program Header Count
		push 0							; [ebp - 12] Program Header Size
	.locate_program_headers:
		mov esi, [ebp + 8]				; Fetch the ELF Data location
		mov eax, [esi + ELFHdr.e_phoff]	; Get the program header offset
		add eax, esi					; Add the location of the ELF data
		mov [ebp - 4], eax				; Store for easy lookup in future
		movzx ecx, word[esi + ELFHdr.e_phnum]
		mov [ebp - 8], ecx				; Store the program header count
		movzx eax, word[esi + ELFHdr.e_phentsize]
		mov [ebp - 12], eax				; Store the program header size
	.get_program_header_entry:
		push dword[ebp - 4]				; Push program header as argument
		call _elf_describe_phdr
		add esp, 4
	.handle_entry:
		mov esi, [ebp - 4]				; Fetch the current program header
		mov eax, [esi + ELFPhdr.p_type]	; Get the program section type.
		cmp eax, 0x00000000
		je .next_entry					; Skip NULL sections, as per spec
		cmp eax, 0x00000001
		je .handle_load_section			; Handle a load section.
		nop
		jmp .unsupported_section		; We've hit an unsupported section.
	.handle_load_section:
		push dword[ebp + 8]				; Push the ELF location as 2nd arg
		push dword[ebp - 4]				; Push the program header as 1st arg
		call _elf_parse_load_section
		add esp, 8
	.next_entry:
		mov eax, [ebp - 4]				; Fetch the current program header
		add eax, [ebp - 12]				; Move to the next program header
		mov [ebp - 4], eax				; Update the current program header
		mov ecx, [ebp - 8]				; Get the current header count
		sub ecx, 1						; Subtract 1
		mov [ebp - 8], ecx
		cmp ecx, 0						; Have we reached the last one?
		je .end_of_list					; No, proceed to next entry
		jmp .get_program_header_entry
	.end_of_list:
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.unsupported_section:
		push .unsupported_message
		call _elf_panic
	.unsupported_message:
		db "Attempted to load an ELF file with unsupported section.", 0xA, 0x0

;;
;; Output the type name of the specified ELF Program Header.
;;
;;	void elf_describe_phdr(void *elf_phdr)
;;
_elf_describe_phdr:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .prefix
		call _send_serial_bytes
		add esp, 4
		mov esi, [ebp + 8]				; Fetch the Program Header
		mov eax, [esi + ELFPhdr.p_type]	; Lookup the type of section
		cmp eax, 0x00000000
		je .is_pt_null
		cmp eax, 0x00000001
		je .is_pt_load
		cmp eax, 0x00000002
		je .is_pt_dyn
		cmp eax, 0x00000003
		je .is_pt_interp
		cmp eax, 0x00000004
		je .is_pt_note
		cmp eax, 0x00000005
		je .is_pt_shlib
		cmp eax, 0x00000006
		je .is_pt_phdr
		cmp eax, 0x60000000
		je .is_pt_loos
		cmp eax, 0x6fffffff
		je .is_pt_hios
		cmp eax, 0x70000000
		je .is_pt_loproc
		cmp eax, 0x7fffffff
		jmp .finish
	.is_pt_null:
		push .pt_null
		jmp .show_type
	.is_pt_load:
		push .pt_load
		jmp .show_type
	.is_pt_dyn:
		push .pt_dyn
		jmp .show_type
	.is_pt_interp:
		push .pt_interp
		jmp .show_type
	.is_pt_note:
		push .pt_note
		jmp .show_type
	.is_pt_shlib:
		push .pt_shlib
		jmp .show_type
	.is_pt_phdr:
		push .pt_phdr
		jmp .show_type
	.is_pt_loos:
		push .pt_loos
		jmp .show_type
	.is_pt_hios:
		push .pt_hios
		jmp .show_type
	.is_pt_loproc:
		push .pt_loproc
		jmp .show_type
	.is_pt_hiproc:
		push .pt_hiproc
		jmp .show_type
	.show_type:
		call _send_serial_bytes
		add esp, 4
	.finish
		push .suffix
		call _send_serial_bytes
		add esp, 4
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.prefix:
		db "ELF Program Header (type = PT_", 0x0
	.suffix:
		db ")", 0xA, 0x0
	.pt_null:
		db "NULL", 0x0
	.pt_load:
		db "LOAD", 0x0
	.pt_dyn:
		db "DYNAMIC", 0x0
	.pt_interp:
		db "INTERP", 0x0
	.pt_note:
		db "NOTE", 0x0
	.pt_shlib:
		db "SHLIB", 0x0
	.pt_phdr:
		db "PHDR", 0x0
	.pt_loos:
		db "LOOS", 0x0
	.pt_hios:
		db "HIOS", 0x0
	.pt_loproc:
		db "LOPROC", 0x0
	.pt_hiproc:
		db "HIPROC", 0x0

;;
;; Parse a "load section" from the specified ELF file, using the specified
;; program header.
;;
;;	void elf_parse_load_section(void *elf_phdr, void *elf_data)
;;
_elf_parse_load_section:
	.prologue:
		push ebp
		mov ebp, esp
	.allocate_memory:
		mov esi, [ebp + 8]				; Fetch the program header
		mov edi, [esi + ELFPhdr.p_vaddr]; Get the virtual address of the section
		mov ecx, [esi + ELFPhdr.p_memsz]; Get the size of the section
		push 0							; Not identity mapped
		push ecx						; The amount of memory
		push edi						; The location of memory
		call _alloc_memory				; Allocate the requested memory
		pop edi							; Restore location
		pop ecx							; Restore amount of memory
	.clear_memory:
		xor eax, eax
		shr ecx, 2						; Divide by 4, operate on dwords
		rep stosd
	.copy_section:
		mov esi, [ebp + 8]				; Fetch the program header
		mov edi, [esi + ELFPhdr.p_vaddr]; Get the virtual address of the section
		mov ecx, [esi + ELFPhdr.p_filesz]; Get the size of the section in file
		mov esi, [esi + ELFPhdr.p_offset]; Get the file offset of the section
		add esi, [ebp + 12]				; Add the location of the ELF data
		rep movsb
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
