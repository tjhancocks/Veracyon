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

#include <arch/i386/pit.h>
#include <arch/i386/port.h>
#include <arch/i386/interrupt.h>
#include <arch/i386/interrupt_frame.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////

static struct {
	uint32_t phase;
	uint32_t subticks;
	uint32_t ticks;
} pit_info;


////////////////////////////////////////////////////////////////////////////////

static void pit_set_frequency(uint32_t freq)
{
	int32_t divisor = 1193180 / freq;

	fprintf(COM1, 
		"Setting frequency of Programmable Interrupt Timer to %dHz\n",
		freq);

	outb(0x43, 0x36);
	outb(0x40, divisor & 0xFF);
	outb(0x40, (divisor >> 8) & 0xFF);
}


////////////////////////////////////////////////////////////////////////////////

static void pit_interrupt_event(
	struct interrupt_frame *frame __attribute__((unused))
) {
	if (++pit_info.subticks >= pit_info.phase) {
		pit_info.subticks = 0;
		++pit_info.ticks;
	}

	// TODO: Send notification to timers.
}


////////////////////////////////////////////////////////////////////////////////

void pit_prepare(void)
{
	fprintf(COM1, "Installing the Programmable Interrupt Timer.\n");
	pit_info.phase = 1000;

	pit_set_frequency(pit_info.phase);
	interrupt_handler_add(0x20, pit_interrupt_event);
}

uint32_t pit_get_ticks(void)
{
	return pit_info.ticks;
}

uint32_t pit_get_subticks(void)
{
	return pit_info.subticks;
}
