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
#include <drawing/base.h>

#include <device/device.h>
#include <device/RS232/RS232.h>
#include <device/VT100/VT100.h>


__attribute__((noreturn)) void kwork(void)
{
	// Some basic information to be shown to the user.
	kprint("\033[96mVERACYON VERSION %s\033[0m\n", __BUILD_VERSION__);
	kprint("\033[90mCopyright (c) 2017-2018 Tom Hancocks. MIT License.\033[0m");
	kprint("\n\n");

	for (uint8_t n = 0; n < 8; ++n) {
		kprint("\033[3%dm Text \033[0m\n", n);
	}
	kprint("\033[90m Text \033[0m\n");
	kprint("\033[97m Text \033[0m\n");

	while (1) {
		sleep(50);
	}
}

__attribute__((noreturn)) void kmain(
	struct boot_config *config
) {
	// Install early devices/drivers (Phase 1). These are low level, minimal 
	// support devices that will be required during system setup.
	RS232_prepare();

	// Make sure the architecture is prepared.
	gdt_prepare();

	// Make sure we have a panic handler in place before starting on the meat of
	// the kernel.
	prepare_panic_handler(config);

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

	// Prepare the VESA drawing layer if required
	if (config->vesa_mode == vesa_mode_text) {
		drawing_prepare(config);
		clear_screen(0x00000000);
	}

	// Prepare the VT100 terminal layer.
	VT100_prepare(config);

	// Establish multitasking and processes
	process_prepare();

	// init_shell();
	kwork();
}