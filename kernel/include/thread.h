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
	thread_terminated
};

struct thread_registers 
{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eflags;
	uint32_t cr3;
	uint32_t eip;
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

 */
struct thread *thread_spawn(const char *label, void(*thread_main)(void));

/**

 */
void perform_yield_on_interrupt(struct interrupt_frame *frame);

void describe_frame(struct interrupt_frame *frame);

#endif