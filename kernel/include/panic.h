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

#ifndef __VKERNEL_PANIC__
#define __VKERNEL_PANIC__

struct boot_config;
struct interrupt_frame;

enum panic_type
{
	panic_general,
	panic_memory,
	panic_error,
};

struct panic_info
{
	enum panic_type type;
	const char *title;
	const char *message;
};

/**
 Prepares the Kernel's panic handling functionality, and overrides the panic
 handler put in place by CoreLoader.

 	- config: A valid boot configuration structure.
 */
void prepare_panic_handler(struct boot_config *config);

/**
 Invoke a panic in the kernel. This is typically used as a way to escape from
 a potentially serious or fatal situation the the kernel has gotten into.
	
	- info: Contains information about circumstances of the panic. Can be NULL.
	- frame: Contains register values at the time of the panic. Can be NULL.

 WARNING:
	This is not a means of warning the user. It is a nuclear option and will
	result in the system coming to a halt permanatly.
 */
void panic(struct panic_info *info, struct interrupt_frame *frame);

#endif