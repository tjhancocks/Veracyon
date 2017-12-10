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

#ifndef __VKERNEL_PROCESS__
#define __VKERNEL_PROCESS__

#include <kern_types.h>

struct thread_block;
struct process_block;

struct process_registers {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eip;
	uint32_t eflags;
	uint32_t cr3;
} __attribute__((packed));

struct thread_block {
	uint32_t id;
	struct process_registers registers;
	struct process_block *process;
	struct thread_block *next;
	struct thread_block *prev;
};

struct process_block {
	uint32_t id;
	const char *name;
	uint32_t page_directory;
	struct thread_block *main_thread;
	struct thread_block *current_thread;
	uint32_t threads;
	struct process_block *next;
	struct process_block *prev;
};

/**
 Create the root process. This is the root process of the system and is 
 needed to help coordinate system calls later.

 This will also establish the presence of the main thread in the root process.
 */
void process_spawn_root(void);

/**
 Render a list of all processes out to the debug shell. This includes ID's, 
 Name's and Thread counts.
 */
void list_processes(void);

#endif