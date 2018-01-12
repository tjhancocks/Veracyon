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

#ifndef __VKERNEL_ATOMIC__
#define __VKERNEL_ATOMIC__

#include <kern_types.h>
#include <macro.h>

typedef uint32_t atom_t;

#ifndef atomic_start
#define atomic_start(__atom)	\
	{ \
		if (get_eflags() & 0x200) { \
			__atom = 0x11223344; \
			__asm__ __volatile__("cli"); \
		} \
		else { \
			__atom = 0; \
		} \
	}
#endif

#ifndef atomic_end
#define atomic_end(__atom)	\
	{ \
		if (__atom == 0x11223344) { \
			__atom = 0; \
			__asm__ __volatile__("sti"); \
		} \
	}
#endif

#endif