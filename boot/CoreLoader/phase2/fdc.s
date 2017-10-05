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

	[bits 	32]

;;
;; Floppy Disk Controller constants
;;
	FDC_BASE				equ 0x03f0
	FDC_REG_A				equ FDC_BASE
	FDC_REG_B				equ FDC_BASE | 0x1
	FDC_DOR					equ FDC_BASE | 0x2
	FDC_MSR					equ FDC_BASE | 0x4
	FDC_FIFO				equ FDC_BASE | 0x5
	FDC_CCR					equ FDC_BASE | 0x7
	FDC_CMD_SPECIFY			equ 0x3
	FDC_CMD_WRITE			equ 0x5
	FDC_CMD_READ			equ 0x6
	FDC_CMD_RECALIBRATE		equ 0x7
	FDC_CMD_SENSE			equ 0x8
	FDC_CMD_SEEK			equ 0xF
	FDC_MOTOR_OFF			equ 0x0
	FDC_MOTOR_ON			equ 0x1
	FDC_MOTOR_WAIT			equ 0x2
	FDC_DIRECTION_READ		equ 0x1
	FDC_DIRECTION_WRITE		equ 0x2

;;
;; Setup the Floppy Disk Controller driver.
;;
;;	void prepare_fdc(void)
;;
_prepare_fdc:
	.prologue:
		push ebp
		mov ebp, esp
	.irq_handler:
		push _fdc_irq_handler
		push 6 + 0x20
		call _register_hardware_interrupt_handler
		add esp, 8
	.install_stubs:
		mov esi, DISK_INTERFACE
		mov dword[esi + DiskInterface.read_sectors], _fdc_read_sectors.main
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; The Floppy Disk Controller interrupt handler is a naked function and should
;; not be called directly.
;;
_fdc_irq_handler:
	.main:
		mov esi, DISK_DRIVER
		mov byte[esi + FDCData.irq_received], 1
		ret

;;
;; Wait for the Floppy Disk Controller to issue an IRQ.
;;
;;	void fdc_wait(void)
;;
_fdc_wait:
	.prologue:
		push ebp
		mov ebp, esp
		push fdc_string.wait_1
		call _send_serial_bytes
		add esp, 4
	.detect_bochs:
		in al, 0xE9
		cmp al, 0xE9
		je .skip
		push eax
	.L0:
		push 500						; 500 Milliseconds
		call _sleep
		add esp, 4
		mov esi, DISK_DRIVER
		movzx eax, byte[esi + FDCData.irq_received]
		test eax, eax
		jnz .finish
		jmp .L0
	.skip:
		push eax
		push 300
		call _sleep
		add esp, 4
	.finish:
		push fdc_string.done
		call _send_serial_bytes
		add esp, 4
		mov esi, DISK_DRIVER
		mov byte[esi + FDCData.irq_received], 0
		pop eax
		mov esp, ebp
		pop ebp
		ret

;;
;; Send the specified command to the Floppy Disk Controller.
;;
;;	void fdc_send_cmd(unsigned char cmd)
;;
_fdc_send_cmd:
	.prologue:
		push ebp
		mov ebp, esp
		pushad
	.main:
		mov ecx, 600					; 30 Seconds (600 attempts)
		mov eax, [ebp + 8]				; Fetch the command
	.L0:
		push ecx
		push 10							; Wait for 10ms
		call _sleep
		add esp, 4
		mov edx, FDC_MSR
		in al, dx
		test al, 0x80					; Is bit 7 of the result set?
		jnz .send
		pop ecx
		loop .L0
	.timeout:
		push .timeout_message
		call _send_serial_bytes
		cli
		hlt
	.send:
		pop ecx
		mov edx, FDC_FIFO
		mov eax, [ebp + 8]
		out dx, al
	.epilogue:
		popad
		mov esp, ebp
		pop ebp
		ret
	.timeout_message:
		db "Floppy Disk Controller timed out sending command.", 0xA
		db "Halting system.", 0x0

