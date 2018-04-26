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

#ifndef __VKERNEL_KPRINT__
#define __VKERNEL_KPRINT__

#include <kern_types.h>
#include <device/device.h>

/**
 Kernel specific print function, providing similar capabilities printf().
 Routes output to the "TERM" device.

 	- fmt: The format string with which to use.
 */
void kprint(const char *restrict fmt, ...);

/**
 Kernel specific print function, providing similar capabilities printf().
 Allows the destination route (i.e. COM1, TERM) to be specified,
 so that all output is sent to the correct/desired place.

	- dev: The device in which to send the result to.
 	- fmt: The format string with which to use.
 */
void kdprint(device_t dev, const char *restrict fmt, ...);

/**
 Kernel specific print function, providing similar capabilities printf().
 Allows the destination route (i.e. COM1, TERM) to be specified,
 so that all output is sent to the correct/desired place.

	- dev: The device in which to send the result to.
 	- fmt: The format string with which to use.
 	- va: The variadic arguments structure with substitution values.
 */
void kdprintv(device_t dev, const char *restrict fmt, va_list va);

#endif