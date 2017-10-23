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

#include <panic.h>
#include <kern_types.h>
#include <kprint.h>

static uintptr_t *panic_handler = NULL;

void register_panic_handler(
	struct boot_config *config,
	void(*handler)(struct panic_info *, struct registers *)
) {
	panic_handler = config->panic_handler;
	*panic_handler = (uintptr_t)handler;
	kdprint(dbgout, "Registered panic handler %p at %p\n", 
		handler, panic_handler);
}

__attribute__((noreturn)) void panic(
	struct panic_info *info, struct registers *registers
) {
	void(*handler)(struct panic_info *, struct registers *) = (void *)(
		*panic_handler
	);

	if (handler)
		handler(info, registers);

	// Report the panic to the serial console and halt the system.
	kdprint(dbgout, "Kernel panic occurred. Halting system now.\n");

	while (1)
		__asm__ __volatile__("cli; hlt");
}
