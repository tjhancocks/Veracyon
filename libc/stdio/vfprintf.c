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
#include <task.h>
#include <process.h>
#include <pipe.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <device/RS232/RS232.h>

#ifndef __PRINTF_BUFFER_LEN
#	define __PRINTF_BUFFER_LEN 1024
#endif

void vfprintf(FILE *fd, const char *restrict fmt, va_list args)
{
	char buffer[__PRINTF_BUFFER_LEN] = { 0 };
	vsnprintf((void *)&buffer, __PRINTF_BUFFER_LEN, fmt, args);
	buffer[__PRINTF_BUFFER_LEN - 1] = '\0';
	
#if __libk__
	// After multitasking is enabled we want to use process pipes for output.
	// Certain FILEs such as COM1 and VT100 will not work at this point.
	struct process *current = NULL;
	struct pipe *pipe = NULL; 
	if (fd != COM1 && fd != VT100 &&
		task_allowed() && 
		task_get_current() &&
		task_get_current()->thread &&
		(current = task_get_current()->thread->owner) &&
		(pipe = process_get_pipe(current, *((uint32_t *)fd)))
	) {
		size_t buffer_len = strlen(buffer);
		pipe_write(pipe, buffer, buffer_len);
	}
	else {
		// Make sure we are COM1 or VT100 at this point.
		if (fd != COM1 || fd != VT100) {
			fd = COM1;
		}

		device_t dev = get_device(*((uint32_t *)fd));
		dv_write(dev, buffer);
	}
#endif
}