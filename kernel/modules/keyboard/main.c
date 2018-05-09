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

int keyboard_main(void)
{
	// Identify the keyboard output pipe.
	size_t pipe_count = 0;
	struct pipe **input_pipes = pipe_get_for_process(
		process_get(4), 
		p_recv | p_keyboard,
		&pipe_count
	);

	if (pipe_count != 1) {
		// TODO: Handle this error case
		return;
	}

	// Get the actual pipe.
	atom_t key_event_atom;
	struct pipe *pipe = input_pipes[0];

	// Enter an infinite loop and keep checking for input.
	while (1) {
		// Wait for input, and then process it.
		while (!pipe_has_unread(pipe, NULL)) {
			key_wait();
		}

		bool is_empty = false;
		uint8_t scancode = 0;
		if (!(scancode = pipe_read_byte(pipe, &is_empty)) || is_empty) {
			continue;
		}

		atomic_start(key_event_atom);
		fprintf(dbgout, "[KBD] Received scancode: %02x\n", scancode);

		struct keyevent *event = keyevent_make(scancode);
		if (event->pressed) {
			free(event);
			fprintf(dbgout, "[KBD] Ignoring\n");
			atomic_end(key_event_atom);
			continue;
		}

		fprintf(dbgout, "[KBD] => keycode=%02x modifiers=%02x\n", 
			event->keycode, event->modifiers);

		char c = keycode_to_ascii(event->keycode, event->modifiers);
		free(event);


		fprintf(dbgout, "[KBD] => char '%c'\n", c);
		
		// Acquire a reference to the key process.
		struct process *key = process_get_key();
		fprintf(dbgout, "[KBD] Sending to '%s'\n", key->name);
		struct pipe *stdin = pipe_get_best(key, p_recv);
		fprintf(dbgout, "[KBD] Using pipe <%p>\n", stdin);
		pipe_write_byte(stdin, c);

		atomic_end(key_event_atom);
	}
}
