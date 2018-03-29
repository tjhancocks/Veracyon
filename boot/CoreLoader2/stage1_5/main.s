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

	[bits 16]

; CoreLoader entry point. The bootsector _should_ be dropping CPU control at
; this point.
CoreLoader.main:
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
	.config:
		; The user may have supplied a boot configuration file. We need to fetch
		; it from disk, read and parse it and then handle further loading as
		; appropriate.
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
	.catch:
		cli
		hlt

; Strings used in Stage1.5 of CoreLoader.
CoreLoader.Stage1_5.Strings:
	.version:	
		db "CoreLoader Version 0.3-alpha", 0xD
		db "Copyright (c) 2017-2018 Tom Hancocks. MIT License.", 0xD, 0xD, 0x0

; Include all supporting source files and objects.
CoreLoader.Stage1_5.Supporting:
	%include "CoreLoader2/Stage1_5/rs232.s"
	%include "CoreLoader2/Stage1_5/memory.s"