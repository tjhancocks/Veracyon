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

#ifndef __VKERNEL_PHYSICAL_MEMORY_MANAGER__
#define __VKERNEL_PHYSICAL_MEMORY_MANAGER__

#include <kern_types.h>
#include <boot_config.h>

/**
 Retrieve the ending address in memory of the kernel.

 RETURNS:
	The ending address of the kernel.
 */
uintptr_t kernel_end_address(void);

/**
 Reserve the specified number of bytes within the Kernel's hard wired working
 memory. The starting address of the reserved address will be returned.
	
	- length: The number of bytes of working memory to be reserved.

 RETURNS:
 	The starting address of the working memory reserved.
 */
uintptr_t reserve_kernel_working_memory(uint32_t length);

/**
 Setup and prepare the physical memory manager. This is an involved function and
 requires a valid boot configuration to be provided. It will search for what
 physical frames are actually available to the kernel, how many are in use and
 thus how many are still available. It will then proceed to construct a stack
 of free/available frame addresses then can be used later.

 	- config: A valid boot configuration structure provided by the boot loader.

 WARNING:
 	Under no circumstances should this function be called more than once. This
 	will trigger a kernel panic in future versions.
 */
void physical_memory_prepare(struct boot_config *config);

/**
 Allocates the next available physical frame.

 RETURNS:
 	The address of the allocated frame.
 */
uintptr_t kframe_alloc(void);

/**
 Free the specified frame and return it to the free/available frame stack, for
 re-use.
 */
void kframe_free(uintptr_t frame);

#endif