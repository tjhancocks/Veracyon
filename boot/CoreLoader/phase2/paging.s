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
;; Allocate and prepare the specified page table. Specified frames should be
;; identity mapped!
;;
;;	void alloc_page_table(unsigned int page_table, void *frame);
;;
_alloc_page_table:
	.prologue:
		push ebp
		mov ebp, esp
	.check_page_dir:
		mov edi, PAGE_DIR
		mov eax, [ebp + 8]				; Fetch the page_table number
		mov ebx, [edi + (eax * 4)]		; Check to see if the entry is used
		test ebx, 0x1					; Is the page present?
		jnz .epilogue					; Jump to end if page table is present
		mov ebx, [ebp + 12]				; Fetch the frame
		and ebx, 0xFFFFF000				; Mask out the frame address.
		or ebx, 0x3						; Set the appropriate flags
		mov [edi + (eax * 4)], ebx		; Store the page table in the directory
	.clear_page_table:
		mov edi, [ebp + 12]				; Fetch the frame address...
		and edi, 0xFFFFF000				; ...and mask to the boundary
		mov ecx, 0x400					; 1024 dword elements
		xor eax, eax
		rep stosd						; Clear the page table
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Allocate a new frame at the specified page if required. If a frame is 
;; specified then that frame will be assigned to the page.
;;
;;	void alloc_page(void *address, void *frame)
;;
_alloc_page:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] Page Number
		push 0 							; [ebp - 8] Page Table Number
	.check_frame:
		mov eax, [ebp + 12]				; Fetch the specified frame 
		test eax, eax					; Is it zero/null?
		jnz .calculate_page				; If no then assign the page if required
		push 0							; We don't need an identity mapped frame
		call _alloc_frame 				; Allocate a new frame
		add esp, 4
		mov [ebp + 12], eax				; Store the frame for later.
	.calculate_page:
		mov ebx, [ebp + 8]				; Fetch the linear address.
		mov ecx, ebx					; Copy in to ecx register as well
		shr ebx, 22						; Calculate the Page Table Number
		shr ecx, 12
		and ecx, 0x3FF					; Calculate the Page Number
		mov [ebp - 4], ecx				; Store the Page Number
		mov [ebp - 8], ebx				; Store the Page Table Number
	.check_page:
		mov edi, PAGE_DIR
		mov eax, [edi + (ebx * 4)]		; Fetch the Page Directory table entry.
		test eax, 0x1					; Is the Page Table present?
		jz .page_table_missing
		and eax, 0xFFFFF000				; Remove flags from Page Table address
		mov eax, [eax + (ecx * 4)]		; Fetch the Page Table page entry.
		test eax, 0x1					; Is the page present?
		jz .page_missing
		nop
		jmp .epilogue					; Go to the end, everything is present.
	.page_table_missing:
		push 1							; Indicate we need identity mapping
		call _alloc_frame 				; Allocate new physical frame
		add esp, 4
		push eax						; result is in eax, use as frame
		mov eax, [ebp - 8]				; Fetch the page table number
		push eax						; use as page_table
		call _alloc_page_table 			; Allocate the the page table.
		add esp, 8
	.page_missing:
		mov edi, PAGE_DIR
		mov ebx, [ebp - 8]				; Fetch the Page Table number
		mov eax, [edi + (ebx * 4)]		; Fetch the Page Directory table entry.
		and eax, 0xFFFFF000
		mov edi, eax
		mov ebx, [ebp - 4]				; Fetch the Page number
		mov eax, [ebp + 12]				; Fetch the frame
		and eax, 0xFFFFF000				; Make sure the address is aligned
		or eax, 0x3						; Mark it as present, read/write
		mov [edi + (ebx * 4)], eax		; Add the entry
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Find the next available physical frame. This will "allocate" the requested 
;; frame in the process, so the frame address should be correctly stored, and
;; handled by the caller, otherwise it will "lost".
;;
;;	void *alloc_frame(unsigned int identity_map)
;;
_alloc_frame:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] Frame address
	.main:
		mov esi, BOOT_CONFIG
		mov eax, [esi + BootConf.first_free_frame]
		mov [ebp - 4], eax
		add eax, 0x1000					; Move to the next physical frame
		mov [esi + BootConf.first_free_frame], eax
	.identity_map:
		mov eax, [ebp + 8]				; Fetch the identity_map flag
		test eax, eax					; Should we identity map the frame?
		jz .epilogue					; If no, then jump to the epilogue
		mov eax, [ebp - 4]				; Get the frame address
		push eax						; frame
		push eax						; address
		call _alloc_page 				; Allocate the page/frame
		add esp, 8
	.epilogue:
		mov eax, [ebp - 4]				; Fetch the frame address
		mov esp, ebp
		pop ebp
		ret

