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

; CoreLoader entry point. The bootsector _should_ be dropping CPU control at
; this point.
CoreLoader.Stage1_5.main:
	.RS232_setup:
		; The first thing we need to do is ensure the RS232 debug serial port
		; is setup and ready to go. All debug logs will be sent out to an
		; external monitor via COM1.
		call rs232.init
		mov si, CoreLoader.Stage1_5.Strings.version
		call rs232.send_bytes
	.memory:
		; The second thing we need to ensure is configured and known is the
		; memory, and its specific mapping and layout. This will allow and help
		; in placing the kernel and setting up a paging environment.
		call memory.detect
	.file_system:
		; Load the appropriate file system driver for use. Each driver provides
		; a set of common interfaces, which can be assigned to the appropriate
		; stub interfaces.
		call vfs.init
	.config:
		; The user may have supplied a boot configuration file. We need to fetch
		; it from disk, read and parse it and then handle further loading as
		; appropriate.
		mov si, CoreLoader.Stage1_5.Files.config
		call vfs.read_file
		call config.parse_file
		jc .config_error
		mov si, CoreLoader.Stage1_5.Strings.config_loaded
		call rs232.send_bytes
		jmp .display
	.config_error:
		; If we land here then there was a problem with the config file.
		; TODO: This should be handled better than this at some point, but for
		; now it will suffice.
		mov si, CoreLoader.Stage1_5.Strings.config_error
		call rs232.send_bytes
		jmp .catch
	.display:
		; We need to detect all present VESA/VBE modes, and switch to the most
		; appropriate mode. This may be a graphical mode or a text mode 
		; depending on the users preference. The fall back order is:
		;
		;	1. User Preference (if given)
		;	2. Native VESA Graphical Mode (if available)
		; 	3. 800x600 Graphical Mode (if possible)
		; 	4. 80x25 Text Mode
	.protected_mode:
		; Get the CPU in to protected mode. For this the Global Descriptor 
		; Table, A20 Address Line and Protected Mode flags need to be installed.
		mov si, CoreLoader.Stage1_5.Strings.prepare_pmode
		call rs232.send_bytes
		call gdt.init
	.a20:
		; Check to see if the A20 line can be set using the "Fast A20" method.
		; If it can not then halt the system after reporting it. Not currently
		; supporting other methods of setting it.
		in al, 0x2d						; Get CMOS register 2d
		test al, (1 << 1)				; Test if FAST A20 is enabled.
		jz .no_fast_a20
	.fast_a20:
		in al, 0x92
		or al, 2
		out 0x92, al
		jmp .enable_unreal_mode
	.no_fast_a20:
		mov si, CoreLoader.Stage1_5.Strings.fast_a20_unsupported
		call rs232.send_bytes
		jmp .catch
	.enable_unreal_mode:
		; Protected Mode is managed by bit 0 of the CR0 register. By setting
		; this bit, the system will be running in unreal mode until we jump to
		; 32bit code.
		mov eax, cr0					; Fetch the current contents of CR0
		or eax, (1 << 0)				; Set the protected mode BITS
		mov cr0, eax					; Write the updated value back to CR0.
	.finalize_pmode:
		; We now need to finalize protected mode. To do this we need to install
		; Global Descriptor Table into the CPU and perform a far jump to the
		; appropriate segment and routine.
		; If this is wrong in anyway then the system will triple fault and 
		; reset.
		; We _MUST_ also make sure the interrupts are disabled. If we interrupt
		; once we switch to protected mode before we've configured an interrupt
		; descriptor table, we will also cause a triple fault.
		cli 							; Make sure interrupts are disabled!
		in al, 0x70						; Disable Non Maskable Interrupts in CPU
		or al, 0x80	
		out 0x70, al
		mov eax, GDTPTR_ADDR
		lgdt [cs:eax]					; Load the GDT Pointer into the CPU.
		mov ax, 0x10					; Kernel Data Segment is 0x10 (Sel #3)
		mov ds, ax						; Make sure all data segments are fixed
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax
		jmp 0x8:CoreLoader.Stage2.main	; Far jump to 32-bit protected mode.
	.catch:
		cli
		hlt

; Strings used in Stage1.5 of CoreLoader.
CoreLoader.Stage1_5.Strings:
	.version:	
		db "CoreLoader Version 0.3-alpha", 0xD
		db "Copyright (c) 2017-2018 Tom Hancocks. MIT License.", 0xD, 0xD, 0x0
	.config_loaded:
		db "BOOT.CLI successfully loaded and parsed.", 0xD, 0x0
	.config_error:
		db "BOOT.CLI could not be loaded successfully.", 0xD, 0x0
	.prepare_pmode:
		db "Preparing to enter protected mode.", 0xD, 0x0
	.fast_a20_unsupported:
		db "Fatal Error: FAST A20 is not enabled on this machine.", 0xD, 0x0

; Files & Dependancies
CoreLoader.Stage1_5.Files:
	.config:
		db "BOOT    CLI", 0x0

; Include all supporting source files and objects.
CoreLoader.Stage1_5.Supporting:
	%include "CoreLoader/Stage1_5/macro.s"
	%include "CoreLoader/Stage1_5/rs232.s"
	%include "CoreLoader/Stage1_5/memory.s"
	%include "CoreLoader/Stage1_5/fs.s"
	%include "CoreLoader/Stage1_5/config.s"
	%include "CoreLoader/Stage1_5/gdt.s"
