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

#include <device/RS232/RS232.h>
#include <arch/arch.h>

////////////////////////////////////////////////////////////////////////////////

#define COM1_PORT 0x3F8

static struct device __rs232 = { 0 };

////////////////////////////////////////////////////////////////////////////////

void rs232_write(struct device *dev __attribute__((unused)), uint8_t c)
{
	outb(COM1_PORT, c);
}

int rs232_ready(struct device *dev __attribute__((unused)))
{
	return (inb(COM1_PORT + 5) & 0x20);
}

////////////////////////////////////////////////////////////////////////////////

void rs232_prepare(void)
{
	// We need to configure the RS232 device.
	__rs232.dev_id = device_next_id;
	__rs232.name = "COM1";
	__rs232.kind = device_com1;
	__rs232.opts = DP_WRITE | DP_ATOMIC_WRITE;
	__rs232.write_byte = rs232_write;
	__rs232.can_write = rs232_ready;

	// Now initialise the RS232 port. Make sure it has the correct configuration
	// TODO: We're still relying on the boot loader to have done this for us.

	// Bind the device to the appropriate handle.
	device_bind(&COM1, &__rs232);
}

struct device *RS232_get_device(void)
{
	return &__rs232;
}