;;
;; Read a single byte from the Floppy Disk Controller. This is used for 
;; receiving the return value of a command, not for reading data from the disk.
;;
;;	unsigned char fdc_read_byte(void)
;;
_fdc_read_byte:
	.prologue:
		push ebp
		mov ebp, esp
		pushad
	.main:
		mov ecx, 600					; 30 second timeout (600 attempts)
	.L0:
		push ecx
		push 10							; Wait for 10ms
		call _sleep
		add esp, 4
		mov edx, FDC_MSR
		xor eax, eax
		in al, dx
		test al, 0x80					; Is bit 7 of the result set?
		jnz .read
		pop ecx
		loop .L0
	.timeout:
		push .timeout_message
		call _send_serial_bytes
		cli
		hlt
	.read:
		pop ecx
		mov edx, FDC_FIFO
		in al, dx
		mov esi, DISK_DRIVER
		mov byte[esi + FDCData.cmd_result], al
	.epilogue:
		popad
		push esi
		mov esi, DISK_DRIVER
		movzx eax, byte[esi + FDCData.cmd_result]
		pop esi
		mov esp, ebp
		pop ebp
		ret
	.timeout_message:
		db "Floppy Disk Controller timed out reading command result.", 0xA
		db "Halting system.", 0x0

;;
;; Check the interrupt status in the Floppy Disk Controller.
;;
;;	void fdc_sense(unsigned char *st0, unsigned char *cyl)
;;
_fdc_sense:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push FDC_CMD_SENSE
		call _fdc_send_cmd
		add esp, 4
		call _fdc_read_byte
		mov [ebp + 8], eax				; *st0 = eax
		call _fdc_read_byte
		mov [ebp + 12], eax				; *cyl = eax
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Calibrate the primary Floppy Disk Drive.
;;
;;	void fdc_calibrate(void)
;;
_fdc_calibrate:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		call _fdc_motor_on
		mov ecx, 10						; Make 10 attempts to calibrate
	.L0:
		push ecx
		push FDC_CMD_RECALIBRATE
		call _fdc_send_cmd
		mov dword[esp], 0
		call _fdc_send_cmd
		push 100
		call _sleep
		add esp, 8
		call _fdc_wait
		push 0							; Reserve space for cyl
		push 0							; Reserve space for st0
		call _fdc_sense
		mov eax, [esp]					; EAX = st0
		test eax, 0xC0
		jnz .status
		mov eax, [esp + 4]				; EAX = cyl
		add esp, 8
		cmp eax, 0
		je .found_sector_zero
	.next_attempt:
		pop ecx
		loop .L0
		jmp .failed
	.status:
		add esp, 8
		nop
		; ... Maybe add something for reporting status here ...
		jmp .next_attempt
	.found_sector_zero:
		pop ecx
		call _fdc_motor_off
		mov eax, 1
		mov esp, ebp
		pop ebp
		ret
	.failed:
		call _fdc_motor_off
		push .failed_message
		call _send_serial_bytes
		xor eax, eax
		mov esp, ebp
		pop ebp
		ret
	.failed_message:
		db "Floppy Disk Drive calibration failed.", 0xA, 0x0

;;
;; Reset the primary Floppy Disk Drive.
;;
;;	void fdc_reset(void)
;;
_fdc_reset:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push fdc_string.reset_1
		call _send_serial_bytes
		add esp, 4
		mov esi, DISK_DRIVER
		mov byte[esi + FDCData.irq_received], 0
		push fdc_string.disabling
		call _send_serial_bytes
		add esp, 4
		mov edx, FDC_DOR
		xor eax, eax
		out dx, al						; Disable Controller
		push fdc_string.enabling
		call _send_serial_bytes
		add esp, 4
		mov eax, 0x0C
		out dx, al						; Enable Controller
		push fdc_string.enabled
		call _send_serial_bytes
		add esp, 4
		call _fdc_wait
		push 0
		push 0
		call _fdc_sense
		add esp, 8
	.set_speed:
		push fdc_string.reset_2
		call _send_serial_bytes
		add esp, 4
		mov edx, FDC_CCR
		xor eax, eax
		out dx, al
		push FDC_CMD_SPECIFY
		call _fdc_send_cmd
		add esp, 4
		push 0xdf
		call _fdc_send_cmd
		add esp, 4
		push 0x02
		call _fdc_send_cmd
		add esp, 4
	.check_failure:
		call _fdc_calibrate
		push eax
		push fdc_string.reset_3
		call _send_serial_bytes
		add esp, 4
		pop eax
		test eax, eax
		jz .failed
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.failed:
		push .failed_message
		call _send_serial_bytes
		cli
		hlt
	.failed_message:
		db "Floppy Disk Drive reset failed. Halting system.", 0xA, 0x0

