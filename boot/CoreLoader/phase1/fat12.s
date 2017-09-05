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

	[bits	16]

$FAT12:
	.cluster		dw 0

;;
;; Read file into the file buffer.
;; NOTE: This will clobber registers
;;
;; 	IN: SI => File name
;; OUT: ES:BX => File buffer
;;
fat12_read_file:
	.prologue:
		push bp
		mov bp, sp
		push si
	.load_root:
		xor ecx, ecx
		mov si, 0x7c00
		xor cx, cx
		xor dx, dx
		mov ax, 0x0020					; FAT uses 32 byte entries
		mul word[gs:si + 17]			; Multiply by root entries
		div word[gs:si + 11]			; Divide by bytes per sector
		xchg ax, cx
		mov al, byte[gs:si + 16]		; Number of FATs
		mul word[gs:si + 22]			; Sectors per FAT
		add ax, word[gs:si + 14]		; Reserved sectors
		mov word[$DISK.base_sector], ax	; Base of root directory
		add word[$DISK.base_sector], cx	; Add the size of it.
		xor bx, bx
		push ax
		mov ax, 0x2000
		mov es, ax						; ES:BX = 0x2000:0x0000 (0x20000)
		pop ax
		call disk_read_sectors			; Read the sectors from disk
	.find_file:
		mov cx, word[gs:si + 17]		; Fetch the number of root entries
		xor di, di						; Start at the beginning
	.L0:
		push cx
		mov cx, 11						; File names are 11 characters
		mov si, [bp - 2]				; Restore the file name
		push di
		rep cmpsb						; Test for equality
		pop di
		je .load_fat
		pop cx
		add di, 0x0020					; Move to the next entry.
		loop .L0
	.missing_file:
		mov si, strings16.missing
		call send_serial_bytes
		stc 							; Set the carry flag to indicate error
		mov sp, bp
		pop bp
		ret
	.load_fat:
		mov dx, word[es:di + 26]		; Fetch the first cluster of the file
		mov word[$FAT12.cluster], dx	; Store the cluster number
		mov si, 0x7c00
		movzx ax, byte[gs:si + 16]		; Number of FATs
		mul word[gs:si + 22]			; Sectors per FAT
		mov cx, ax
		mov ax, word[gs:si + 14]		; Reserved sectors
		xor bx, bx
		push bx
		call disk_read_sectors
	.load_file:
		pop bx
		mov ax, word[$FAT12.cluster]
		call cluster_to_lba				; Convert the cluster to LBA
		mov si, 0x7c00
		movzx cx, byte[gs:si + 13]		; Sectors per cluster
		push es
		push 0x3000
		pop es							; ES:BX = 0x3000:0x0000 (0x30000)
		call disk_read_sectors
		pop es
		push bx
		mov ax, word[$FAT12.cluster]	; Get the current cluster
		mov cx, ax
		mov dx, ax
		shr dx, 1						; Divide by 2
		add cx, dx						; = (3/2)
		xor bx, bx						; FAT location
		add bx, cx						; index into FAT
		mov dx, word[es:bx]				; Read 2 bytes from FAT
		test ax, 1						; Is it an odd or even entry?
		jnz .odd_cluster
	.even_cluster:
		and dx, 0x0FFF					; Lower twelve bits
		jmp .cluster_done
	.odd_cluster:
		shr dx, 4						; Upper twelve bits
	.cluster_done:
		mov word[$FAT12.cluster], dx	; Update the cluster
		cmp dx, 0x0FF0					; End of file?
		jb .load_file					; Next cluster from file?
	.file_done:
		push 0x3000
		pop es
		xor bx, bx
	.epilogue:
		mov sp, bp
		pop bp
		ret