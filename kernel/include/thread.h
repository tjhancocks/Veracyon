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

#define ROOT_THREAD_ID	0
#define IDLE_THREAD_ID 	1

struct thread;

enum thread_status 
{
	thread_ready,
	thread_suspended,
	thread_terminated,
	thread_waiting_keyevent,
	thread_waiting_timer,
	thread_waiting_irq,
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
	uint64_t status_info;
	struct thread_registers state;
	struct thread *next;
	struct thread *prev;
};

/**
 Prepare the initial threading and multitasking environment. This will ensure
 the main kernel thread is constructed, ready and current.

 NOTE: This does not mean that the kernel will begin context switching 
 immediately. Context switching will not start until the second thread is
 constructed.
 */
void threading_prepare(void);

/**
 Construct an idle thread. This is a fallback thread that is used whenever all
 other threads are busy or waiting for events to occur.

 NOTE: The idle thread will typically be ID 1, and upon creation enable context
 switching in the kernel.
 */
void establish_idle_thread(void);

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
 Returns a reference to the current thread.
 */
struct thread *current_thread(void);

/**
 Report the status of the current process.
 */
enum thread_status current_thread_status(void);

/**
 Suspend the current thread for the specified period of time (milliseconds)
 */
void thread_wait_time(uint64_t ms);

/**
 Suspend the current thread until a keyevent is received.
 */
void thread_wait_keyevent(void);

/**
 Suspend the current thread until a keyevent is received.
 */
void thread_wait_irq(uint8_t irq);

/**
 Halt the current thread until it is awoken by an interrupt. This will cause a
 yield.
 */
void thread_halt(void);

#endif