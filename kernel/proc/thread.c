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

#include <thread.h>
#include <kheap.h>
#include <panic.h>
#include <kprint.h>
#include <arch/arch.h>
#include <serial.h>
#include <macro.h>

static uint32_t next_thread_id = 0;
static struct {
	struct thread *first;
	struct thread *last;
	struct thread *current;
} thread_pool;


////////////////////////////////////////////////////////////////////////////////

__attribute__((noreturn))
void idle_main(void)
{
	while (1) {
		// kputc_serial('I');
		__asm__ __volatile__("hlt");
	}
}

__attribute__((noreturn))
void test_main(void)
{
	while (1) {
		// kputc_serial('T');
		__asm__ __volatile__("hlt");
	}
}


////////////////////////////////////////////////////////////////////////////////

void threading_prepare(void)
{
	struct thread *root_thread = thread_spawn("root", NULL);
	struct thread *idle_thread = thread_spawn("idle", idle_main);
	// struct thread *test_thread = thread_spawn("test", test_main);

	// Configure each thread for use.
	root_thread->status = thread_ready;
	idle_thread->status = thread_ready;
	// test_thread->status = thread_ready;

	// Enable multitasking/threading
	thread_pool.current = root_thread;
}


////////////////////////////////////////////////////////////////////////////////

void *kalloc_stack(uint32_t size, uint32_t eip)
{
	uint32_t *stack = (uint32_t *)kalloc(size * sizeof(*stack));
	memset(stack, 0, size * sizeof(*stack));

	// We need to construct the initial values of the stack.
	stack[size - 1] = 0x10;		// SS
	stack[size - 2] = 0x0;		// USER ESP
	stack[size - 3] = 0x208;	// EFLAGS
	stack[size - 4] = 0x8;		// CS
	stack[size - 5] = eip;		// EIP
	stack[size - 6] = 0x0;		// ERROR CODE
	stack[size - 7] = 0x0;		// INTERRUPT

	// Return what EBP should be.
	return stack + size;
}

struct thread *thread_spawn(const char *label, void(*thread_main)(void))
{
	// Begin constructing the basics of a new thread
	struct thread *thread = kalloc(sizeof(*thread));
	memset(thread, 0, sizeof(*thread));

	thread->label = label;
	thread->id = next_thread_id++;
	thread->status = thread_suspended;
	thread->state.eip = (uint32_t)thread_main;
	thread->state.eflags = 0x208;

	if (thread->id > 0) {
		thread->state.ebp = kalloc_stack(0x1000, thread->state.eip);
		thread->state.esp = thread->state.ebp - (7 * sizeof(uint32_t));
	}

	// Add the thread into the pool.
	thread_pool.first = thread_pool.first ?: thread;

	thread->prev = thread_pool.last;
	if (thread->prev) 
		thread->prev->next = thread;

	thread_pool.last = thread;

	// Finish up and return the thread instance to the caller.
	return thread;
}


////////////////////////////////////////////////////////////////////////////////

void describe_frame(struct interrupt_frame *frame)
{
	kdprint(dbgout, "THREAD: %s\n", thread_pool.current->label);
	kdprint(dbgout, "GS/FS/ES/DS: %08x %08x %08x %08x\n", 
		frame->gs, frame->fs, frame->es, frame->ds);

	kdprint(dbgout, "EDI/ESI/EBP/ESP: %08x %08x %08x %08x\n", 
		frame->edi, frame->esi, frame->ebp, frame->esp);
	kdprint(dbgout, "EBX/EDX/ECX/EAX: %08x %08x %08x %08x\n", 
		frame->ebx, frame->edx, frame->ecx, frame->eax);

	kdprint(dbgout, "INT_NO/ERROR: %08x %08x\n", 
		frame->interrupt, frame->error_code);

	kdprint(dbgout, "EIP/CS: %08x %08x\n", 
		frame->eip, frame->cs);
	kdprint(dbgout, "EFLAGS/USER_ESP/SS: %08x %08x %08x\n", 
		frame->eflags, frame->user_esp, frame->ss);
}

////////////////////////////////////////////////////////////////////////////////

void perform_yield_on_interrupt(struct interrupt_frame *frame)
{
	// Only allow a yield if we're in multitasking mode!
	if (!thread_pool.current)
		return;

	// kdprint(dbgout, "\n==== perform_yield_on_interrupt(%p) ====\n", frame);
	// describe_frame(frame);

	// Save the appropriate values in to the current thread.
	thread_pool.current->state.eax = frame->eax;
	thread_pool.current->state.ebx = frame->ebx;
	thread_pool.current->state.ecx = frame->ecx;
	thread_pool.current->state.edx = frame->edx;
	thread_pool.current->state.edi = frame->edi;
	thread_pool.current->state.esi = frame->esi;
	thread_pool.current->state.esp = frame->esp;
	thread_pool.current->state.ebp = frame->ebp;
	thread_pool.current->state.eip = frame->eip;
	thread_pool.current->state.eflags = frame->eflags;

	// Switch to the next task and store the new state values.
	// kdprint(dbgout, "Switching from \"%s\" to ", thread_pool.current->label);
	thread_pool.current = thread_pool.current->next ?: thread_pool.first;
	// kdprint(dbgout, "\"%s\"\n", thread_pool.current->label);

	frame->eax = thread_pool.current->state.eax;
	frame->ebx = thread_pool.current->state.ebx;
	frame->ecx = thread_pool.current->state.ecx;
	frame->edx = thread_pool.current->state.edx;
	frame->edi = thread_pool.current->state.edi;
	frame->esi = thread_pool.current->state.esi;
	frame->esp = thread_pool.current->state.esp;
	frame->ebp = thread_pool.current->state.ebp;
	frame->eip = thread_pool.current->state.eip;
	frame->eflags = thread_pool.current->state.eflags;

	// describe_frame(frame);
	// kdprint(dbgout, "================================================\n");
}
