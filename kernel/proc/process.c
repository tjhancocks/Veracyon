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

#include <process.h>
#include <kheap.h>
#include <kprint.h>
#include <panic.h>
#include <macro.h>
#include <memory.h>
#include <task.h>

#define DEFAULT_STACK_SIZE	16 * 1024	// 16KiB

////////////////////////////////////////////////////////////////////////////////

static struct process *first_process = NULL;
static struct process *last_process = NULL;
static struct process *current_process = NULL;
static uint32_t process_count = 0;
static uint32_t next_pid = 0;

////////////////////////////////////////////////////////////////////////////////

int idle(void)
{
	while (1) {
		__asm__ __volatile__("hlt");
	}
}

void process_prepare(void)
{
	// Create a reference for the kernel and discard the result. We need to 
	// adopt the appropriate information for the process.
	struct process *kernel_proc = process_spawn("kernel", NULL);
	
	// If no kernel process has been spawned then panic. We will be unable to
	// continue.
	if (!kernel_proc) {
		struct panic_info info = (struct panic_info) {
			panic_general,
			"UNABLE TO INITIALISE KERNEL PROCESS",
			"The process header for the kernel could not be created. This is a"
			" serious error."
		};
		panic(&info, NULL);
	}

	// Specify the page directory of the kernel process.
	kernel_proc->page_dir = REGISTER(cr3);
	kdprint(dbgout, "Process %d (%s) established with page directory: %p\n",
		kernel_proc->pid, kernel_proc->name, kernel_proc->page_dir);

	// Spawn the idle process
	struct process *idle_proc = process_spawn("idle", idle);

	// If no idle process has been spawned then panic. It would be unwise to
	// continue.
	if (!idle_proc) {
		struct panic_info info = (struct panic_info) {
			panic_general,
			"UNABLE TO INITIALISE IDLE PROCESS",
			"The process header for the idle process could not be created. This"
			" is a serious error."
		};
		panic(&info, NULL);
	}

	idle_proc->page_dir = REGISTER(cr3);
	kdprint(dbgout, "Process %d (%s) established with page directory: %p\n",
		idle_proc->pid, idle_proc->name, idle_proc->page_dir);

	// Enable multitasking
	task_set_allowed(1);
}

////////////////////////////////////////////////////////////////////////////////

struct process *process_spawn(const char *name, int(*_entry)(void))
{
	kdprint(dbgout, "Spawning new process: %s\n", name);

	// Create a new blank process and prepare to populate it with the 
	// appropriate information.
	struct process *proc = kalloc(sizeof(*proc));
	memset(proc, 0, sizeof(*proc));

	// Basic metadata
	proc->name = name;	// TODO: Copy the name string into the process.
	proc->pid = next_pid++;
	kdprint(dbgout, "   * assigning pid: %d\n", proc->pid);

	// Standard Pipes
	proc->stdin.size = 1024;
	proc->stdin.buffer = kalloc(proc->stdin.size);
	proc->stdin.r_idx = 0;
	proc->stdin.w_idx = 0;
	kdprint(dbgout, "   * stdin pipe created: %d bytes\n", proc->stdin.size);

	// Main thread
	proc->threads.main = process_spawn_thread(proc, "Main Thread", _entry);

	// Insert the process into the process chain/list.
	if (!first_process) {
		first_process = proc;
	}

	proc->prev = last_process;
	if (last_process) {
		last_process->next = proc;
	}
	last_process = proc;

	++process_count;

	return proc;
}

////////////////////////////////////////////////////////////////////////////////

struct thread *process_spawn_thread(
	struct process *owner,
	const char *label, 
	int(*start)(void)
) {
	// Perform a check on the starting point. It must _not_ be NULL unless the
	// owning process is the kernel. Even then the kernel, must not yet have a 
	// thread.
	if (!owner) {
		kdprint(dbgout, "WARNING: Attempting to create an orphaned thread.\n");
		return NULL;
	}
	else if (owner->threads.main && start == NULL) {
		kdprint(dbgout, "WARNING: Attempting to create an invalid thread.\n");
		return NULL;
	}

	// Create the new thread and prepare to configure it.
	struct thread *thread = thread_create(label, start);
	thread->owner = owner;

	// If their is a specified starting point (almost all threads) then we need
	// to create a new stack for the thread.
	if (start) {
		if (thread_stack_init(thread, DEFAULT_STACK_SIZE) == 0) {
			struct panic_info info = (struct panic_info) {
				panic_general,
				"UNABLE TO INITIALISE THREAD STACK",
				"The system was unable to create a new stack for the thread."
			};
			panic(&info, NULL);
		}
	}

	// Create a task for the thread.
	if (task_create(thread) == 0) {
		kdprint(dbgout, "Failed to create task for thread %d.\n", thread->tid);
	}

	// Return the new thread to the caller.
	return thread;
}

////////////////////////////////////////////////////////////////////////////////

struct process *process_current(void)
{
	return current_process;
}

struct process *process_get(uint32_t pid)
{
	struct process *proc = first_process;
	while (proc && proc->pid != pid) {
		proc = proc->next;
	}
	return proc;
}
