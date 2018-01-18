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
#include <macro.h>

static const char *exception_name[] = {
	"Divide-by-zero Error",
	"Debug",
	"Non-maskable Interrupt",
	"Breakpoint",
	"Overflow",
	"Bound Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"Reserved",
	"x87 Floating-Point Exception",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Security Exception",
	"Reserved"
};

static uintptr_t *panic_handler = NULL;

////////////////////////////////////////////////////////////////////////////////

void render_register(const char *name, uint32_t value, uint32_t x, uint32_t y)
{
	kprint("\t\033[94m%8s: ", name);
	kprint("\033[96m%08x\n", value);
}

__attribute__((noreturn)) void panic(
	struct panic_info *info, struct interrupt_frame *frame
) {
	
	// If there is no info structure, then we need to construct one.
	if (!info && frame && frame->interrupt < 0x20) {
		struct panic_info default_info = (struct panic_info) {
			panic_error,
			exception_name[frame->interrupt],
			"An unrecoverable exception occured in the CPU. "
			"Halting system immediately."
		};
		info = &default_info;
	}

	// We need to clear the screen and get back into a basic terminal 
	// presentation.
	kprint("\033[44m\033[2J\n");

	if (frame && frame->interrupt < 0x20)
		kprint(" CPU Exception:");
	kprint("\033[97m %s\n", info->title);

	// The message is the next bit of information to be displayed. This needs
	// be printed so that it can word wrap to subsequent in a clean way.
	// TODO: Word wrapping.
	kprint("\033[94m %s\n\n", info->message);

	// Finally we want to start displaying register information. This will help
	// with debugging and knowing state.
	if (frame) {
		uint32_t y = 0;

		render_register("EAX", frame->eax, 2 + (18 * 0), y+2);
		render_register("EBX", frame->ebx, 2 + (18 * 1), y+2);
		render_register("ECX", frame->ecx, 2 + (18 * 2), y+2);
		render_register("EDX", frame->edx, 2 + (18 * 3), y+2);

		render_register("ESI", frame->esi, 2 + (18 * 0), y+3);
		render_register("EDI", frame->edi, 2 + (18 * 1), y+3);
		render_register("ESP", frame->esp, 2 + (18 * 2), y+3);
		render_register("EBP", frame->ebp, 2 + (18 * 3), y+3);

		uint32_t cr0 = REGISTER(cr0);
		uint32_t cr2 = REGISTER(cr2);
		uint32_t cr3 = REGISTER(cr3);
		uint32_t cr4 = REGISTER(cr4);
		render_register("CR0", cr0, 2 + (18 * 0), y+5);
		render_register("CR2", cr2, 2 + (18 * 1), y+5);
		render_register("CR3", cr3, 2 + (18 * 2), y+5);
		render_register("CR4", cr4, 2 + (18 * 3), y+5);

		render_register("GS", frame->gs & 0xFFFF, 2 + (18 * 0), y+6);
		render_register("FS", frame->fs & 0xFFFF, 2 + (18 * 1), y+6);
		render_register("ES", frame->es & 0xFFFF, 2 + (18 * 2), y+6);
		render_register("DS", frame->ds & 0xFFFF, 2 + (18 * 3), y+6);

		render_register("SS", frame->ss & 0xFFFF, 2 + (18 * 0), y+7);
		render_register("CS", frame->cs & 0xFFFF, 2 + (18 * 1), y+7);

		render_register("EIP", frame->eip, 2 + (18 * 0), y+9);
		render_register("EFLAGS", frame->eflags, 2 + (18 * 1), y+9);
	}
	

	// Make sure we don't return or process anything further.
	while (1)
		__asm__ __volatile__("cli; hlt");
}

////////////////////////////////////////////////////////////////////////////////

void prepare_panic_handler(
	struct boot_config *config
) {
	panic_handler = config->panic_handler;
	*panic_handler = (uintptr_t)panic;
	kdprint(COM1, "Registered panic handler %p at %p\n", 
		*panic_handler, panic_handler);
}