;;
;; Turn on the Floppy Disk Motor if it is not already powered on.
;;
;;	void fdc_motor_on(void)
;;
_fdc_motor_on:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		mov esi, DISK_DRIVER
		movzx eax, byte[esi + FDCData.state]
		test eax, eax
		jnz .L1
		mov edx, FDC_DOR
		mov eax, 0x1C
		out dx, al
		push 500
		call _sleep
		add esp, 4
	.L1:
		push .powering_on
		call _send_serial_bytes
		add esp, 4
		mov esi, DISK_DRIVER
		mov byte[esi + FDCData.state], FDC_MOTOR_ON
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.powering_on:
		db "Powering Floppy Disk Drive motor on", 0xA, 0x0

;;
;; Turn off the Floppy Disk Motor if it is not already turned off.
;;
;;	void fdc_motor_off(void)
;;
_fdc_motor_off:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .powering_off
		call _send_serial_bytes
		add esp, 4
		mov esi, DISK_DRIVER
		mov dword[esi + FDCData.ticks], 300
		mov byte[esi + FDCData.state], FDC_MOTOR_WAIT
		call _fdc_timer
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.powering_off:
		db "Preparing to power Floppy Disk Drive motor off", 0xA, 0x0

;;
;; Kill the Floppy Disk Driver motor.
;;
;;	void fdc_motor_kill(void)
;;
_fdc_motor_kill:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push .killed
		call _send_serial_bytes
		add esp, 4
		mov edx, FDC_DOR
		mov eax, 0x0c
		out dx, al
		mov esi, DISK_DRIVER
		mov byte[esi + FDCData.state], FDC_MOTOR_OFF
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.killed:
		db "Killed floppy disk drive motor.", 0xA, 0x0

;;
;; Suspend the main thread until the Floppy Disk Controller timer tick counter
;; has reached 0.
;;
;; 	void fdc_timer(void)
;;
_fdc_timer:
	.prologue:
		push ebp
		mov ebp, esp
	.L0:
		push 500						; Wait for 500ms
		call _sleep
		add esp, 4
		mov esi, DISK_DRIVER
		movzx eax, byte[esi + FDCData.state]
		cmp eax, FDC_MOTOR_WAIT
		je .L1
		jmp .epilogue
	.L1:
		mov eax, [esi + FDCData.ticks]
		sub eax, 50
		mov dword[esi + FDCData.ticks], eax
		test eax, eax
		jz .L2
		jmp .L0
	.L2:
		call _fdc_motor_kill
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Seek the specified head of the Floppy Disk Drive to the specified cylinder.
;;
;;	void fdc_seek(unsigned short head, unsigned short cylinder)
;;
_fdc_seek:
	.prologue:
		push ebp
		mov ebp, esp
	.motor_on:
		call _fdc_motor_on
		mov ecx, 10						; Attempt 10 retries
	.L0:
		push ecx
		push FDC_CMD_SEEK
		call _fdc_send_cmd
		mov eax, [ebp + 8]				; EAX = head
		shl eax, 2						; (head << 2)
		mov [esp], eax
		call _fdc_send_cmd
		mov eax, [ebp + 12]				; EAX = cylinder
		mov [esp], eax
		call _fdc_send_cmd
		add esp, 4
		call _fdc_wait
		push 0							; Reserve space for cyl
		push 0							; Reserve space for st0
		call _fdc_sense
		mov ebx, [esp + 4]				; EBX = cyl
		add esp, 8
		mov eax, [ebp + 12]				; EAX = cylinder
		cmp eax, ebx
		je .epilogue
		pop ecx
		loop .L0
	.timeout:
		push .timeout_message
		call _send_serial_bytes
		cli
		hlt
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.timeout_message:
		db "Floppy Disk Drive seek timed out. Halting system.", 0xA, 0x0

