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

#include <arch/x86/port.h>
#include <arch/x86/interrupt.h>
#include <arch/x86/cpu_state.h>
#include <kprint.h>
#include <kheap.h>
#include <null.h>
#include <memory.h>

static interrupt_handler_t *idt_stubs = NULL;
static interrupt_handler_t *interrupt_handlers = NULL;

void interrupt_irq_stub(struct interrupted_cpu_state *state)
{
	// Attempt to find the appropriate handler, and execute it.
	uint8_t irq = state->interrupt + 0x20;
	interrupt_handler_t fn = interrupt_handlers[irq];
	if (fn) {
		fn(state);
	}
}

void interrupt_handlers_prepare(struct boot_config *config)
{
	// Disable interrupts for the duration of this function.
	__asm__ __volatile__("cli");

	idt_stubs = (interrupt_handler_t *)config->interrupt_stubs;
	kdprint(dbgout, "Interrupt stubs table is located at %p\n", idt_stubs);

	// Allocate space for the interrupt handlers table.
	interrupt_handlers = kalloc(sizeof(*interrupt_handlers) * 256);
	memset(interrupt_handlers, 0, sizeof(*interrupt_handlers) * 256);

	// Install a generic stub for each of the IRQ's - these will be used for
	// preemption.
	for (uint8_t irq = 0x20; irq < 0x30; ++irq) {
		idt_stubs[irq] = interrupt_irq_stub;
		kdprint(dbgout, "  > installing stub for interrupt %02x\n", irq);
		kdprint(dbgout, "    > idt_stubs[0x%02x] = %p\n",
			irq, idt_stubs[irq]);
	}

	kdprint(dbgout, "Installed interrupt stubs for each IRQ\n");

	// Re-enable interrupts
	__asm__ __volatile__("sti");
}

void interrupt_handler_add(uint8_t interrupt, interrupt_handler_t handler)
{
	kdprint(dbgout, "Installing interrupt handler %p for interrupt %02x (%d)\n", 
		handler, interrupt, interrupt);
	interrupt_handlers[interrupt] = handler;
}
