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

; Read the specified file from the root directory into the specified
; file buffer
;
; 	IN:	   SI => File Name
;  OUT: ES:BX => File Buffer
;
; This will set the carry flag if the file could not be found.
; This will clobber registers
fat12.read_file:
	.prologue:
		push bp
		mov bp, sp
		push si
	.load_root:
		xor ecx, ecx
		xor edx, edx
		mov si, word BPB_ADDR				; We need to access BPB values
		mov ax, 32							; FAT uses 32 byte directory entries
		mul word[gs:si + BPB.root_ents]		; * root directory entries
		div word[gs:si + BPB.bps]			; / bytes per sector
		xchg ax, cx							; Clear AX and set the sector count
		movzx ax, byte[gs:si + BPB.fats]	; The number of FATs in volume
		mul word[gs:si + BPB.spf]			; * sectors per FAT
		add ax, word[gs:si + BPB.reserved]	; + reserved sectors
		mov word[$DISK.base], ax			; Save the base of the root dir
		add word[$DISK.base], cx			; + the size of the root dir
		push ax
		mov ax, TSB_SEG						; Prepare the Temporary Storage
		mov bx, TSB_OFFSET					; buffer.
		mov es, ax							; ES:BX = TSB_SEG:TSB_OFFSET
		pop ax
		call disk.read_sectors
	.find_file:
		mov cx, word[gs:si + BPB.root_ents]	; Fetch the number of root entries
		xor di, di
	.L0:
		push cx
		mov cx, 11							; File names are 11 bytes long...
		mov si, [bp - 2]					; Restore the file name from stack
		push di
		rep cmpsb							; Test for equivalence...
		pop di
		je .load_fat
		pop cx
		add di, 32							; Advance to the next entry
		loop .L0
	.missing_file:							; We failed to find the file...
		mov si, .missing_str
		call rs232.send_bytes
		mov si, [bp - 2]
		call rs232.send_bytes
		stc
		mov sp, bp
		pop bp
		ret
	.load_fat:
		mov dx, word[es:di + 26]			; First cluster in the file
		mov word[bp - 2], dx				; Save the cluster
		mov si, BPB_ADDR
		movzx ax, byte[gs:si + BPB.fats]	; Get the number of FATs.
		mul word[gs:si + BPB.spf]			; * sectors per FAT.
		mov cx, ax							; CX = sector count
		mov ax, word[gs:si + BPB.reserved]	; + Reserved sector count
		xor bx, bx
		push bx								; ES:BX = ES:0000, start of buffer
		call disk.read_sectors				; Read the FAT.
	.load_file:
		pop bx
		mov ax, word[bp - 2]				; Get the next cluster number
		call fat12.cluster_to_lba			; What is the LBA of it?
		mov si, BPB_ADDR
		movzx cx, byte[gs:si + BPB.spc]		; Number of sectors in a cluster
		push es
		push FILE_SEG						; We want to read the file in to the
		pop es								; file segment
		call disk.read_sectors
		pop es
		push bx
		mov ax, word[bp - 2]				; Fetch the current cluster
		mov cx, ax
		mov dx, ax
		shr dx, 1							; / 2
		add cx, dx							; = (3/2)
		xor bx, bx							; FAT Location
		add bx, cx							; Index into FAT
		mov dx, word[es:bx]					; Read 2 bytes from fat12.read_file
		test ax, 1							; Is it odd or even entry?
		jnz .odd_cluster
	.even_cluster:
		and dx, 0x0FFF
		jmp .cluster_done
	.odd_cluster:
		shr dx, 4
	.cluster_done:
		mov word[bp - 2], dx				; Update the cluster number
		cmp dx, 0x0FF0
		jb .load_file
	.file_done:
		push FILE_SEG
		pop es
		xor bx, bx
	.epilogue:
		mov sp, bp
		pop bp
		ret
	.missing_str:
		db "File not found on disk: ", 0x0

; Convert the specified cluster number to an LBA address.
;
;	IN:	AX => Cluster
;  OUT: AX => LBA
;
fat12.cluster_to_lba:
	.prologue:
		push bp
		mov bp, sp
		push gs
		push 0
		pop gs
	.main:
		mov si, BPB_ADDR
		sub ax, 2
		xor cx, cx
		mov cl, byte[gs:si + BPB.spc]
		mul cx
		add ax, word[$DISK.base]
	.epilogue:
		pop gs
		mov sp, bp
		pop bp
		ret