;;
;; Initialise DMA Read Mode for the Floppy Disk Controller.
;;
;;	void fdc_read_dma_init(void)
;;
_fdc_read_dma_init:
	.prologue:
		push ebp
		mov ebp, esp
	.set_mode:
		; 01:0:0:01:10 = single/inc/no-auto/from-mem/chan2
		mov eax, 01000110b
	.apply:
		mov esi, DISK_DRIVER
		mov [esi + FDCData.mode], al
		mov eax, 0x06
		out 0x0a, al					; Mask channel 2
		mov eax, 0xff
		out 0x0c, al					; Reset Flip-Flop
		mov eax, DMA_BUFFER
		out 0x04, al					; Address Low Byte
		shr eax, 8
		out 0x04, al					; Address High Byte
		shr eax, 8
		out 0x81, al					; Externel Page Register
		mov al, 0xff
		out 0x0c, al					; Reset Flip-Flop
		mov eax, 0x47FF
		out 0x05, al					; Count Low Byte
		shr eax, 8
		out 0x05, al					; Count High Byte
		mov esi, DISK_DRIVER
		mov al, [esi + FDCData.mode]
		out 0x0b, al					; Set the Mode
		mov al, 0x02
		out 0x0a, al					; Unmask Channel 2
	.epilogue:
		mov esp, ebp
		pop ebp
		ret

;;
;; Do a full cylinder read of both tracks for the specified cylinder.
;;
;;	void fdc_read_cylinder(unsigned short cylinder)
;;
_fdc_read_cylinder:
	.prologue:
		push ebp
		mov ebp, esp
		push 0							; [ebp - 4] cylinder
		push 0							; [ebp - 8] command
		push 0							; [ebp - 12] st0
		push 0							; [ebp - 16] st1
		push 0							; [ebp - 20] st2
		push 0							; [ebp - 24] rcy
		push 0							; [ebp - 28] rhe
		push 0							; [ebp - 32] rse
		push 0							; [ebp - 36] bps
	.prepare:
		call _fdc_reset
		mov eax, [ebp + 8]
		mov [ebp - 4], eax
		push fdc_string.read_cyl_1
		call _send_serial_bytes
		add esp, 4
	.construct_command:
		; Read is MT:MF:SK:0:0:1:1:0
		; where MT = multitrack, MF = MFM mode, SK = skip deleted
		; Specify multitrack and MFM mode
		mov eax, FDC_CMD_READ
		or eax, 0xc0
		mov [ebp - 8], eax
	.seek:
		push fdc_string.read_cyl_2
		call _send_serial_bytes
		add esp, 4
		mov eax, [ebp + 8]
		push eax
		push 0							; Head 0 & specified cylinder
		call _fdc_seek
		add esp, 8
		mov eax, [ebp + 8]				; EAX = cylinder
		push eax
		push 1							; Head 1 & specified cylinder
		call _fdc_seek
		add esp, 8
	.seek_done:
		push fdc_string.read_cyl_3
		call _send_serial_bytes
		add esp, 4
		mov ecx, 20						; Attempt 20 retries
	.L0:
		push ecx
		call _fdc_motor_on
		call _fdc_read_dma_init
		push 500						; 500ms for heads to settle after seek
		call _sleep
		add esp, 4
	.issue_commands:
		mov eax, [ebp - 4]				; Fetch cylinder
		push eax
		mov eax, [ebp - 8]				; Fetch command
		push eax
		call _fdc_transfer_commands
		add esp, 8
		call _fdc_wait
	.read_results:
		call _fdc_read_byte
		mov [ebp - 12], eax				; Store st0
		call _fdc_read_byte
		mov [ebp - 16], eax				; Store st1
		call _fdc_read_byte
		mov [ebp - 20], eax				; Store st2
		call _fdc_read_byte
		mov [ebp - 24], eax				; Received cylinder
		call _fdc_read_byte
		mov [ebp - 28], eax				; Received head
		call _fdc_read_byte
		mov [ebp - 32], eax				; Received track
		call _fdc_read_byte
		mov [ebp - 36], eax				; Bytes per sector
		mov eax, [ebp - 12]				; Fetch st0
		test eax, 0xC0					; Check if there is a status error
		jnz .soft_error
		mov eax, [ebp - 16]				; Fetch st1
		test eax, 0xBD					; Check if there was an error
		jnz .soft_error	
		mov eax, [ebp - 20]				; Fetch st2
		test eax, 0x73					; Check if there was an error
		jnz .soft_error	
		mov eax, [ebp - 36]				; Fetch bps
		cmp eax, 2						; It should be equal to 2
		jne .hard_error
		jmp .finished
	.soft_error:
		pop ecx
		loop .retry
	.timeout:
		call _fdc_motor_kill
		push .timeout_message
		call _send_serial_bytes
		cli
		hlt
	.retry:
		call _fdc_motor_off
		push fdc_string.read_cyl_4
		call _send_serial_bytes
		add esp, 4
		jmp .L0
	.hard_error:
		call _fdc_motor_kill
		push .hard_error_message
		call _send_serial_bytes
		cli
		hlt
	.finished:
		call _fdc_motor_off
		push fdc_string.read_cyl_5
		call _send_serial_bytes
		add esp, 4
		pop ecx
		xor eax, eax
	.epilogue:
		mov esp, ebp
		pop ebp
		ret
	.timeout_message:
		db "Floppy Disk Drive timed out reading data. Halting system."
		db 0xA, 0x0
	.hard_error_message:
		db "Floppy Disk Drive has incorrect BPS value. Halting system."
		db 0xA, 0x0

