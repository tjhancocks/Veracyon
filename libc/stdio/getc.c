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

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

#if __libk__
#include <pipe.h>
#include <device/keyboard/scancode.h>
#include <device/keyboard/keycode.h>
extern struct pipe *pipe_for_file(FILE *file);
#endif

int getc(FILE *fd)
{

#if __libk__
	struct pipe *pipe = pipe_for_file(fd);
	if (!pipe) {
		fprintf(dbgout, "[libk] Failed to acquire pipe for file.\n");
		return 0;
	}

	while (!pipe_has_unread(pipe, NULL)) 
		sleep(5);

	// We have keyboard input. Now read the character from the pipe.
	return (char)pipe_read_byte(pipe, NULL);;
#endif

	return 0;
}