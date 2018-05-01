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

static void kbdin_write_scancode(uint8_t code)
{
	// Get the frontmost process. We'll need to get the keyboard input for it in
	// order to write the scancode to it. 
	struct process *proc = process_get_frontmost();

	// If there is no frontmost process then just discard the scancode.
	// TODO: At some point an agent should accept all scancodes as well in order
	// to handle global shortcuts.
	if (!proc)
		return;

	// Write the scancode to standard input of the frontmost process.
	pipe_write(proc->pipe.kbdin, &code, sizeof(code));
}

static uint8_t kbdin_read_scancode(void)
{
	// Get the frontmost process. We'll need to get the keyboard input for it in
	// order to write the scancode to it. 
	struct process *proc = task_get_current()->thread->owner;

	// If there is no frontmost process then just return NUL char
	// TODO: Maybe a panic here as it should be impossible?
	if (!proc)
		return '\0';

	// The scancode is at the first available in the pipe.
	return pipe_read_byte(proc->pipe.kbdin);
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
