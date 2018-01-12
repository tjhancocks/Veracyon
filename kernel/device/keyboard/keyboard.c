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
#include <kprint.h>
#include <thread.h>

////////////////////////////////////////////////////////////////////////////////

#define KEYBOARD_BUFFER_LEN	64

static uint8_t keyboard_buffer[KEYBOARD_BUFFER_LEN] = { 0 };
static uint32_t keyboard_read_idx = 0;
static uint32_t keyboard_write_idx = 0;

////////////////////////////////////////////////////////////////////////////////

static int32_t keyboard_buffer_count(void)
{
	return keyboard_write_idx - keyboard_read_idx;
}

static void keyboard_insert_scancode(uint8_t code)
{
	// Get the insertion index into the buffer and get the mod of it. 
	uint32_t idx = keyboard_write_idx % KEYBOARD_BUFFER_LEN;
	++keyboard_write_idx;

	// Check if the insertion point is over the buffer length from the read 
	// point. If it is pull the read point forwards.
	int32_t diff = keyboard_buffer_count();
	if (diff >= KEYBOARD_BUFFER_LEN)
		keyboard_read_idx += (diff - KEYBOARD_BUFFER_LEN);

	// Write the scancode into the insertion point of the buffer.
	keyboard_buffer[idx] = code;	
}

static uint8_t keyboard_read_scancode(void)
{
	uint32_t idx = keyboard_read_idx++ % KEYBOARD_BUFFER_LEN;
	return keyboard_buffer[idx];
}

////////////////////////////////////////////////////////////////////////////////


void keyboard_driver_prepare(void)
{
	ps2_keyboard_initialise();
}

void keyboard_received_scancode(uint8_t scancode)
{	
	keyboard_insert_scancode(scancode);
}

struct keyevent *keyboard_consume_key_event(void)
{
	struct keyevent *event = NULL;

	// Check if there are any items in the keyboard buffer. If there are not
	// then return a dummy keyevent.
	if (keyboard_buffer_has_items())
		event = keyevent_make(keyboard_read_scancode());

	return event;
}

struct keyevent *keyboard_wait_for_keyevent(void)
{
	// Wait for input from the keyboard. Halt until we're awoken by hardware.
	// TODO: Wait until key event wakes thread.
	return keyboard_consume_key_event();
}

uint32_t keyboard_buffer_has_items(void)
{
	return keyboard_buffer_count() > 0 ? 1 : 0;
}
