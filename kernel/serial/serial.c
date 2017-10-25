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

#include <kern_types.h>
#include <null.h>
#include <serial.h>
#include <arch/arch.h>
#include <term.h>

#define COM1_PORT 0x3F8

int serial_fifo_ready(void)
{
	return (inb(COM1_PORT + 5) & 0x20);
}

void kputc_serial(const char c)
{
	while (serial_fifo_ready() == 0);
	outb(COM1_PORT, c);
}

void kputs_serial(const char *restrict str)
{
	while (str && *str)
		kputc_serial(*str++);
}

void serial_prepare(void)
{
	kputs_serial("Preparing serial port for Kernel... ");
	term_bind_putc(dbgout, kputc_serial);
	term_bind_puts(dbgout, kputs_serial);
	term_puts(dbgout, "done.\n");
}
