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

	[bits	16]

;;
;; Disk driver globals. Used for temporary values that are being used to read
;; sectors.
;;
$DISK:
	.absolute_track		db 0
	.absolute_sector	db 0
	.absolute_head		db 0
	.base_sector		dw 0

;;
;; Read the specified number of sectors from the boot disk. This relies on
;; BIOS and should be agnostic of a specific disk media.
;; NOTE: This will clobber registers.
;;
;;	IN: AX => Starting sector
;; 	IN: CX => Number of sectors
;; OUT: ES:BX => Read buffer
;;
disk_read_sectors:
	.prologue:
		push bp
		mov bp, sp
		push gs
		push 0
		pop gs
	.next_sector:
		mov di, 5						; Make 5 attempts to read the sector
	.L0:
		push ax
		push bx
		push cx
		call lba_to_chs
		mov si, BIOS_PARAM_BLOCK		; Bootsector location
		mov ax, 0x0201					; BIOS read one sector
		mov ch, byte[$DISK.absolute_track]
		mov cl, byte[$DISK.absolute_sector]
		mov dh, byte[$DISK.absolute_head]
		mov dl, byte[gs:si + BPBlock.drive]; Fetch the boot drive number
		int 0x13						; Invoke the BIOS function
		jnc .success					; If no read error, then jump to success
		xor ax, ax						; Reset the disk
		int 0x13
		dec di 							; Decrement the retry counter
		pop cx
		pop bx
		pop ax
		jnz .L0							; Attempt to read again
		int 0x18						; Report boot failure.
	.success:
		pop cx
		pop bx
		pop ax
		mov si, BIOS_PARAM_BLOCK
		add bx, word[gs:si + BPBlock.bps]; Move to next buffer
		inc ax							; Move to the next sector
		loop .next_sector
	.epilogue:
		pop gs
		mov sp, bp
		pop bp
		ret

;;
;; Convert a Logical Block Address to a Cylinder-Head-Sector address.
;; NOTE: This will clobber registers.
;;
;;	IN: AX => LBA Address
;;
lba_to_chs:
	.prologue:
		push bp
		mov bp, sp
		push gs
		push 0
		pop gs
	.main:
		mov si, BIOS_PARAM_BLOCK 		; Bootsector
		xor dx, dx
		div word[gs:si + BPBlock.spt]	; Divide by sectors per track
		inc dl							; First sector is sector 1, not 0
		mov byte[$DISK.absolute_sector], dl
		xor dx, dx
		div word[gs:si + BPBlock.head_count]; Divide by heads per cylinder
		mov byte[$DISK.absolute_head], dl
		mov byte[$DISK.absolute_track], al
	.epilogue:
		pop gs
		mov sp, bp
		pop bp
		ret

;;
;; Convert the specified cluster to an LBA address.
;; NOTE: This will clobber registers.
;;
;;	IN: AX => Cluster
;; OUT: AX => LBA
;;
cluster_to_lba:
	.prologue:
		push bp
		mov bp, sp
		push gs
		push 0
		pop gs
	.main:
		mov si, BIOS_PARAM_BLOCK
		sub ax, 2						; Zero based cluster number
		xor cx, cx
		mov cl, byte[gs:si + BPBlock.spc]; Sectors per cluster
		mul cx
		add ax, word[$DISK.base_sector]
	.epilogue:
		pop gs
		mov sp, bp
		pop bp
		ret
