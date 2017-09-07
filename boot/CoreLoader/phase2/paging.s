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
;; Setup the basic paging environment and identity map the lowest 1MiB of
;; memory. This will ensure that the BootLoader and any early code/data is still
;; addressable once paging is enabled. In this the Page Table must be 
;; established as well.
;;
;;	void prepare_paging(void)
;;
_prepare_paging:
	.prologue:
		push ebp
		mov ebp, esp
	.page_directory:
		mov edi, PAGE_DIR				; Location of the root page directory
		mov ecx, 0x400					; 4KiB - 1024 Dwords
		xor eax, eax
		cld
		rep stosd
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Enable the paging environment. This must not be called before identity 
;; mapping is in place otherwise the system will triple fault.
;;
;; 	void enable_paging(void)
;;
_enable_paging:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov eax, PAGE_DIR
		mov cr3, eax
		mov eax, cr0
		or eax, 0x80000001
		mov cr0, eax
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Setup Identity Mapping for the lower 1MiB.
;;
;;	void configure_lower_identity_mapping(void)
;;
_configure_lower_identity_mapping:
	.prologue:
		push ebp
		mov ebp, esp
	.clear_page_table:
		mov edi, ID_MAP_PAGE_TABLE
		mov ecx, 0x400
		xor eax, eax
		cld
		rep stosd
	.populate_page_table:
		mov edi, ID_MAP_PAGE_TABLE
		mov ecx, 0x100					; 256 Entries (1MiB of memory)
		xor eax, eax
		or eax, 0x03
	.L0:
		stosd
		add eax, 0x1000 				; Next page
		loop .L0
	.register_page_table:
		mov edi, PAGE_DIR				; Page Directory. First entry.
		mov eax, ID_MAP_PAGE_TABLE | 3
		stosd
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Setup the Kernel Page Tables.
;;
;; 	void prepare_kernel_page_tables(void)
;;
_prepare_kernel_page_tables:
	.prologue:
		push ebp
		mov ebp, esp
	.skipped:
		push strings.skipped
		call _send_serial_bytes
		add esp, 4
		jmp .epilogue
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Setup the Linear Frame Buffer Page Tables.
;;
;; 	void prepare_lfb_page_tables(void)
;;
_prepare_lfb_page_tables:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] Page table count
		push 0							; [ebp - 8] Linear Frame Buffer Base
		push 0							; [ebp - 12] Linear Frame Buffer Size
	.required_test:
		mov esi, BOOT_CONFIG
		movzx eax, byte[esi]
		or eax, eax
		jz .unrequired
	.calculate_page_table_count:
		xor edx, edx
		mov eax, [esi + 0x29]			; Fetch the linear frame buffer
		mov [ebp - 8], eax				; Store it.
		mov eax, [esi + 0x39]			; Fetch the screen size in bytes
		mov [ebp - 12], eax
		shr eax, 24						; Calcuate how many page tables are
		mov [ebp - 4], eax				; required and save it
		mov ecx, eax
		add ecx, 1
		mov edi, PAGE_DIR				; Location of root page directory
		mov eax, VESA_PAGE_TABLE_1		; First VESA page table is at...
		mov ebx, [ebp - 8]				; Restore the linear frame buffer base
		shr ebx, 22						; Get the page directory offset
	.next_page_table:
		and eax, 0xFFFFF000
		or eax, 0x03					; Apply attributes to the frame address
		mov dword[edi + ebx * 4], eax	; Store the page table frame
		add eax, 0x1000					; Move to the start of the next frame
		loop .next_page_table
	.setup_lfb_frames:
		mov eax, [ebp - 8]				; Fetch the linear frame buffer base
		or eax, 0x03					; Set appropriate attributes on entry 
		mov ecx, [ebp - 12]				; Fetch the linear frame buffer size
		shr ecx, 12						; Divide by the size of a page
		add ecx, 1
		mov edi, VESA_PAGE_TABLE_1		; Location of the first VESA Page Table
	.allocate_lfb_frame:
		stosd
		add eax, 0x1000
		loop .allocate_lfb_frame
	.clear_screen:
		mov edi, [ebp - 8]				; Fetch the linear frame buffer location
		mov ecx, [ebp - 12]				; Fetch the linear frame buffer size
		shr ecx, 2						; Divide by 4
		mov esi, BOOT_CONFIG
		mov eax, [esi + BootConf.background_color]
		rep stosd
		jmp .done
	.unrequired:
		push strings.unrequired
		call _send_serial_bytes
		add esp, 4
		jmp .epilogue
	.done:
		push strings.done
		call _send_serial_bytes
		add esp, 4
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
