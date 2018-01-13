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

#ifndef __VKERNEL_TASK__
#define __VKERNEL_TASK__

#include <kern_types.h>
#include <thread.h>
#include <arch/arch.h>

struct task
{
	struct thread *thread;
	struct task *prev;
	struct task *next;
};

/**
 Is multitasking enabled?
 */
int task_allowed(void);

/**
 Toggle the allowed status of multitasking
 */
void task_set_allowed(int flag);

/**
 Setup a new task. This will schedule the specified thread in the multitasking
 environment.

 RETURN: 0 - if the task could not be created. Any other value if the task was
 created.
 */
int task_create(struct thread *thread);

/**
 Yield the current task. This can only be done in an interrupt frame.
 */
void yield(struct interrupt_frame *frame);

/**
 Returns the currently executing task.
 */
struct task *task_get_current(void);

/**
 Resume all relavent tasks that have been blocked due to the specified reason.
 */
void task_resume_any_for(enum thread_mode_reason reason, uint64_t info);

#endif