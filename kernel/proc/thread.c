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
#include <memory.h>
#include <device/keyboard/keyboard.h>

static uint32_t next_thread_id = 2;
static struct {
	struct thread *first;
	struct thread *last;
	struct thread *current;
} thread_pool;

static struct thread _kernel_main_thread = (struct thread) {
	.id = ROOT_THREAD_ID,
	.label = "vkernel::root_thread",
	.status = thread_ready,
	.status_info = 0,
	.state = 0,
	.next = NULL,
	.prev = NULL
};

////////////////////////////////////////////////////////////////////////////////

__attribute__((noreturn))
void idle_main(void)
{
	while (1) {
		__asm__ __volatile__("hlt");
	}
}

////////////////////////////////////////////////////////////////////////////////

void thread_halt(void)
{
	force_yield_on_next_interrupt();
	__asm__ __volatile__("hlt");
}


////////////////////////////////////////////////////////////////////////////////

enum thread_status current_thread_status(void)
{
	return thread_pool.current ? thread_pool.current->status : thread_ready;
}

void thread_wait_time(uint64_t ms)
{
	// If there is no current thread, or it is the "idle" thread then ignore.
	if (!thread_pool.current || thread_pool.current->id == 1)
		return;


	thread_pool.current->status = thread_waiting_timer;
	thread_pool.current->status_info = ms;
	__asm__ __volatile__("hlt");
}

void thread_wait_keyevent(void)
{
	// If there is no current thread, or it is the "idle" thread then ignore.
	if (!thread_pool.current || thread_pool.current->id == 1)
		return;


	thread_pool.current->status = thread_waiting_keyevent;
	thread_pool.current->status_info = 0;
	__asm__ __volatile__("hlt");
}

void thread_wait_irq(uint8_t irq)
{
	// If there is no current thread, or it is the "idle" thread then ignore.
	if (!thread_pool.current || thread_pool.current->id == 1)
		return;


	thread_pool.current->status = thread_waiting_irq;
	thread_pool.current->status_info = (uint64_t)irq;
	__asm__ __volatile__("hlt");
}


////////////////////////////////////////////////////////////////////////////////

void threading_prepare(void)
{
	thread_pool.current = &_kernel_main_thread;
	thread_pool.first = thread_pool.current;
	thread_pool.last = thread_pool.last;

	kdprint(dbgout, "Established root thread as TID::%d [%s]\n",
		thread_pool.current->id, thread_pool.current->label);
}

void establish_idle_thread(void)
{
	struct thread *idle_thread = thread_spawn("idle", idle_main);
	idle_thread->id = IDLE_THREAD_ID;
	idle_thread->status = thread_ready;
}


////////////////////////////////////////////////////////////////////////////////

void *kalloc_stack(uint32_t size, uint32_t eip)
{
	uint32_t *stack = (uint32_t *)kalloc(size * sizeof(*stack));
	memset(stack, 0, size * sizeof(*stack));

	// We need to construct the initial values of the stack.
	stack[size - 1]  = 0x10;			// SS
	stack[size - 2]  = 0x00;			// USER ESP
	stack[size - 3]  = 0x208;			// EFLAGS
	stack[size - 4]  = 0x08;			// CS
	stack[size - 5]  = eip;				// EIP
	stack[size - 6]  = 0x00;			// ERROR CODE
	stack[size - 7]  = 0x00;			// INTERRUPT
	stack[size - 8]  = 0x00;			// EDI
	stack[size - 9]  = 0x00;			// ESI
	stack[size - 10] = (uint32_t)stack;	// EBP
	stack[size - 11] = 0x00;			// ESP
	stack[size - 12] = 0x00;			// EBX
	stack[size - 13] = 0x00;			// EDX
	stack[size - 14] = 0x00;			// ECX
	stack[size - 15] = 0x00;			// EAX
	stack[size - 16] = 0x10;			// DS
	stack[size - 17] = 0x10;			// ES
	stack[size - 18] = 0x10;			// FS
	stack[size - 19] = 0x10;			// GS

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

	if (thread->id > 0) {
		thread->state.ebp = (uint32_t)kalloc_stack(
			0x1000, 
			(uint32_t)thread_main
		);
		thread->state.esp = thread->state.ebp - (19 * sizeof(uint32_t));
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

struct thread *next_ready_thread(void)
{
	struct thread *candidate = thread_pool.current->next ?: thread_pool.first;
	
	while (candidate != thread_pool.current) {
		
		// Is the candidate ready? If so then use it.
		if (candidate->status == thread_ready) {
			return candidate;
		}

		// Is the candidate waiting on a keyevent? If so check the keyboard
		// buffer for events. If there are available events then mark the 
		// thread as ready, and switch to it.
		if (candidate->status == thread_waiting_keyevent && 
			keyboard_buffer_has_items() > 0) 
		{
			goto MARK_READY_AND_RETURN;
		}

		// There was no way to bring the thread into scope. Move to the next 
		// one.
		candidate = candidate->next ?: thread_pool.first;
	}

	// At this point we just want to fall back on the next thread.
	candidate = thread_pool.current->next ?: thread_pool.first;
	return candidate;

MARK_READY_AND_RETURN:
	candidate->status = thread_ready;
	return candidate;
}

extern void switch_stack(uint32_t esp, uint32_t ebp);

void perform_yield_on_interrupt(struct interrupt_frame *frame)
{
	// Only allow a yield if we're in multitasking mode!
	if (!thread_pool.current || thread_pool.first == thread_pool.last)
		return;

	// Get the next thread. If it is the same as the current thread, or not
	// ready, then abort.
	struct thread *next_thread = next_ready_thread();
	if (next_thread == thread_pool.current || 
		next_thread->status != thread_ready)
	{
		return;
	}

	// Perform the stack switch. For this we need to calculate the required
	// future position of the current stack. This involves a small amount of
	// calculation based on the frame.
	thread_pool.current->state.ebp = frame->ebp;
	thread_pool.current->state.esp = (uint32_t)frame;
	thread_pool.current = next_thread;

	// The next thread should already have an appropriate stack for switching
	// to. If it does not, then this will cause a panic.
	switch_stack(next_thread->state.esp, next_thread->state.ebp);
}
