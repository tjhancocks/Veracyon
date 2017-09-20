/*
 Copyright (c) 2017 Tom Hancocks
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

ENTRY(start)
SECTIONS
{
	. = 0xC0000000;
	kernel_start = .; _kernel_start = .; __kernel_start = .;

	.text ALIGN(0x1000) : {
		*(.text)
	}
	.rodata ALIGN(0x1000) : {
		*(.rodata*)
	}
	.data ALIGN(0x1000) :  {
		*(.data)
	}
	.bss ALIGN(0x1000) :  {
		sbss = .;
		*(COMMON)
		*(.bss)
		ebss = .;
	}
	
	end = .;
	kernel_end = .; _kernel_end = .; __kernel_end = .;

	/DISCARD/ : {
		*(.common)
		*(.eh_frame)
		*(.note.gnu.build-id)
	}
}