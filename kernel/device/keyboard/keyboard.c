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

////////////////////////////////////////////////////////////////////////////////

#define KEYBOARD_BUFFER	64

struct buffer_item;
struct buffer_item {
	struct buffer_item *next;
	struct keyevent *event;	
};

////////////////////////////////////////////////////////////////////////////////

static struct buffer_item *buffer_first = NULL;
static struct buffer_item *buffer_last = NULL;
static uint32_t buffer_count = 0;

////////////////////////////////////////////////////////////////////////////////

uint32_t keyboard_buffer_has_items()
{
	return buffer_count;
}

struct keyevent *keyboard_buffer_dequeue()
{
	struct buffer_item *item = buffer_first;
	
	if (item == NULL)
		return NULL;

	struct keyevent *event = item->event;

	--buffer_count;
	buffer_first = item->next;
	if (buffer_count == 0)
		buffer_last = NULL;
	kfree(item);

	return event;
}

void keyboard_buffer_enqueue(struct keyevent *event)
{
	if (buffer_count >= KEYBOARD_BUFFER) {
		return;
	}
	if (buffer_count >= KEYBOARD_BUFFER ||
		(buffer_last && buffer_last->event && event &&
		 buffer_last->event->keycode == event->keycode)
	) {
		return;
	}

	struct buffer_item *item = kalloc(sizeof(*item));
	item->event = event;
	item->next = NULL;

	if (buffer_last) {
		buffer_last->next = item;
	}
	buffer_last = item;

	if (!buffer_first) {
		buffer_first = item;
	}

	++buffer_count;
}


////////////////////////////////////////////////////////////////////////////////


void keyboard_driver_prepare()
{
	ps2_keyboard_initialise();
}

void keyboard_received_scancode(uint8_t scancode)
{
	struct keyevent *event = keyevent_make(scancode);
	keyboard_buffer_enqueue(event);
}

struct keyevent *keyboard_consume_key_event()
{
	// Check if there are any items in the keyboard buffer. If there are not
	// then return a dummy keyevent.
	if (keyboard_buffer_has_items() == 0)
		return NULL;

	return keyboard_buffer_dequeue();
}

struct keyevent *keyboard_wait_for_keyevent()
{
	// Wait for input from the keyboard. Halt until we're awoken by hardware.
	while (keyboard_buffer_has_items() == 0)
		__asm__ __volatile__(
			"nop\n"
			"hlt"
		);

	return keyboard_consume_key_event();
}
