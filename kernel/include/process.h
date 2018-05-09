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

#ifndef __VKERNEL_PROCESS__
#define __VKERNEL_PROCESS__

#include <stdint.h>
#include <thread.h>
#include <pipe.h>

enum process_launch_flags
{
	// The process is a root/kernel level agent and doesn't accept user input.
	P_ROOT = 1 << 0,

	// The process should exist within user-space.
	P_USR = 1 << 1,

	// The process should exist as an UI agent, accepting user input.
	P_UI = 1 << 2
};

struct process 
{
	uint32_t pid;
	const char *name;
	uintptr_t page_dir;
	int allow_frontmost;
	uint32_t switched_out;

	struct {
		size_t count;
		struct pipe **pipe;
	} pipe;
	
	struct {
		struct thread *main;
	} threads;

	struct process *next;
	struct process *prev;
};

/**
 Prepare basic process environment and configuration.
 */
void process_prepare(void);

/**
 Launch a new process with the specified name, starting location and 
 launch flags.

 NOTE: This is the preferred method of launching processes.
 */
struct process *process_launch(
	const char *name,
	int(*_entry)(void),
	enum process_launch_flags flags
);

/**
 Spawn a new process with the specified meta-data. A newly spawned process
 will be automatically given a new main thread. This main thread will be placed
 at the specified entry function.

 NOTE: The first spawned process will be the kernel, and will adopt all current
 kernel configurations.
 */
struct process *process_spawn(const char *name, int(*_entry)(void));

/**
 Spawn a new thread inside the specified process.

 WARNING: Currently processes can only own a single thread. The kernel will
 currently panic if a secondary thread is spawned.
 */
struct thread *process_spawn_thread(
	struct process *owner,
	const char *label, 
	int(*thread)(void)
);

/**
 Get a pointer to the frontmost process. This will be the kernel if there is no
 front most process currently.
 */
struct process *process_get_frontmost(void);

/**
 Get a pointer to the key process. This will be the kernel if there is no key
 process currently.
 */
struct process *process_get_key(void);

/**
 Get a pointer to the process with the specified pid.

 - pid: The process ID to look up.
 */
struct process *process_get(uint32_t pid);

/**
 Establish a pipe between the two specified processes
 */
void process_make_pipe(
	struct process *owner, 
	struct process *target, 
	enum pipe_purpose mask
);

#endif