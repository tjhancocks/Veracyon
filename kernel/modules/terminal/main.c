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
#include <atomic.h>
#include <device/keyboard/keyboard.h>
#include <device/device.h>

extern FILE *file_for_pipe(struct pipe *pipe);

void terminal_receive_pipes(void)
{
	device_t dev = get_device(__VT100_ID);
	size_t pipe_count = 0;
	struct pipe **input_pipes = pipe_get_for_process(
		process_get(TERMINAL_PID), 
		p_send,
		&pipe_count
	);

	for (uint32_t i = 0; i < pipe_count; ++i) {
		struct pipe *pipe = input_pipes[i];
		ssize_t unread_len = 0;
		
		atom_t pipe_read;
		atomic_start(pipe_read);

		if (pipe_has_unread(pipe, &unread_len) && unread_len > 0) {
			while (unread_len) {
				// Try and locate where the end of the "string" is.
				ssize_t sublen = 0;
				bool complete = false;
				for (; sublen < unread_len; ++sublen) {
					char c = pipe_peek_byte(pipe, sublen);
					if (c == '\0' || c == '\n') {
						complete = true;
						sublen++;
						break;
					}
				}

				// If the "string" is not complete then abort.
				if (!complete) {
					break;
				}

				// We have the length of the string. We can extract it.
				unread_len -= sublen;
				char *str = calloc(sublen + 1, 1);
				for (int32_t i = 0; i < sublen; ++i) {
					str[i] = pipe_read_byte(pipe, NULL);
				}
				dv_write(dev, str);
				free(str);
			}
		}

		atomic_end(pipe_read);
	}
}

int terminal_main(void)
{
	while (1) {
		terminal_receive_pipes();
		sleep(10);
	}
}
