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

struct __vFILE {
	uintptr_t descriptor;
	uint32_t fallback_device;
};

#ifndef __PRINTF_BUFFER_LEN
#	define __PRINTF_BUFFER_LEN 1024
#endif

void vfprintf(FILE *fd, const char *restrict fmt, va_list args)
{
	if (!fd) {
		// TODO: Panic here. This is serious. Warn if we're in LIBC.
		return;
	}

	char buffer[__PRINTF_BUFFER_LEN] = { 0 };
	vsnprintf((void *)&buffer, __PRINTF_BUFFER_LEN, fmt, args);
	buffer[__PRINTF_BUFFER_LEN - 1] = '\0';

#if __libk__
	device_t dev = NULL;
	
	// Pipes only become reliably available once multitasking is enabled.
	if (!task_allowed()) goto PIPE_UNAVAILABLE;

	struct task *task = task_get_current();
	if (!task) goto PIPE_UNAVAILABLE;
	if (!task->thread) goto PIPE_UNAVAILABLE;
	if (!task->thread->owner) goto PIPE_UNAVAILABLE;

	struct process *proc = task->thread->owner;
	struct pipe *pipe = pipe_get_best(proc, fd->descriptor);
	if (!pipe) goto PIPE_UNAVAILABLE;

	size_t buffer_len = strlen(buffer);
	pipe_write(pipe, (uint8_t *)buffer, buffer_len);
	return;

PIPE_UNAVAILABLE:
	// Check for a direct device to fallback to.
	dev = get_device(fd->fallback_device);
	if (dev) {
		dv_write(dev, buffer);
	}
#endif
}