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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <process.h>
#include <pipe.h>
#include <device/keyboard/keyboard.h>
#include <device/device.h>

extern FILE *file_for_pipe(struct pipe *pipe);

void console_receive_pipes(void)
{
	device_t dev = get_device(__VT100_ID);
	size_t pipe_count = 0;
	struct pipe **input_pipes = pipe_get_for_process(
		process_get(3), 
		p_send,
		&pipe_count
	);
	for (uint32_t i = 0; i < pipe_count; ++i) {
		FILE *stream = file_for_pipe(input_pipes[i]);
		if (!feof(stream)) {
			char line[81] = { 0 };
			fgets(line, 80, stream);
			dv_write(dev, line);
		}
	}
}

int console_main(void)
{
	while (1) {
		console_receive_pipes();
		sleep(10);
	}
}
