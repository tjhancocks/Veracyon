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

#ifndef __VKERNEL_THREAD__
#define __VKERNEL_THREAD__

#include <kern_types.h>
#include <arch/arch.h>

struct thread;

enum thread_status 
{
	thread_ready,
	thread_suspended,
	thread_terminated,
	thread_waiting_keyevent,
	thread_waiting_timer,
};

struct thread_registers 
{
	uint32_t esp;
	uint32_t ebp;
	uint32_t cr3;
};

struct thread 
{
	uint32_t id;
	const char *label;
	enum thread_status status;
	struct thread_registers state;
	struct thread *next;
	struct thread *prev;
};

/**
 Prepare the initial threading and multitasking environment ready for use.
 */
void threading_prepare(void);

/**
 Spawn a new thread with the specified label and entry point.
 */
struct thread *thread_spawn(const char *label, void(*thread_main)(void));

/**
 Yield the current thread for a new thread.

 WARNING: This should only be called from an interrupt.
 */
void perform_yield_on_interrupt(struct interrupt_frame *frame);

/**
 Report the status of the current process.
 */
enum thread_status current_thread_status(void);

/**
 Suspend the current thread for the specified period of time (milliseconds)
 */
void thread_wait_time(uint32_t ms);

/**
 Suspend the current thread until a keyevent is received.
 */
void thread_wait_keyevent(void);



void describe_frame(struct interrupt_frame *frame);

#endif