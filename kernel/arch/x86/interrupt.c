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
#include <arch/x86/registers.h>
#include <kprint.h>

static interrupt_handler_t *idt_handlers = 0;

void interrupt_handlers_prepare(struct boot_config *config)
{
	idt_handlers = (interrupt_handler_t *)config->interrupt_handlers;
	kprint("Interrupt Handlers table is located at %p\n", idt_handlers);
}

void interrupt_handler_add(uint8_t interrupt, interrupt_handler_t handler)
{
	kprint("Installing interrupt handler %p for interrupt %02x (%d)\n", 
		handler, interrupt, interrupt);
	idt_handlers[interrupt] = handler;
}
