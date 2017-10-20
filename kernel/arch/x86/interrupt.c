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

typedef void(*interrupt_handler_t)(struct registers *);
static interrupt_handler_t *idt_handlers = 0;

static void ps2_keypress_handler(struct registers *registers)
{
	uint8_t scancode = inb(0x60);
	kprint("keypress (scancode = %02x)\n", scancode);
}

void interrupt_handlers_prepare(struct boot_config *config)
{
	idt_handlers = (interrupt_handler_t *)config->interrupt_handlers;
	kprint("Interrupt Handlers table is located at %p\n", idt_handlers);

	// Install the test key handler
	idt_handlers[0x21] = ps2_keypress_handler;
	kdprint(krnout, "  0x21 -> %p\n", idt_handlers[0x21]);
}