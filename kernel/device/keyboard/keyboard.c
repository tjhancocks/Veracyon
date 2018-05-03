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

#include <device/keyboard/keyboard.h>
#include <device/keyboard/scancode.h>
#include <device/ps2/keyboard.h>
#include <arch/arch.h>
#include <kheap.h>
#include <stdio.h>
#include <stddef.h>
#include <task.h>
#include <pipe.h>

////////////////////////////////////////////////////////////////////////////////

static struct pipe *keyboard_get_frontmost_pipe()
{
	// Ask the process API for the Keyboard Receiver pipe for the frontmost
	// process.
	return pipe_get_best(
		process_get_frontmost(), 
		p_recv | p_keyboard
	);
}

static struct pipe *keyboard_get_current_pipe()
{
	// Ask the process API for the Keyboard Receiver pipe for the owning process
	// of the current task.
	return pipe_get_best(
		task_get_current()->thread->owner, 
		p_recv | p_keyboard
	);
}

////////////////////////////////////////////////////////////////////////////////

static void kbdin_write_scancode(uint8_t code)
{
	struct pipe *pipe = keyboard_get_frontmost_pipe();
	if (!pipe)
		return;
	pipe_write(pipe, &code, sizeof(code));
}

static uint8_t kbdin_read_scancode(void)
{
	struct pipe *pipe = keyboard_get_current_pipe();
	if (!pipe)
		return '\0';
	return pipe_read_byte(pipe);
}

////////////////////////////////////////////////////////////////////////////////


void keyboard_driver_prepare(void)
{
	ps2_keyboard_initialise();
}

void keyboard_received_scancode(uint8_t scancode)
{	
	kbdin_write_scancode(scancode);
	task_resume_any_for(reason_key_wait, 0);
}

struct keyevent *keyboard_consume_key_event(void)
{
	return keyevent_make(kbdin_read_scancode());
}

struct keyevent *keyboard_wait_for_keyevent(void)
{
	// Wait for input from the keyboard. Halt until we're awoken by hardware.
	struct keyevent *event = keyboard_consume_key_event();
	while (!event) {
		__asm__ __volatile__("hlt");
		event = keyboard_consume_key_event();
	}
	return event;
}
