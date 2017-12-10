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

#ifndef __VKERNEL_X86_INTERRUPT__
#define __VKERNEL_X86_INTERRUPT__

#include <boot_config.h>
#include <kern_types.h>
#include <arch/x86/cpu_state.h>

typedef void(*interrupt_handler_t)(struct interrupted_cpu_state *);

/**
 Prepare the kernel for using and adding interrupt handlers. This will simply
 check the boot configuration to determine where the interrupt handler look up
 table is located.

 	- config: A valid boot configuration structure.
 */
void interrupt_handlers_prepare(struct boot_config *config);

/**
 Specify a function to be an interrupt handler for the specified interrupt.
 Interrupts are in the range of 0x00 - 0xFF, and the handler function should
 have the prototype:

 	void function(struct interrupted_cpu_state *);

 To remove an existing interrupt handler, NULL should be passed in place of a
 function pointer.

 	- interrupt: The interrupt number of which the handler will be for.
 	- handler: A function pointer to the function that will handle the interrupt
 */
void interrupt_handler_add(uint8_t interrupt, interrupt_handler_t handler);

#endif