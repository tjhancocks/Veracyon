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

#include <thread.h>
#include <kheap.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <memory.h>
#include <panic.h>
#include <macro.h>
#include <task.h>
#include <uptime.h>
#include <atomic.h>

////////////////////////////////////////////////////////////////////////////////

static uint32_t next_tid = 0;

////////////////////////////////////////////////////////////////////////////////

struct thread *thread_create(const char *label, int(*start)(void))
{
	fprintf(COM1, "Creating new thread: %s\n", label ?: "(unnamed)");

	// Construct the basic thread.
	struct thread *thread = kalloc(sizeof(*thread));
	memset(thread, 0, sizeof(*thread));

	// Basic thread metadata
	thread->tid = next_tid++;
	thread->label = label ?: "";

	// Configure the thread state. Make sure it is ready to be switched in by
	// the scheduler.
	thread->state.mode = thread_running;
	thread->state.reason = 0;
	thread->state.info = 0;

	thread->start = start;

	fprintf(COM1, "   * assigning tid: %d\n", thread->tid);
	return thread;
}

////////////////////////////////////////////////////////////////////////////////

__attribute__((noreturn))
static void _thread_start(void)
{
	// Get the thread from the current task. It is the context that we require.
	struct thread *this = task_get_current()->thread;

	// Make sure that a thread instance has been provided. If it has not then,
	// panic. We're executing a task that is invalid.
	if (!this || !this->start) {
		struct panic_info info = (struct panic_info) {
			panic_general,
			"INVALID THREAD STARTED",
			"The context provided to _thread_start was invalid."
		};
		panic(&info, NULL);
	}

	fprintf(COM1, "Starting thread execution: %d (%p)\n",
		this->tid, this);

	// Call the main function of the thread. We should remain inside this
	// function until the thread is ready to terminate.
	this->start();

	// Mark the thread as terminated. This will prevent the scheduler from
	// switching to it and leave it marked for removal.
	this->state.mode = thread_killed;
	this->state.reason = reason_exited;

	// Enter an infinite loop, so that we don't fall out of the bottom of the 
	// stack.
	while (1)
		__asm__ __volatile__("hlt");
}

////////////////////////////////////////////////////////////////////////////////

int thread_stack_init(struct thread *thread, uint32_t size)
{
	// Ensure the thread reference that we were given is actually correct.
	if (!thread)
		return false;

	// Construct a new stack
	uint32_t *stack = kalloc(size * sizeof(*stack));
	uint32_t off = 1;
	memset(stack, 0, size * sizeof(*stack));

	fprintf(COM1, "Initialising stack for thread: %d (%p)\n",
		thread->tid, thread);

	// The _thread_start function needs parameters to exist on the stack.
	stack[size - (off++)] = (uint32_t)thread;
	stack[size - (off++)] = 0x00;

	// We also need to setup a interrupt frame. We'll be switching to this
	// thread via an interrupt, so the interrupt needs to be able to return
	// successfully without raising a fault.
	stack[size - (off++)] = 0x10;						// SS
	stack[size - (off++)] = 0x00;						// UESP
	stack[size - (off++)] = 0x208;						// EFLAGS
	stack[size - (off++)] = 0x08; 						// CS
	stack[size - (off++)] = (uint32_t)_thread_start;	// EIP
	stack[size - (off++)] = 0x00;						// ERROR CODE
	stack[size - (off++)] = 0x00;						// INTERRUPT
	stack[size - (off++)] = 0x00;						// EDI
	stack[size - (off++)] = 0x00;						// ESI
	stack[size - (off++)] = (uint32_t)stack;			// EBP
	stack[size - (off++)] = 0x00;						// ESP (ignored)
	stack[size - (off++)] = 0x00;						// EBX
	stack[size - (off++)] = 0x00;						// EDX
	stack[size - (off++)] = 0x00;						// ECX
	stack[size - (off++)] = 0x00;						// EAX
	stack[size - (off++)] = 0x10;						// DS
	stack[size - (off++)] = 0x10;						// ES
	stack[size - (off++)] = 0x10;						// FS
	stack[size - (off)] = 0x10;							// GS

	// Setup the stack information in the thread.
	thread->stack.ebp = (uint32_t)stack + (size * sizeof(*stack));
	thread->stack.esp = thread->stack.ebp - (off * sizeof(*stack));

	return true;
}

////////////////////////////////////////////////////////////////////////////////

void sleep(uint64_t ms)
{
	// If the duration is zero then cancel.
	if (ms == 0)
		return;

	// Fetch the current thread and mark it as suspended. Specify the reason, 
	// and the resume time.
	atom_t atom;
	atomic_start(atom);

	struct thread *current = task_get_current()->thread;

	current->state.info = get_uptime_ms() + ms;
	current->state.reason = reason_sleep;
	current->state.mode = thread_paused;

	// Indicate to the system that we need to be preempted now.
	request_preemption();

	atomic_end(atom);

	// Wait until the thread is marked as running before resuming. Once it is,
	// break from the loop and continue on.
	while (current->state.mode != thread_running)
		__asm__ __volatile__("hlt");
}

void key_wait(void)
{
	// Fetch the current thread and mark it as blocked. Specify the reason, 
	// and the resume condition.
	atom_t atom;
	atomic_start(atom);

	struct thread *current = task_get_current()->thread;

	current->state.reason = reason_key_wait;
	current->state.mode = thread_blocked;

	// Indicate to the system that we need to be preempted now.
	request_preemption();

	atomic_end(atom);

	// Wait until the thread is marked as running before resuming. Once it is,
	// break from the loop and continue on.
	while (current->state.mode != thread_running)
		__asm__ __volatile__("hlt");
}