;;
;; Setup paging in the specified linear address range, starting at the 
;; specified frame.
;;
;;	void alloc_mem(void *base, unsigned int size, void *frame)
;;
_alloc_mem:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] Required page count
		push 0							; [ebp - 8] Linear Address to allocate
		push 0							; [ebp - 12] First physical frame
	.calculate_page_count:
		mov eax, [ebp + 8]				; Fetch the memory range base
		and eax, 0xFFF					; Only need to in page offset
		add eax, [ebp + 12]				; Add the memory range size
		mov ebx, eax
		shr eax, 12						; Get the number of pages required
		mov [ebp - 4], eax				; Save the page count
		test ebx, 0xFFF					; Are we perfectly on a page boundary?
		jz .prepare_allocation			; Yes, prepare to allocate pages
		add eax, 1						; Add another page to the count
		mov [ebp - 4], eax				; Save the ammended page count
	.prepare_allocation:
		mov eax, [ebp + 8]				; Fetch the linear base address
		mov [ebp - 8], eax
		mov eax, [ebp + 16]				; Fetch the physical frame
		mov [ebp - 12], eax
		mov ecx, [ebp - 4]				; Fetch the page count
		jmp .alloc
	.next_frame:
		mov eax, [ebp - 12]				; Fetch the frame
		test eax, eax					; If the frame is null then skip this
		jz .next_linear_address
		add eax, 0x1000					; Move to the next frame
		mov [ebp - 12], eax				; Save the updated frame
	.next_linear_address:
		mov eax, [ebp - 8]				; Fetch the linear address
		add eax, 0x1000					; Move to the next linear address
		mov [ebp - 8], eax				; Save the updated linear address
	.alloc:
		mov [ebp - 4], ecx				; Save the current page count
		mov eax, [ebp - 8]				; Fetch the linear address
		mov ebx, [ebp - 12]				; Fetch the frame
		push ebx
		push eax
		call _alloc_page 				; Allocate the page
		add esp, 8
		mov ecx, [ebp - 4]				; Restore the current page count
		loop .next_frame				; Move to the next frame/page
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
		push 0							; [ebp - 4] Page table count
		push 0							; [ebp - 8] Kernel Base
		push 0							; [ebp - 12] Kernel Size
	.main:
		push 0							; NULL frame
		mov esi, BOOT_CONFIG
		mov eax, [esi + BootConf.kernel_size]
		push eax						; Kernel Size
		mov eax, [esi + BootConf.kernel_base]
		push eax						; Kernel Base
		call _alloc_mem
		add esp, 12
	.done:
		push strings.done
		call _send_serial_bytes
		add esp, 4
		jmp .epilogue
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
		movzx eax, byte[esi + BootConf.vesa_mode]
		or eax, eax
		jz .unrequired
	.calculate_page_table_count:
		xor edx, edx
		mov eax, [esi + BootConf.lfb]	
		mov [ebp - 8], eax				; Store it locally for reference
		mov eax, [esi + BootConf.screen_size]
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
