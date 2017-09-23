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
		mov dword[esi + FileSystemInterface.file_read], _fat12_file_read
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
;; 	void fat12_file_read(const char *name, void *dst)
;;
_fat12_file_read
	.main:
	.epilogue:
		mov esp, ebp
		pop ebp
		ret