;;
;; Issue each of the DMA transfern commands to the Floppy Disk Controller.
;;
;;	void fdc_transfer_commands(unsigned char cmd, unsigned short cylinder)
;;
_fdc_transfer_commands:
	.prologue:
		push ebp
		mov ebp, esp
		push fdc_string.transfer_cmds
		call _send_serial_bytes
		add esp, 4
	.main:
		mov eax, [ebp + 8]				; Fetch the command
		push eax
		call _fdc_send_cmd 				; Set the current direction
		mov dword[esp], 0		
		call _fdc_send_cmd 				; 0:0:0:0:0:HD:US1:US0
		mov eax, [ebp + 12]				; EAX = cylinder
		mov dword[esp], eax		
		call _fdc_send_cmd 				; Cylinder
		mov dword[esp], 0		
		call _fdc_send_cmd 				; First Head (should match with above)
		mov dword[esp], 1		
		call _fdc_send_cmd 				; First Sector (Counts from 1)
		mov dword[esp], 2		
		call _fdc_send_cmd 				; Bytes/Sector (128*2^N, X=2 -> 512)
		mov dword[esp], 18		
		call _fdc_send_cmd 				; Number of tracks to operate on
		mov dword[esp], 0x1b		
		call _fdc_send_cmd 				; GAP3 length. 27 is default of 3.5"
		mov dword[esp], 0xff		
		call _fdc_send_cmd 				; Data length (0xff is BPS != 0) 
	.epilogue:
		push fdc_string.done
		call _send_serial_bytes
		add esp, 4
		mov esp, ebp
		pop ebp
		ret

;;
;; Translate from a logical sector number to the CHS numbering of a floppy
;; disk.
;;
;;	void fdc_translate_lba(unsigned int lba,
;; 						   unsigned char *cylinder,
;; 						   unsigned char *head,
;; 						   unsigned char *sector)
;;
_fdc_translate_lba:
	.prologue:
		push ebp
		mov ebp, esp
		push fdc_string.lba_1
		call _send_serial_bytes
		add esp, 4
	.main:
		mov eax, [ebp + 8]				; EAX = lba
		xor edx, edx
		mov ebx, 18
		div ebx
		add edx, 1
		and edx, 0xFF
		mov [ebp + 20], edx				; *sector = edx
		xor edx, edx
		mov ebx, 2
		div ebx
		and edx, 0xFF
		and eax, 0xFF
		mov [ebp + 16], edx				; *head = edx
		mov [ebp + 12], eax				; *cylinder = eax
	.epilogue:
		push fdc_string.done
		call _send_serial_bytes
		add esp, 4
		push dword[ebp + 8]
		call _send_serial_number
		add esp, 4
		push .arrow
		call _send_serial_bytes
		add esp, 4
		push dword[ebp + 12]
		call _send_serial_number
		add esp, 4
		push .comma
		call _send_serial_bytes
		add esp, 4
		push dword[ebp + 16]
		call _send_serial_number
		add esp, 4
		push .comma
		call _send_serial_bytes
		add esp, 4
		push dword[ebp + 20]
		call _send_serial_number
		add esp, 4
		push .nl
		call _send_serial_bytes
		add esp, 4
		mov esp, ebp
		pop ebp
		ret
	.arrow:
		db " => (", 0x0
	.comma:
		db ", ", 0x0
	.nl:
		db ")", 0xA, 0x0

