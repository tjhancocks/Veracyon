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

; Disk Driver globals. These are used for configuring the disk driver read
; operations.
$DISK:
	.abs_track:
		db 0x00
	.abs_sector:
		db 0x00
	.abs_head:
		db 0x00
	.base:
		dw 0x0000

; Read the specified number of sectors from the boot disk. This relies
; on the BIOS and should be agnostic of specific disk media.
;
;	IN:    AX => Starting Sector
;	IN:    CX => Number of Sectors
;  OUT: ES:BX => Read Buffer
;
; NOTE: This will clobber registers
disk.read_sectors:
	.prologue:
		push bp
		mov bp, sp
		push gs
		push 0
		pop gs
	.next_sector:
		mov di, 5						; Maximum of five attempts to read
	.L0:
		push ax
		push bx
		push cx
		call disk.lba_to_chs 
		mov si, BPB_ADDR
		mov ax, 0x0201					; Read just one sector at a time
		mov ch, byte[$DISK.abs_track]
		mov cl, byte[$DISK.abs_sector]
		mov dh, byte[$DISK.abs_head]
		mov dl, byte[gs:si + EBPB.drive]; What type of media are we booted from?
		int 0x13						; Perform disk operation...
		jnc .success					; If carry is not set, then successful
		xor ax, ax
		int 0x13						; Reset the disk
		dec di
		pop cx
		pop bx
		pop ax
		jnz .L0							; Next attempt, if we have more attempts
		int 0x18						; Soft reset the computer.
	.success:
		pop cx
		pop bx
		pop ax
		mov si, BPB_ADDR
		add bx, word[gs:si + BPB.bps]	; Move the write pointer forward by 1
		inc ax							; sector.
		loop .next_sector
	.epilogue:
		pop gs
		mov sp, bp
		pop bp
		ret

; Convert a logical block address (LBA) to a Cylinder Head Sector (CHS)
; address.
;
;	IN: AX => LBA Address
;
; NOTE: This will clobber registers.
disk.lba_to_chs:
	.prologue:
		push bp
		mov bp, sp
		push gs
		push 0
		pop gs
	.main:
		mov si, BPB_ADDR
		xor dx, dx
		div word[gs:si + BPB.spt]		; Divide LBA by sectors per track
		inc dl							; First sector is 1, not 0
		mov byte[$DISK.abs_sector], dl
		xor dx, dx
		div word[gs:si + BPB.heads]		; Divide result by number of heads
		mov byte[$DISK.abs_head], dl
		mov byte[$DISK.abs_track], al
	.epilogue:
		pop gs
		mov sp, bp
		pop bp
		ret
