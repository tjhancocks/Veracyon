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

#ifndef __VKERNEL_THREAD__
#define __VKERNEL_THREAD__

#include <stdint.h>
#include <arch/arch.h>
#include <process.h>

enum thread_mode
{
	thread_running,
	thread_paused,
	thread_blocked,
	thread_killed,
};

enum thread_mode_reason
{
	reason_none,
	reason_irq_wait,
	reason_key_wait,
	reason_sleep,
	reason_process,
	reason_exited,
};

struct thread
{
	uint32_t tid;
	const char *label;
	struct process *owner;
	struct {
		uint32_t esp;
		uint32_t ebp;
	} stack;
	struct {
		enum thread_mode mode;
		enum thread_mode_reason reason;
		uint64_t info;
	} state;
	int(*start)(void);
};

/**
 Construct a new thread object.
 */
struct thread *thread_create(const char *label, int(*start)(void));

/**
 Initialise a new stack for the specified thread.
 */
int thread_stack_init(struct thread *thread, uint32_t size);

/**
 Put the current thread to sleep for the specified period of time (milliseconds)
 */
void sleep(uint64_t ms);

/**
 Sleep the thread until a keyboard event is received. Any keyboard will resume
 the thread.
 */
void key_wait(void);

#endif