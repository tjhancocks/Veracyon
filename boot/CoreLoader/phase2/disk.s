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

	[bits	32]

;;
;; Boot media types.
;;
	BOOT_MEDIA_UNKNOWN		equ 0
	BOOT_MEDIA_FDD			equ 1
	BOOT_MEDIA_HDD			equ 2

;;
;; Media descriptor strings.
;;
_media_descriptor_strings:
	.f0:
		db "[F0]  FDC  2.88MiB  3.5-inch, 2-sided, 36-sector", 0xA, 0x0
	.f0_1:
		db "[F0]  FDC  1.44MiB  3.5-inch, 2-sided, 18-sector", 0xA, 0x0
	.f9:
		db "[F9]  FDC  720KiB  3.5-inch, 2-sided, 9-sector", 0xA, 0x0
	.f9_1:
		db "[F9]  FDC  1.20MiB  5.25-inch, 2-sided, 15-sector", 0xA, 0x0
	.fd:
		db "[FD]  FDC  360KiB  5.25-inch, 2-sided, 9-sector", 0xA, 0x0
	.ff:
		db "[FF]  FDC  320KiB  5.25-inch, 2-sided, 8-sector", 0xA, 0x0
	.fc:
		db "[FC]  FDC  180KiB  5.25-inch, 1-sided, 9-sector", 0xA, 0x0
	.fe:
		db "[FE]  FDC  160KiB  5.25-inch, 1-sided, 8-sector", 0xA, 0x0
	.f8:
		db "[F8]  HDD  Fixed Disk", 0xA, 0x0
	.unknown:
		db "[??]  Unknown boot media.", 0xA, 0x0

;;
;; Identify the media of the boot drive.
;;
_identify_boot_media:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov esi, BIOS_PARAM_BLOCK
		movzx eax, byte[esi + BPBlock.media_descriptor]
		movzx ebx, word[esi + BPBlock.spt]
		cmp eax, 0xF0
		je .identify_floppy_disk_f0
		cmp eax, 0xF9
		je .identify_floppy_disk_f9
		cmp eax, 0xFD
		je .identify_floppy_disk_fd
		cmp eax, 0xFF
		je .identify_floppy_disk_ff
		cmp eax, 0xFC
		je .identify_floppy_disk_fc
		cmp eax, 0xFE
		je .identify_floppy_disk_fe
		cmp eax, 0xF8
		je .identify_fixed_disk_f8
	.unknown:
		push _media_descriptor_strings.unknown
		jmp .report_type
	.identify_floppy_disk_f0:
		cmp ebx, 36
		jne .f0_1
		push _media_descriptor_strings.f0
		jmp .handle_fdc
	.f0_1:
		cmp ebx, 18
		jne .unknown
		push _media_descriptor_strings.f0_1
		jmp .handle_fdc
	.identify_floppy_disk_f9:
		cmp ebx, 9
		jne .f9_1
		push _media_descriptor_strings.f9
		jmp .handle_fdc
	.f9_1:
		cmp ebx, 15
		jne .unknown
		push _media_descriptor_strings.f9_1
		jmp .handle_fdc
	.identify_floppy_disk_fd:
		push _media_descriptor_strings.fd
		jmp .handle_fdc
	.identify_floppy_disk_ff:
		push _media_descriptor_strings.ff
		jmp .handle_fdc
	.identify_floppy_disk_fc:
		push _media_descriptor_strings.fc
		jmp .handle_fdc
	.identify_floppy_disk_fe:
		push _media_descriptor_strings.fe
		jmp .handle_fdc
	.identify_fixed_disk_f8:
		push _media_descriptor_strings.f8
		jmp .report_type
	.handle_fdc:
		call _prepare_fdc
	.report_type:
		call _send_serial_bytes
		add esp, 4
		stc
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Read the specified sectors from the boot disk.
;;
;;	void disk_read_sectors(unsigned int sector,
;;						   unsigned int count,
;;						   void *dst)
;;
_disk_read_sectors:
	.prologue:
		push ebp
		mov ebp, esp
	.find_function:
		mov esi, DISK_INTERFACE
		mov eax, [esi + DiskInterface.read_sectors]
		test eax, eax
		jz .failed
		jmp eax
	.failed:
		push .missing
		call _send_serial_bytes
		cli
		hlt
		jmp $
	.missing:
		db "Disk read_sectors() function was not installed. Aborting.", 0xA, 0x0
