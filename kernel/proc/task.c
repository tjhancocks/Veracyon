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

#include <task.h>
#include <kheap.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>
#include <panic.h>
#include <uptime.h>

////////////////////////////////////////////////////////////////////////////////

static struct task *first_task = NULL;
static struct task *last_task = NULL;
static struct task *current_task = NULL;
static uint32_t task_count = 0;
static int allowed = 0;

////////////////////////////////////////////////////////////////////////////////

static struct task *task_get_next(void);
extern void switch_stack(uint32_t esp, uint32_t ebp);

////////////////////////////////////////////////////////////////////////////////

void task_set_allowed(int flag)
{
	fprintf(COM1, "%sabling multitasking\n", flag ? "En" : "Dis");
	allowed = flag;
}

int task_allowed(void)
{
	return allowed;
}

////////////////////////////////////////////////////////////////////////////////

int task_create(struct thread *thread)
{
	// Make sure a thread has actually been specified first. The thread must
	// also have an owning process.
	if (!thread || !thread->owner)
		return 0;

	fprintf(COM1, "* Creating task for thread %d\n", thread->tid);

	struct task *task = kalloc(sizeof(*task));
	memset(task, 0, sizeof(*task));

	task->thread = thread;
	task->prev = last_task;

	if (last_task) {
		last_task->next = task;
	}
	last_task = task;

	if (!first_task) {
		first_task = task;
		current_task = task;
	}

	++task_count;

	return 1;
}

////////////////////////////////////////////////////////////////////////////////

void yield(struct interrupt_frame *frame)
{
	// We shouldn't even attempt this until we have multiple tasks available!
	if (allowed == 0 || task_count <= 0)
		return;

	struct task *next = task_get_next();

	// If no task is available to switch to, or its the same as the current task
	// then abort.
	if (!next || next == current_task)
		return;

	// We've got a task to switch to. First we need to update the current task,
	// so that the stack is remembered.
	current_task->thread->stack.esp = (uint32_t)frame;
	current_task->thread->stack.ebp = frame->ebp;
	current_task->thread->owner->switched_out++;
	current_task = next;

	// Perform the switch. If anything has been misconfigured here, we'll be in
	// crash land before we know it.
	switch_stack(next->thread->stack.esp, next->thread->stack.ebp);
}

////////////////////////////////////////////////////////////////////////////////

static int task_can_resume(struct task *task)
{
	// If no task specified then cancel. Makes no sense.
	if (!task)
		return 0;
	else if (task->thread->state.mode == thread_running)
		return 1;
	else if (task->thread->state.mode == thread_killed)
		return 0;
	else if (task->thread->state.mode == thread_blocked)
		return 0;

	// Paused threads need to be inspected for their resuming conditions. 
	// Check to see if any of them have met their conditions. Blocked threads
	// should be ignored. They need to be specifically woken up (typically by
	// I/O tasks.)
	switch (task->thread->state.reason) {
		case reason_sleep:
			return (get_uptime_ms() >= (suseconds_t)task->thread->state.info);

		case reason_process:
		case reason_irq_wait:
		case reason_key_wait:
			task->thread->state.reason = thread_blocked;
			return 0;

		case reason_exited:
			task->thread->state.reason = thread_killed;
		case reason_none:
		default:
			return 0;
	}

	// If we reach this point then we know that the task can not resume.
	return 0;
}

void task_resume_any_for(enum thread_mode_reason reason, uint64_t info)
{
	// Step through all tasks and mark all that fit the specified reason/info
	// as running.
	struct task *task = first_task;
	do {
		if (task->thread->state.mode != thread_blocked)
			continue;
		else if (task->thread->state.reason != reason)
			continue;
		else if (task->thread->state.info != info)
			continue;

		task->thread->state.mode = thread_running;
		task->thread->state.reason = reason_none;
		task->thread->state.info = 0;
	} 
	while ((task = task->next));
}

////////////////////////////////////////////////////////////////////////////////

struct task *task_get_current(void)
{
	return current_task;
}

static struct task *task_get_next(void)
{
	// Get a reference to the current task. We'll then need to step along the
	// task chain, until we've inspected each task, and check each one for its
	// availability to resume. If they can be resumed then we should mark it as
	// resumed. We're looking for the first task that can be resumed.
	// TODO: Potentially factor in time since last run. 
	struct task *task = task_get_current();
	uint32_t tasks_remaining = task_count;

	do {
		--tasks_remaining;
		task = task->next ?: first_task;
		
		if (task_can_resume(task)) {
			task->thread->state.mode = thread_running;
			task->thread->state.reason = reason_none;
			task->thread->state.info = 0;
			return task;
		}
	}
	while (task && tasks_remaining > 0);

	return task;
}
