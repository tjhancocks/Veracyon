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

#include <process.h>
#include <kheap.h>
#include <kprint.h>
#include <macro.h>
#include <panic.h>

#define THREAD_STACK_SIZE	0x1000	// 4KiB

static struct process_block *process_list;
static struct process_block *current_process;
static uint32_t process_count = 0;
static uint32_t next_process_id = 0;

static struct thread_block *thread_list;
static struct thread_block *current_thread;
static uint32_t thread_count = 0;
static uint32_t next_thread_id = 0;

////////////////////////////////////////////////////////////////////////////////

static struct thread_block *thread_spawn(struct process_block *owner)
{
	kdprint(dbgout, "Spawning new basic thread for process: %s\n",
		owner->name);

	// Allocate the thread.
	struct thread_block *thread = kalloc(sizeof(*thread));
	thread->id = next_thread_id++;
	thread->next = thread_list;
	thread->prev = NULL;

	// Bind the thread the specified process.
	thread->process = owner;
	owner->threads++;

	// Add to the thread pool.
	if (thread_list)
		thread_list->prev = thread;

	// Allocate stack space for the thread, unless this is the first thread to
	// be allocated.
	if (thread->id == 0) {
		thread->registers.esp = REGISTER(esp);
		thread->registers.ebp = REGISTER(ebp);
		kdprint(dbgout, "Adopting current stack at: %p\n",
			thread->registers.ebp);
	}
	else {
		thread->registers.esp = (uint32_t)kalloc(THREAD_STACK_SIZE)
							  + THREAD_STACK_SIZE;
		thread->registers.ebp = thread->registers.esp;
		kdprint(dbgout, "Allocated stack at: %p (%d bytes)\n",
			thread->registers.ebp, THREAD_STACK_SIZE);
	}
	
	// Inject the thread into the thread pool and return the new thread to 
	// the caller.
	++thread_count;
	return (thread_list = thread);
}


////////////////////////////////////////////////////////////////////////////////

struct process_block *process_spawn(void)
{
	kdprint(dbgout, "Spawning blank process\n");

	struct process_block *proc = kalloc(sizeof(*proc));
	proc->id = next_process_id++;
	proc->name = "<unnamed>";

	// Use the current page directory by default. At some point this will
	// clone the current page directory.
	proc->page_directory = REGISTER(cr3);

	// Prepare the main thread for the process. 
	proc->main_thread = thread_spawn(proc);
	proc->current_thread = proc->main_thread;

	// Hook the process up to the process list.
	proc->next = process_list;
	proc->prev = NULL;

	if (process_list)
		process_list->prev = proc;

	process_list = proc;
	++process_count;

	// Return the process to the caller.
	kdprint(dbgout, "Finished spawning process %d\n", proc->id);
	return proc;
}

void process_spawn_root(void)
{
	if (process_list || current_process || process_count) {
		struct panic_info info = (struct panic_info) {
			panic_general,
			"MULTIPLE SPAWN OF ROOT PROCESS",
			"Attempted to spawn the root process multiple times."
		};
		panic(&info, NULL);
	}

	kdprint(dbgout, "Establishing root process\n");

	// Root process block. Much information will be populated at a later point.
	struct process_block *root = process_spawn();
	root->name = "root";

	kdprint(dbgout, "Root process successfully created.\n");
}


////////////////////////////////////////////////////////////////////////////////

void list_processes(void)
{
	kprint("\n\033[97m %5s %-20s %-8s\033[0m\n", "ID", "Name", "Threads");

	struct process_block *process = process_list;
	while (process) {
		kprint(" %5d %-20s %-8d\n", 
			process->id, process->name, process->threads);
		process = process->next;
	}

	kprint("\n");
}