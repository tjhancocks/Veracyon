/*
 Copyright (c) 2017-2018 Tom Hancocks
 
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

#if __libk__
#include <device/device.h>
#endif

#include <stdio.h>

#ifndef __PRINTF_BUFFER_LEN
#	define __PRINTF_BUFFER_LEN 1024
#endif

void vfprintf(FILE *fd, const char *restrict fmt, va_list args)
{
	char buffer[__PRINTF_BUFFER_LEN] = { 0 };
	vsnprintf(&buffer, __PRINTF_BUFFER_LEN, fmt, args);
	buffer[__PRINTF_BUFFER_LEN - 1] = '\0';
	
#if __libk__
	device_t dev = get_device(*((uint32_t *)fd));
	dv_write(dev, buffer);
#endif
}