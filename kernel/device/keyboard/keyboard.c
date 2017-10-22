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
#include <kprint.h>
#include <panic.h>
#include <arch/arch.h>
#include <kheap.h>

////////////////////////////////////////////////////////////////////////////////

#define KEYBOARD_BUFFER	1024

struct raw_key_event;
struct raw_key_event {
	struct raw_key_event *next;
	uint8_t scancode;	
};

////////////////////////////////////////////////////////////////////////////////

static uint8_t keyboard_modifier_state = 0;
struct raw_key_event *keyboard_buffer_first = NULL;
struct raw_key_event *keyboard_buffer_last = NULL;
static uint32_t keyboard_buffer_count = 0;

////////////////////////////////////////////////////////////////////////////////

uint32_t keyboard_buffer_has_items()
{
	return keyboard_buffer_count;
}

uint8_t keyboard_buffer_dequeue()
{
	struct raw_key_event *event = keyboard_buffer_first;
	keyboard_buffer_first = event->next;
	keyboard_buffer_count--;
	uint8_t scancode = event->scancode;
	kfree(event);
	return scancode;
}

void keyboard_buffer_enqueue(uint8_t raw_code)
{
	if (keyboard_buffer_count >= KEYBOARD_BUFFER)
		return;

	struct raw_key_event *event = kalloc(sizeof(*event));
	event->scancode = raw_code;

	if (keyboard_buffer_last)
		keyboard_buffer_last->next = event;

	keyboard_buffer_last = event;
	keyboard_buffer_first = keyboard_buffer_first ?: event;
	keyboard_buffer_count++;
}


////////////////////////////////////////////////////////////////////////////////


void keyboard_driver_prepare()
{
	ps2_keyboard_initialise();
}

void keyboard_received_scancode(uint8_t raw_code)
{
	keyboard_buffer_enqueue(raw_code);
}

uint8_t keyboard_modifier_flags()
{
	return keyboard_modifier_state;
}

struct scancode_info keyboard_consume_key_event()
{
	// Check if there are any items in the keyboard buffer. If there are not
	// then return a dummy keyevent.
	if (keyboard_buffer_has_items() == 0)
		return (struct scancode_info) {
			.index = 0xFF,
			.scancode = 0x00,
			.name = NULL,
			.pressed = 0,
			.modifier = 0
		};

	uint8_t raw_code = keyboard_buffer_dequeue();
	struct scancode_info info = scancode_info_make(raw_code);

	// Check for modifier codes to begin with.
	if ((info.modifier & keyboard_mod_none) == 0 && info.modifier > 1) {
		// This is a modifier scancode.
		if (info.pressed)
			keyboard_modifier_state |= info.modifier;
		else
			keyboard_modifier_state &= ~info.modifier;
	}

	return info;
}

struct scancode_info keyboard_get_scancode()
{
	// Wait for input from the keyboard. Halt until we're awoken by hardware.
	while (keyboard_buffer_has_items() == 0)
		__asm__ __volatile__(
			"nop\n"
			"hlt"
		);

	struct scancode_info info = keyboard_consume_key_event();
	return info;
}
