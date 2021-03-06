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

#ifndef __VKERNEL_KHEAP__
#define __VKERNEL_KHEAP__

#include <stdint.h>

#define kHEAP_ALLOC_MAGIC	0xA110CA7E
#define kHEAP_AVAIL_MAGIC	0xF7EEF7EE

struct kheap_block  {
	uint32_t magic;
	struct kheap_block *prev;
	struct kheap_block *next;
	size_t size;
} __attribute__((packed));

/**
 Allocate a block of memory that is of the specified length.

 	- length: The number of bytes to be allocated.

 Returns:
 	Pointer to the start of the allocated memory
 */
void *kalloc(size_t length);

/**
 Free the specified allocated memory.

 	- ptr: A pointer to the start of allocated memory that is to be free'd.
 */
void kfree(void *ptr);

/**
 Describe the structure of the kernel heap.
 */
void kheap_dump_structure(void);

#endif