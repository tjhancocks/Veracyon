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
;; Enable paging.
;;
;;	void enable_paging(void)
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
;; Setup identity mapping for the first 1MiB of memory.
;;
;;	void _prepare_lower_id_map(void)
;;
_prepare_lower_id_map:
	.prologue:
		push ebp
		mov ebp, esp
	.clear_page_table:
		mov edi, ID_MAP_PAGE_TABLE
		mov ecx, 0x400					; The table has 1024 entries
		xor eax, eax
		rep stosd
	.prepare_mappings:
		mov edi, ID_MAP_PAGE_TABLE
		mov eax, 0x3
		mov ecx, 0x100					; 256 Entries (Represents 1MiB of RAM)
	.add_mappings:
		stosd
		add eax, 0x1000					; Get the address of the next frame
		loop .add_mappings
	.add_to_directory:
		mov edi, PAGE_DIR
		mov dword[edi], ID_MAP_PAGE_TABLE | 0x3
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Allocate a new page table. This will clear the frame being allocated, and 
;; then add the appropriate entry into the page directory. If the page table
;; already exists then this function will do nothing.
;;
;; 	void alloc_page_table(unsigned int page_table)
;;
_alloc_page_table:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] Frame
	.check_page_table:
		mov esi, PAGE_DIR
		mov eax, [ebp + 8]				; Fetch the page table
		mov ebx, [esi + (eax * 4)]		; Fetch the directory entry
		test ebx, 0x1					; Is the page present?
		jnz .epilogue
	.allocate_frame:
		mov esi, BOOT_CONFIG
		mov eax, [esi + BootConf.next_ptable_frame]
		; TODO: Check to ensure we've not exhausted allowed frames!!
		mov [ebp - 4], eax				; Save the allocated frame
		add eax, 1
		mov [esi + BootConf.next_ptable_frame], eax
	.clear_page_table:
		mov edi, [ebp - 4]
		xor eax, eax
		mov ecx, 0x100
		rep stosd
	.add_to_directory:
		mov esi, PAGE_DIR
		mov eax, [ebp + 8]				; Fetch the page table
		mov ebx, [ebp - 4]				; Get the frame
		and ebx, 0xFFFFF000				; Ensure it is correctly masked
		or ebx, 0x3						; and configured correctly.
		mov [esi + (eax * 4)], ebx		; Add entry into directory
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Allocate the page and frame for the specified linear address. The mapping
;; should be an identity mapping only if specified. If the page is already
;; allocated then nothing will happen.
;;
;;	void alloc_page(void *linear, unsigned int identity_mapped)
;;
_alloc_page:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] Page Directory Index
		push 0							; [ebp - 8] Page Table Index
	.calculate_indexes:
		mov eax, [ebp + 8]				; Fetch the linear address
		shr eax, 12						; Get the Page Table Index
		mov ebx, eax					; Copy into ebx
		shr eax, 10						; Get the Page Directory Index
		and ebx, 0x3ff					; Mask out the Page Table Index
		mov [ebp - 4], eax
		mov [ebp - 8], ebx
	.check_page:
		mov esi, PAGE_DIR
		mov esi, [esi + (eax * 4)]		; Get the address of the page table
		test esi, 0x1					; Is the present flag set?
		jz .alloc_page_table			; No. Allocate the page table.
		and esi, 0xFFFFF000				; Mask out the flags
		mov edx, [esi + (ebx * 4)]		; Get the page entry from the page table
		test edx, 0x1					; Is the present flag set?
		jz .alloc_page					; No. Allocate the page
		nop
		jmp .epilogue
	.alloc_page_table:
		mov esi, BOOT_CONFIG
		mov ecx, [esi + BootConf.next_ptable_frame]
		push ecx
		add ecx, 0x1000					; Move to next page table frame
		mov [esi + BootConf.next_ptable_frame], ecx
		pop ecx
		and ecx, 0xFFFFF000				; Clear the page table flags area
		or ecx, 0x3						; Set the read/write and present flags
		mov esi, PAGE_DIR
		mov [esi + (eax * 4)], ecx		; Set the page table allocation
	.alloc_page:
		mov ecx, [ebp + 12]				; Fetch the identity_mapped flag
		test ecx, ecx					; and check if is set...
		jnz .identity_map_page			; It is, so make sure the page is ID'd
		mov esi, BOOT_CONFIG
		mov eax, [esi + BootConf.next_frame]
		push eax
		add eax, 0x1000					; Move to next page frame
		mov [esi + BootConf.next_frame], eax
		pop eax
		jmp .set_entry
	.identity_map_page:
		mov eax, [ebp + 8]				; Fetch the linear/frame address
		and eax, 0xFFFFF000
		or eax, 0x3						; Make sure the entry is configured
	.set_entry:
		mov ebx, [ebp - 4]
		mov esi, PAGE_DIR
		mov esi, [esi + (ebx * 4)]
		and esi, 0xFFFFF000
		mov ebx, [ebp - 8]
		and eax, 0xFFFFF000
		or eax, 0x3
		mov [esi + (ebx * 4)], eax		; Write the entry to the page table
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Allocates the specified region of memory.
;;
;;	void alloc_memory(void *ptr, unsigned int size, unsigned int id_map)
;;
_alloc_memory:
	.prologue:
		push ebp
		mov ebp, esp
	.calculate_required_pages:
		xor edx, edx
		mov eax, [ebp + 12]				; Fetch the requested size in bytes
		mov ebx, [ebp + 8]				; Fetch the requested start
		and ebx, 0xFFF					; Get the page boundary offset
		add eax, ebx					; Add it to the size
		mov ebx, 0x1000					; Divide by the size of a frame/page
		div ebx
		mov ecx, eax					; Move the page count into ECX
		mov eax, [ebp + 8]				; Fetch the requested start
		and eax, 0xFFFFF000				; and make sure its page aligned.
		cmp edx, 0						; Was their bytes remaining?
		jz .alloc_next_page				; No, then proceed
		add ecx, 1						; Yes. Increment the page count.
	.alloc_next_page:
		push ecx
		push dword[ebp + 16]			; Identity mapping
		push eax						; Linear address
		call _alloc_page 				; Allocate the page...
		pop eax
		add esp, 4
		pop ecx
		add eax, 0x1000					; Get to the next page.
		loop .alloc_next_page			; Continue until all pages are allocated
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Allocates required memory for the linear frame buffer if it is required.
;;
;; 	void alloc_vesa_memory(void)
;;
_alloc_vesa_memory:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov esi, BOOT_CONFIG
		movzx eax, byte[esi + BootConf.vesa_mode]
		test eax, eax
		jz .unrequired
		mov eax, [esi + BootConf.lfb]
		mov ebx, [esi + BootConf.screen_size]
		push 1							; Identity mapped
		push ebx						; The size of the buffer
		push eax						; The location of the buffer
		call _alloc_memory
		add esp, 12
	.clear_screen:
		mov esi, BOOT_CONFIG
		mov ecx, [esi + BootConf.screen_size]
		shr ecx, 2
		mov edi, [esi + BootConf.lfb]
		mov eax, [esi + BootConf.background_color]
		rep stosd
	.done:
		push strings.done
		call _send_serial_bytes
		jmp .epilogue
	.unrequired:
		push strings.unrequired
		call _send_serial_bytes
	.epilogue:
		mov esp, ebp
		pop ebp
		ret