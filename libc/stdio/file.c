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

#include <pipe.h>
#include <stdio.h>
#include <task.h>
#include <thread.h>
#include <process.h>

struct __vFILE {
	uintptr_t descriptor;
	uint32_t fallback_device;
};

FILE *file_for_pipe(struct pipe *pipe) 
{
	if (pipe->purpose == p_recv) {
		return stdin;
	}
	else if (pipe->purpose == p_send) {
		return stdout;
	}
	else if (pipe->purpose == p_send | p_dbg) {
		return dbgout;
	}
	else if (pipe->purpose == p_send | p_err) {
		return stderr;
	}
	else {
		// TODO: Warn about unknown file for pipe. Default to stderr
		return stderr;
	}
}

struct pipe *pipe_for_file(FILE *file)
{
	return pipe_get_best(task_get_current()->thread->owner, file->descriptor);
}

#endif