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
		mov edi, 0x12000				; Location of the root page directory
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
		mov eax, 0x12000
		mov cr3, eax
		mov eax, cr0
		or eax, 0x80000000
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
		mov edi, 0x13000
		mov ecx, 0x400
		xor eax, eax
		cld
		rep stosd
	.populate_page_table:
		mov edi, 0x13000
		mov ecx, 0x100					; 256 Entries (1MiB of memory)
		xor eax, eax
		mov al, 0x03
	.L0:
		stosd
		add eax, 0x1000 				; Next page
		loop .L0
	.register_page_table:
		mov edi, 0x12000				; Page Directory. First entry.
		mov eax, 0x13003				; 0x13000 | 0x03
		stosd
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

