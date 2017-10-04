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

unsigned char inb(unsigned short port)
{
	unsigned char value = 0;
	__asm__ __volatile__(
		"inb %1, %0"
		: "=a"(value)
		: "dN"(port)
	);
	return value;
}

void outb(unsigned short port, unsigned char value)
{
	__asm__ __volatile__(
		"outb %1, %0"
		:: "dN"(port), "a"(value)
	);
}

void kserial_send(const char *str)
{
	while (*str) {
		while ((inb(0x3f8 + 5) & 0x20) == 0);
		outb(0x3f8, *str++);
	}
}

__attribute__((noreturn)) void kwork(void)
{
	while (1) {
		__asm__ __volatile(
			"hlt\n"
			"nop\n"
		);
	}
}

__attribute__((noreturn)) void kmain(void)
{
	kserial_send("Hello from vkernel!\n");
	kwork();
}