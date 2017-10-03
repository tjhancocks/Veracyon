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
;; Prepare basic FAT12 File System Driver.
;;
;;	void prepare_fat12(void)
;;
_prepare_fat12:
	.prologue:
		push ebp
		mov ebp, esp
	.install_stubs:
		mov esi, FS_INTERFACE
		mov dword[esi + FileSystemInterface.file_read], _fat12_file_read.main
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Load the root directory of the boot disk.
;;
;;	void fat12_load_root(void)
;;
_fat12_load_root:
	.prologue:
		push ebp
		mov ebp, esp
		push FS_BUFFER					; [ebp - 4] destination
		push 0							; [ebp - 8] number of sectors
		push 0							; [ebp - 12] first sector
	.load_bpb:
		push .loading
		call _send_serial_bytes
		add esp, 4
		xor ecx, ecx
		xor edx, edx
		mov esi, BIOS_PARAM_BLOCK
		mov eax, 0x20					; FAT uses 32-byte entries
		mul word[esi + BPBlock.dir_entries]; Multiply by the root entry count
		div word[esi + BPBlock.bps]		; Divide by bytes per sector
		mov [ebp - 8], eax				; Store the number of sectors
		xchg eax, ecx
		movzx eax, byte[esi + BPBlock.fat_count]
		mul word[esi + BPBlock.spf]
		movzx ebx, word[esi + BPBlock.reserved_count]
		add eax, ebx
		mov [ebp - 12], eax				; Store the first sector of the root
		call _disk_read_sectors
		push .done
		call _send_serial_bytes
		add esp, 4
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.loading:
		db "Loading root directory from disk... ", 0xA, 0x0
	.done:
		db "Finished loading root directory!", 0xA, 0x0

;;
;; Load the FAT from the boot disk.
;;
;;	void fat12_load_fat(void)
;;
_fat12_load_fat:
	.prologue:
		push ebp
		mov ebp, esp
		push FS_BUFFER					; [ebp - 4] destination
		push 0							; [ebp - 8] number of sectors
		push 0							; [ebp - 12] first sector
	.main:
		push .loading
		call _send_serial_bytes
		add esp, 4
		mov esi, BIOS_PARAM_BLOCK
		movzx eax, byte[esi + BPBlock.fat_count]
		mul word[esi + BPBlock.spf]
		mov [ebp - 8], eax				; Store the number of sector
		movzx eax, word[esi + BPBlock.reserved_count]
		mov [ebp - 12], eax				; Store the first sector
		call _disk_read_sectors
		push .done
		call _send_serial_bytes
		add esp, 4
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.loading:
		db "Loading FAT from disk... ", 0xA, 0x0
	.done:
		db "Finished loading FAT!", 0xA, 0x0

;;
;; Convert the specified cluster to an LBA address.
;;
;;	unsigned int fat12_cluster_to_lba(unsigned int cluster);
;;
_fat12_cluster_to_lba:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] base sector
	.base_sector:
		mov esi, BIOS_PARAM_BLOCK
		movzx eax, byte[esi + BPBlock.fat_count]
		mul word[esi + BPBlock.spf]
		movzx ebx, word[esi + BPBlock.reserved_count]
		add eax, ebx
		mov [ebp - 4], eax				; Save the base sector
		mov eax, 0x0020					; FAT uses 32 byte entries
		mul word[esi + BPBlock.dir_entries]
		div word[esi + BPBlock.bps]
		add [ebp - 4], eax				; Add the root dir size to base
		mov eax, [ebp + 8]				; Fetch the cluster number
		sub eax, 2						; Zero based cluster number
		movzx ecx, byte[esi + BPBlock.spc]
		mul ecx
		add eax, [ebp - 4]				; Add the base sector
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Read the specified file from the current directory in the FAT12 file system,
;; of the boot disk.
;; NOTE: The current directory will always be the root directory in this basic
;; driver.
;;
;; 	void *fat12_file_read(const char *name)
;;
_fat12_file_read
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push 0							; [ebp - 4] File Record offset
		push 0							; [ebp - 8] First cluster
		push 0							; [ebp - 12] File Size
		push 0x10000000					; [ebp - 16] File Buffer
		push 0							; [ebp - 20] File Buffer Size (Pages)
		push 0							; [ebp - 24] First sector of file
		mov esi, BIOS_PARAM_BLOCK
	.find_file:
		call _fat12_load_root
		mov esi, BIOS_PARAM_BLOCK
		movzx ecx, word[esi + BPBlock.dir_entries]; Fetch the number of entries
		mov edi, FS_BUFFER
	.L0:
		push ecx
		mov ecx, 11						; FAT12 has 8:3 file names
		mov esi, [ebp + 8]				; Fetch the file name to search for
		push edi
		rep cmpsb						; Test for equality
		pop edi
		je .file_found
		pop ecx
		add edi, 0x20					; Move to the next record
		loop .L0
	.missing_file:
		stc
		jmp .epilogue
	.file_found:
		pop ecx
		mov [ebp - 4], edi				; Save the offset to the file record
		movzx edx, word[edi + 26]		; Fetch the first cluster of the file
		mov [ebp - 8], edx				; Save the first cluster
		mov eax, [edi + 28]				; Fetch the size of the file in bytes
		mov [ebp - 12], eax				; Save the size of the file in bytes
	.calculate_buffer_pages:
		xor edx, edx
		mov ebx, 0x1000
		div ebx
		test edx, edx
		jz .alloc_buffer_pages
		add eax, 1
	.alloc_buffer_pages:
		mov [ebp - 20], eax
		push 0							; Not identity mapped
		push dword[ebp - 12]			; The size of the file in bytes
		push dword[ebp - 16]			; Load the file to the 256MiB point
		call _alloc_memory
		add esp, 12
	.load_fat:
		call _fat12_load_fat 			; Fetch the FAT from disk.
	.load_file:
		push dword[ebp - 8]				; The next cluster of the file
		call _fat12_cluster_to_lba		; Convert the cluster number to LBA
		add esp, 4
		mov [ebp - 24], eax				; Save the first of the file
		mov esi, BIOS_PARAM_BLOCK
		movzx ecx, byte[esi + BPBlock.spc]
		push dword[ebp - 16]			; File buffer
		push ecx						; Read all the sectors of the cluster
		push dword[ebp - 24]			; The first sector of the cluster
		call _disk_read_sectors
		add esp, 12
	.update_buffer:
		mov esi, BIOS_PARAM_BLOCK
		movzx ecx, byte[esi + BPBlock.spc]
		movzx eax, word[esi + BPBlock.bps]
		mul ecx
		add eax, [ebp - 16]				; Increment the file buffer by 1 cluster
		mov [ebp - 16], eax				; Save it
	.next_cluster:
		mov eax, [ebp - 8]				; Fetch the current cluster number
		mov ecx, eax
		mov edx, eax
		shr edx, 1						; Divide by 2
		add ecx, edx					; =(3/2)
		mov esi, FS_BUFFER				; Current location of FAT
		movzx edx, word[esi + ecx]		; Read 2 bytes from the FAT
		test eax, 0x1					; Is it an even or odd entry?
		jnz .odd_cluster
	.even_cluster:
		and edx, 0x0FFF
		jmp .cluster_done
	.odd_cluster:
		shr edx, 4
	.cluster_done:
		mov [ebp - 8], edx				; Update the next cluster of the file
		cmp edx, 0x0FF0					; End of File?
		jb .load_file
	.epilogue:
		push .loaded
		call _send_serial_bytes
		mov esp, ebp
		pop ebp
		ret
	.loaded:
		db "Finished loading the file...", 0xA, 0x0
