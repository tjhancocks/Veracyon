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

#include <sema.h>
#include <thread.h>
#include <arch/arch.h>
#include <kprint.h>

static inline int atomic_swap(volatile int *x, int v)
{
	__asm__ __volatile__(
		"xchg %0, %1"
		: "=r"(v), "=m"(*x)
		: "0"(v)
		: "memory"
	);
	return v;
}

static inline void atomic_store(volatile int *p, int x)
{
	__asm__ __volatile__(
		"movl %1, %0"
		: "=m"(*p)
		: "r"(x)
		: "memory"
	);
}

static inline void atomic_inc(volatile int *x)
{
	__asm__ __volatile__(
		"lock;"
		"incl %0"
		: "=m"(*x)
		: "m"(*x)
		: "memory"
	);
}

static inline void atomic_dec(volatile int *x)
{
	__asm__ __volatile__(
		"lock;"
		"decl %0"
		: "=m"(*x)
		: "m"(*x)
		: "memory"
	);
}

void spin_wait(volatile int *address, volatile int *waiters)
{
	if (waiters)
		atomic_inc(waiters);

	while (*address)
		thread_halt();

	if (waiters)
		atomic_dec(waiters);
}

void spin_lock(spin_lock_t lock)
{
	while (atomic_swap(lock, 1))
		spin_wait(lock, lock+1);
}

void spin_unlock(spin_lock_t lock)
{
	if (lock[0]) {
		atomic_store(lock, 0);
		if (lock[1])
			thread_halt();
	}
}

void spin_init(spin_lock_t lock)
{
	lock[0] = 0;
	lock[1] = 0;
}