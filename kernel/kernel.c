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
#include <thread.h>
#include <process.h>

#include <device/device.h>
#include <device/RS232/RS232.h>

__attribute__((noreturn)) void kwork(void)
{
	while (1) {
		__asm__ __volatile__("hlt");
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
	// Install early devices/drivers (Phase 1). These are low level, minimal 
	// support devices that will be required during system setup.
	rs232_prepare();


	// Make sure we have a panic handler in place before starting on the meat of
	// the kernel.
	prepare_panic_handler(config);

	// Some basic information to be shown to the user.
	kprint("\033[96mVERACYON VERSION %s\033[0m\n", __BUILD_VERSION__);
	kprint("\033[90m Copyright (c) 2017 Tom Hancocks. MIT License.\033[0m\n\n");

	// Get the memory management of the system/kernel up and running. This will
	// be needed by a lot of the later functionality.
	physical_memory_prepare(config);
	virtual_memory_prepare(config);

	// Take control of the interrupts system, and get some basic interrupts
	// installed.
	interrupt_handlers_prepare(config);

	// Attempt to install each of the integral device drivers.
	keyboard_driver_prepare();
	pit_prepare();

	// Establish multitasking and processes
	process_prepare();
	
	init_shell();
	kwork();
}