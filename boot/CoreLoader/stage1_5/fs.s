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

	BITS 	16

	%include "CoreLoader/stage1_5/fat12.s"
	%include "CoreLoader/stage1_5/disk.s"

; Concrete File System pointers. The concrete file system should assign 
; appropriate sub-routine pointer values to these so that the virtual file 
; system can access the disk.
fs_driver:
	.read_file: 
		dw 0x0000

; Initialise the Virtual File System. This will check the file system of the
; boot disk and load the appropriate concrete file system driver into the
; interface.
vfs.init:
	.prologue:
		pusha
	.main:
		push BC_SEG
		pop fs
		mov si, .init_vfs_1
		call rs232.send_bytes
		mov si, BC_OFFSET
		movzx si, byte[fs:si + BootConf.fs_type]
		push si
		shl si, 1
		mov si, word[si + .fs_name_lookup]
		call rs232.send_bytes
		pop ax
		cmp ax, FS_FAT12
		je .install_fat12
		mov si, .unsupported_fs
		call rs232.send_bytes
		cli
		hlt
	.install_fat12:
		mov word[fs_driver.read_file], fat12.read_file
		jmp .epilogue
	.epilogue:
		popa
		ret
	.init_vfs_1:
		db "Initialising Virtual File System... ", 0x0
	.fs_name_lookup:
		dw .unknown, .fat12_name, .fat32_name
	.unsupported_fs:
		db "Unsupported File System.", 0xD, 0x0
	.unknown:
		db "Unknown.", 0xD, 0x0
	.fat12_name:
		db "FAT12.", 0xD, 0x0
	.fat32_name:
		db "FAT32.", 0xD, 0x0

; Debug dumping of the file over RS232. This is not recommended for _large_
; files.
;
;  IN: ES:BX => File Buffer
;
vfs.dump_file:
	.prologue:
		pusha
		push ds
		push es
		push bx
	.main:
		pop si
		pop ds
		call rs232.send_bytes
		mov si, .nl
		call rs232.send_bytes
	.epilogue:
		pop ds
		popa
		ret
	.nl:
		db 0xD, 0x0

; Read the specified file from the root directory into the specified
; file buffer
;
; 	IN:	   SI => File Name
;  OUT: ES:BX => File Buffer
;
; This _should_ set the carry flag if the file could not be found.
; NOTE: This will likely clobber registers.
vfs.read_file:
	.main:
		movzx eax, word[fs_driver.read_file]
		or eax, eax
		jz .missing
		jmp eax
  	.missing:
		stc
		ret
