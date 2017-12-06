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
#include <vesa_text.h>
#include <kprint.h>
#include <physical.h>
#include <virtual.h>
#include <kheap.h>
#include <arch/arch.h>
#include <device/keyboard/keyboard.h>
#include <panic.h>
#include <modules/shell.h>


__attribute__((noreturn)) void kwork(void)
{
	while (1) {
		__asm__ __volatile__(
			"hlt"
		);
	}
}

__attribute__((noreturn)) void kmain(
	struct boot_config *config
) {
	// We need the debug serial port to be enabled before anything else so that
	// we can get debug logs for the kernel.
	serial_prepare();

	// Get the appropriate display driver in place.
	if (config->vesa_mode == vga_mode_text) {
		vga_text_prepare(config);
	}
	else if (config->vesa_mode == vesa_mode_text) {
		vesa_text_prepare(config);
	}
	else {
		struct panic_info info = (struct panic_info) {
			panic_general,
			"UNRECOGNISED DISPLAY CONFIGURATION",
			"The specified type of display configuration is not recognised."
		};
		panic(&info, NULL);
	}

	// Make sure we have a panic handler in place before starting on the meat of
	// the kernel.
	prepare_panic_handler(config);

	// Some basic information to be shown to the user.
	kprint("\033[96mVERACYON VERSION 0.1\033[0m\n");
	kprint("\033[90m Copyright (c) 2017 Tom Hancocks. MIT License.\033[0m\n\n");

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

	// Launch a debug kernel shell. This is a blocking function and will prevent
	// the kernel from launching any further until the shell has exited.
	init_shell();
	kprint("\n\033[31mKernel shell exited successfully.\n");

	kwork();
}