;;
;; Read the specified number of sectors from the Flopy Disk, starting from the
;; specified sector.
;;
;;	void fdc_read_sectors(unsigned short sector,
;;						  unsigned short count,
;;						  void *dst)
;;
_fdc_read_sectors:
	.prologue:
		push ebp
		mov ebp, esp
	.main:
		push 0xFF						; [ebp - 4] last_cylinder
		push 0							; [ebp - 8] *sector (unused)
		push 0							; [ebp - 12] *head
		push 0							; [ebp - 16] *cylinder
		push 0							; [ebp - 20] lba
	.prepare:
		mov esi, DISK_DRIVER
		mov eax, [esi + FDCData.last_cylinder]
		mov [ebp - 4], eax
		mov eax, [ebp + 8]				; Fetch the specified sector and use it
		mov [ebp - 20], eax				; as the LBA.
	.next_sector:
		call _fdc_translate_lba 		; LBA -> CHS
		mov eax, [ebp - 16]				; Fetch the *cylinder
		cmp eax, [ebp - 4]				; Is *cylinder equal to last_cylinder?
		je .cylinder_ready				; Yes. We already have the cylinder...
		push eax						; Read the specified cylinder
		mov [ebp - 4], eax				; Update the previous cylinder
		call _fdc_read_cylinder
		add esp, 4
	.cylinder_ready:
		xor edx, edx
		mov eax, [ebp - 20]				; Fetch the current sector (LBA)
		mov ebx, 18
		div ebx							; EDX = EAX % 18
		mov eax, edx
		mov ebx, 512
		mul ebx							; EAX *= 512
		push eax						; Save the current sector offset
		xor edx, edx
		mov eax, [ebp - 12]				; Fetch the head
		mov ebx, 512 * 18
		mul ebx							; Which head are we on?
		pop ebx							; Restore the sector offset
		add eax, ebx					; Combine the two
	.copy:
		mov edi, [ebp + 16]				; Get the current buffer location
		mov esi, DMA_BUFFER
		add esi, eax					; DMA Buffer + Sector Offset
		mov ecx, 512
		rep movsb						; Copy a single sector of data
	.move_to_next_sector:
		mov eax, [ebp - 20]				; Fetch the current sector
		add eax, 1						; Next
		mov [ebp - 20], eax				; Save it
		mov ecx, [ebp + 12]				; Fetch the current sector count
		sub ecx, 1						; Less 1
		mov [ebp + 12], ecx				; Save it
		mov eax, [ebp + 16]				; Fetch the current buffer location
		add eax, 512					; Progress the current buffer location
		mov [ebp + 16], eax				; And save it as well.
		cmp ecx, 0						; Have we got any sectors left?
		jg .next_sector
	.epilogue:
		mov esi, DISK_DRIVER
		mov eax, [ebp - 4]
		mov [esi + FDCData.last_cylinder], eax
		mov esp, ebp
		pop ebp
		ret

fdc_string:
	.read_1:
		db "Reading floppy disk sectors...", 0xA, 0x0
	.read_2:
		db "Finished reading floppy disk sectors!", 0xA, 0x0
	.lba_1:
		db "Translating floppy disk LBA... ", 0x0
	.done: 
		db "done.", 0xA, 0x0
	.transfer_cmds:
		db "Transferring floppy disk controller commands... ", 0x0
	.read_cyl_1:
		db "Preparing to read floppy disk cylinders", 0xA, 0x0
	.read_cyl_2:
		db "Seeking to requested starting sector on floppy disk", 0xA, 0x0
	.read_cyl_3:
		db "Finished seeking floppy disk.", 0xA, 0x0
	.read_cyl_4:
		db "Failed to read, retrying.", 0xA, 0x0
	.read_cyl_5:
		db "Read from floppy disk successfully!", 0xA, 0x0
	.reset_1:
		db "Resetting floppy disk controller.", 0xA, 0x0
	.reset_2:
		db "Setting floppy disk controller transfer speed.", 0xA, 0x0
	.reset_3:
		db "Checking for failures in floppy disk calibration", 0xA, 0x0
	.wait_1:
		db "Waiting for floppy disk controller irq...", 0x0
	.disabling:
		db "Disabling floppy disk controller.", 0xA, 0x0
	.enabling:
		db "Enabling floppy disk controller.", 0xA, 0x0
	.enabled:
		db "Enabled floppy disk controller.", 0xA, 0x0
