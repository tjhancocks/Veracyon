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
#include <device/ps2/keyboard.h>
#include <arch/arch.h>
#include <kheap.h>
#include <stdio.h>
#include <task.h>

////////////////////////////////////////////////////////////////////////////////

static int32_t kbdin_buffer_count(void)
{
	// Get the frontmost process. We'll need to get the KBDIN for it in order
	// to write the scancode to it. 
	struct process *proc = task_get_current()->thread->owner;

	// If there is no process then report 0.
	// TODO: Maybe a panic here as it should be impossible?
	if (!proc)
		return 0;

	return proc->kbdin.w_idx - proc->kbdin.r_idx;
}

static void kbdin_write_scancode(uint8_t code)
{
	// Get the frontmost process. We'll need to get the KBDIN for it in order
	// to write the scancode to it. 
	struct process *proc = process_get_frontmost();

	// If there is no frontmost process then just discard the scancode.
	// TODO: At some point an agent should accept all scancodes as well in order
	// to handle global shortcuts.
	if (!proc)
		return;

	// Get the insertion index into the buffer and get the mod of it. 
	uint32_t idx = proc->kbdin.w_idx % proc->kbdin.size;
	++proc->kbdin.w_idx;

	// Check if the insertion point is over the buffer length from the read 
	// point. If it is pull the read point forwards.
	int32_t diff = proc->kbdin.w_idx - proc->kbdin.r_idx;
	if (diff >= proc->kbdin.size)
		proc->kbdin.r_idx += (diff - proc->kbdin.size);

	// Write the scancode into the insertion point of the buffer.
	proc->kbdin.buffer[idx] = code;	
}

static uint8_t kbdin_read_scancode(void)
{
	// Get the frontmost process. We'll need to get the KBDIN for it in order
	// to write the scancode to it. 
	struct process *proc = task_get_current()->thread->owner;

	// If there is no frontmost process then just return NUL char
	// TODO: Maybe a panic here as it should be impossible?
	if (!proc)
		return '\0';

	uint32_t idx = proc->kbdin.r_idx++ % proc->kbdin.size;
	return proc->kbdin.buffer[idx];
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
	struct keyevent *event = NULL;

	// Check if there are any items in the keyboard buffer. If there are not
	// then return a dummy keyevent.
	if (kbdin_buffer_count() > 0)
		event = keyevent_make(kbdin_read_scancode());

	return event;
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

uint32_t keyboard_buffer_has_items(void)
{
	return kbdin_buffer_count() > 0 ? 1 : 0;
}
