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

#include <arch/arch.h>
#include <device/PS2/keyboard.h>
#include <device/keyboard/keyboard.h>
#include <stdio.h>
#include <kheap.h>
#include <panic.h>

////////////////////////////////////////////////////////////////////////////////

void ps2_keybaord_wait(void)
{
	while ((inb(0x64) & 0x2) == 1)
		__asm__("nop");
}

void ps2_keyboard_interrupt_handler(
	struct interrupt_frame *frame __attribute__((unused))
) {
	ps2_keybaord_wait();
	uint8_t raw_code = inb(0x60);
	keyboard_received_scancode(raw_code);
}

void ps2_keyboard_reset(void)
{
	uint8_t tmp = inb(0x61);
	outb(0x61, tmp | 0x80);
	outb(0x61, tmp & 0x7F);
	(void)inb(0x60);
}

void ps2_keyboard_initialise(void)
{
	fprintf(COM1, "Initialising PS/2 keyboard\n");
	interrupt_handler_add(0x21, ps2_keyboard_interrupt_handler);
	ps2_keyboard_reset();
}
