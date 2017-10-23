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

#include <device/keyboard/keyboard.h>
#include <device/keyboard/scancode.h>
#include <device/keyboard/keycode.h>
#include <ascii.h>
#include <read.h>
#include <null.h>
#include <kheap.h>
#include <term.h>
#include <device/io/file.h>

char read_char()
{
	struct keyevent *event = NULL;

	do {
		// Wait for an event from the keyboard, and if we receive a NULL event
		// then reset. If we receive a keyevent for a released key then ignore
		// it.
		if (event)
			kfree(event);

		event = keyboard_wait_for_keyevent();
	}
	while (event == NULL || event->pressed == 0);

	// Convert the keycode into an ASCII code.
	char c = keycode_to_ascii(event->keycode, event->modifiers);
	return c;
}

const char *read_user_input()
{
	char *buffer = kalloc(sizeof(*buffer) * 1024);
	memset(buffer, 0, sizeof(*buffer) * 1024);
	char *ptr = buffer;
	struct keyevent *event = NULL;
	uint32_t cursor_x = 0;
	uint32_t cursor_y = 0;

	// Get current terminal state so that we can render and handle the input
	// correctly.
	term_get_cursor(&cursor_x, &cursor_y);
	
	// Start listening for key events, exiting only on an exit code or enter.
	do {
		// Listen for a key event.
		if (event) kfree(event);
		event = keyboard_wait_for_keyevent();

		// If this a key press? If yes, then work out what to do with the entry.
		if (event->pressed == 1) {
			if (event->keycode == kKC_ANSI_BACKSPACE) {
				if (ptr >= buffer) {
					// Flush an update now with the last character cleared.
					*(ptr - 1) = ' ';
					term_set_cursor(cursor_x, cursor_y);
					devio_puts(__kKRNOUT, buffer);

					// Set the null buffer and continue.
					if (ptr > buffer)
						*(--ptr) = 0;
					else
						*ptr = 0;
				}
			}
			else if (
				event->keycode == kKC_ANSI_ENTER ||
				event->keycode == kKC_ANSI_NUM_ENTER
			) {
				goto COMPLETE_INPUT;
			}
			else {
				char c = keycode_to_ascii(event->keycode, event->modifiers);
				if (c)
					*ptr++ = c;
			}

			// Draw the input to the console/terminal.
			term_set_cursor(cursor_x, cursor_y);
			devio_puts(__kKRNOUT, buffer);
		}

		// Control codes should only occur on key release.
		else {
			enum key_modifiers control_mask = key_modifier_left_control;
			control_mask |= key_modifier_right_control;
	
			if (
				event->keycode == kKC_ANSI_C && 
				event->modifiers & control_mask
			) {
				devio_puts(__kKRNOUT, "^C");
				goto CANCEL_INPUT;
			}
		}
	}
	while (1); // Loop forever. Exit should be handled internally.

CANCEL_INPUT:
	// The user cancelled the input.
	devio_puts(__kKRNOUT, "\n");
	kfree(event);
	kfree(buffer);
	return NULL;

COMPLETE_INPUT:
	// The user successfully completed the input.
	devio_puts(__kKRNOUT, "\n");
	kfree(event);
	return buffer;
}