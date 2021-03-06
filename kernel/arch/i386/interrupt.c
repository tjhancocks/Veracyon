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

#include <arch/i386/port.h>
#include <arch/i386/interrupt.h>
#include <arch/i386/interrupt_frame.h>
#include <boot_config.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <kheap.h>
#include <memory.h>
#include <task.h>
#include <macro.h>

static interrupt_handler_t *idt_stubs = NULL;
static interrupt_handler_t *interrupt_handlers = NULL;
static uint8_t yield_timer = 0;

#define YIELD_THRESHOLD 20

void request_preemption(void)
{
	yield_timer = YIELD_THRESHOLD;
}

void interrupt_irq_stub(struct interrupt_frame *frame)
{
	// Attempt to find the appropriate handler, and execute it.
	uint8_t irq = frame->interrupt + 0x20;
	interrupt_handler_t fn = interrupt_handlers[irq];
	if (fn) {
		fn(frame);
		if (irq != 0x20)
			yield_timer = YIELD_THRESHOLD;
	}

	if (irq == 0x20 && (++yield_timer >= YIELD_THRESHOLD))
	{
		yield_timer = 0;
		yield(frame);
	}
}

void interrupt_handlers_prepare(struct boot_config *config)
{
	// Disable interrupts for the duration of this function.
	__asm__ __volatile__("cli");

	idt_stubs = (interrupt_handler_t *)config->interrupt_stubs;
	fprintf(dbgout, "Interrupt stubs table is located at %p\n", idt_stubs);

	// Allocate space for the interrupt handlers table.
	interrupt_handlers = kalloc(sizeof(*interrupt_handlers) * 256);
	memset(interrupt_handlers, 0, sizeof(*interrupt_handlers) * 256);

	// Install a generic stub for each of the IRQ's - these will be used for
	// preemption.
	for (uint8_t irq = 0x20; irq < 0x30; ++irq) {
		idt_stubs[irq] = interrupt_irq_stub;
	}

	fprintf(dbgout, "Installed interrupt stubs for each IRQ\n");

	// Re-enable interrupts
	__asm__ __volatile__("sti");
}

void interrupt_handler_add(uint8_t interrupt, interrupt_handler_t handler)
{
	fprintf(dbgout, "Installing interrupt handler %p for interrupt %02x (%d)\n", 
		handler, interrupt, interrupt);
	interrupt_handlers[interrupt] = handler;
}
