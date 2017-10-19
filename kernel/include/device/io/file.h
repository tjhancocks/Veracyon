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

#ifndef __VKERNEL_DEVICE_IO_FILE__
#define __VKERNEL_DEVICE_IO_FILE__

#include <kern_types.h>

// Kernel Console
#define __kKRNOUT	(0x1 << 1)

// Debug Serial Port
#define __kDBGOUT	(0x1 << 2)

// Kernel Console & Debug Serial Port
#define __kALLOUT 	(__kKRNOUT | __kDBGOUT)

void devio_bind_putc(uint32_t handle, void(*fn)(const char));
void devio_bind_puts(uint32_t handle, void(*fn)(const char *restrict));

void devio_putc(uint32_t handle, const char c);
void devio_puts(uint32_t handle, const char *restrict str);

#endif