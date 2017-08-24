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
	[org	0x7c00]

_boot:
	.entry:
		jmp short _start
		nop

; The BIOS Parameter Block contains the information that will be used by the
; BIOS to identity the disk and determine what it should do with it.
bios.parameter_block:
	.oem_id             db "MSWIN4.1"
	.bps                dw 512
	.spc                db 1
	.reserved_count     dw 51
	.fat_count          db 2
	.dir_entries        dw 224
	.total_sectors      dw 2880
	.media_descriptor   db 0xF8
	.spf                dw 9
	.spt                dw 18
	.head_count         dw 2
	.hidden_sectors     dd 0
	.unknown            dd 0
	.drive              db 0
	.dirty              db 1
	.extended_sig       db 0x29
	.volume_id          dd 77
	.volume_label       db "VeracyonOS "
	.fs_type            db "FAT12   "

; This is the main starting point of the system. We need to get the CPU into a
; known state and load the rest of the bootloader.
_start:
	.setup:
		cli
		xor ax, ax
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax
		mov sp, 0xFFFF
	.fdc_reset:
		xor dx, dx
		int 0x13
	.fdc_read:
		mov cx, [bios.parameter_block.reserved_count]
		dec cx
		mov bx, 0x7e00
		mov ax, 1
		call _read_sectors
	.start_stage2:
		jmp 0x0:0x7e00

; Read the specified number of sectors from disk.
_read_sectors:
	.L0:
		mov di, 5               ; Attempts
	.L1:
		pusha
		call _lbachs
		mov ah, 0x02
		mov al, 0x01
		mov ch, [.abs_track]
		mov cl, [.abs_sector]
		mov dh, [.abs_head]
		mov dl, [bios.parameter_block.drive]
		int 0x13
		jnc .successful_read
		dec di
		popa
		jnz .L1
		call _disk_read_error
	.successful_read:
		popa
		add bx, [bios.parameter_block.bps]
		inc ax
		loop .L0
		ret
	.abs_track:
		db 0
	.abs_sector:
		db 0
	.abs_head:
		db 0

_lbachs:
		xor dx, dx
		div word[bios.parameter_block.spt]
		inc dl
		mov [_read_sectors.abs_sector], dl
		xor dx, dx
		div word[bios.parameter_block.head_count]
		mov [_read_sectors.abs_head], dl
		mov [_read_sectors.abs_track], al
		ret

_disk_read_error:
		mov si, .message
		call _puts16
		mov ah, 0x0
		int 0x18
	.message:
		db 0xD, 0xA, 0x0, "Disk read error!", 0x0

_puts16:
		mov ah, 0x0e
	.next:
		lodsb
		or al, al
		jz .eos
		int 0x10
		jmp .next
	.eos:
		ret

        times 0x1FE-($-$$) db 0
        dw 0xAA55
