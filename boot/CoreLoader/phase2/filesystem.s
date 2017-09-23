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
;; File System names
;;
_file_system_names:
	.fat12:
		db "[10]  FAT12", 0xA, 0x0
	.fat16:
		db "[11]  FAT16  (unsupported)", 0xA, 0x0
	.fat32:
		db "[12]  FAT32  (unsupported)", 0xA, 0x0
	.ntfs:
		db "[20]  NTFS   (unsupported)", 0xA, 0x0
	.ext2:
		db "[30]  ext2   (unsupported)", 0xA, 0x0
	.ext3:
		db "[31]  ext3   (unsupported)", 0xA, 0x0
	.ext4:
		db "[32]  ext4   (unsupported)", 0xA, 0x0
	.unknown:
		db "[32]  Unknown file system.", 0xA, 0x0

;;
;; Identify the type of file system on the boot disk.
;;
_identify_file_system:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov esi, BOOT_CONFIG
		movzx eax, byte[esi + BootConf.filesystem]
		cmp eax, FILE_SYSTEM_FAT12
		je .fat12
		cmp eax, FILE_SYSTEM_FAT16
		je .fat16
		cmp eax, FILE_SYSTEM_FAT32
		je .fat32
		cmp eax, FILE_SYSTEM_NTFS
		je .ntfs
		cmp eax, FILE_SYSTEM_EXT2
		je .ext2
		cmp eax, FILE_SYSTEM_EXT3
		je .ext3
		cmp eax, FILE_SYSTEM_EXT4
		je .ext4
		nop
		jmp .unknown
	.fat12:
		call _prepare_fat12
		push _file_system_names.fat12
		jmp .supported
	.fat16:
		push _file_system_names.fat16
		jmp .unsupported
	.fat32:
		push _file_system_names.fat32
		jmp .unsupported
	.ntfs:
		push _file_system_names.ntfs
		jmp .unsupported
	.ext2:
		push _file_system_names.ext2
		jmp .unsupported
	.ext3:
		push _file_system_names.ext3
		jmp .unsupported
	.ext4:
		push _file_system_names.ext4
		jmp .unsupported
	.unknown:
		push _file_system_names.unknown
	.unsupported:
		call _send_serial_bytes
		add esp, 4
		cli
		hlt
		jmp $
	.supported:
		call _send_serial_bytes
		add esp, 4
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Read the specified file from the current (root) directory of the file system.
;;
;;	void file_read(const char *name, void *dst)
;;
_file_read:
	.prologue:
		push ebp
		mov ebp, esp
	.find_function:
		mov esi, FS_INTERFACE
		mov eax, [esi + FileSystemInterface.file_read]
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
		db "File System read_file() function was not installed. Aborting."
		db 0xA, 0x0