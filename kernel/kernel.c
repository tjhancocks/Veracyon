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

#include <boot_config.h>
#include <serial.h>
#include <vga_text.h>
#include <kprint.h>
#include <physical.h>
#include <virtual.h>
#include <kheap.h>
#include <arch/arch.h>
#include <device/keyboard/keyboard.h>

__attribute__((noreturn)) void kwork(void)
{
	while (1) {
		char c = keyboard_get_key();
		kprint("Key press: %c\n", c);
	}
}

__attribute__((noreturn)) void kmain(
	struct boot_config *config
) {
	serial_prepare();

	if (config->vesa_mode == vesa_mode_text) {
		vga_text_prepare(config);
	}

	kprint("VERACYON VERSION 0.1\n");
	kprint(" Copyright (c) 2017 Tom Hancocks. MIT License.\n\n");

	kprint("string test: \"%s\"\n", "Hello, World");
	kprint("signed test: %i/%i\n", -56, 1289);
	kprint("unsigned test: %u\n", 340);
	kprint("hex test: %02x\n", 0xF);
	kprint("pointer test: %p\n", config);

	// Get the memory management of the system/kernel up and running. This will
	// be needed by a lot of the later functionality.
	physical_memory_prepare(config);
	virtual_memory_prepare(config);
	kheap_prepare();

	// Take control of the interrupts system, and get some basic interrupts
	// installed.
	interrupt_handlers_prepare(config);

	// Attempt to install each of the integral device drivers.
	keyboard_driver_prepare();

	kwork();
}