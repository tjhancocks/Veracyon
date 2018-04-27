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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t __kregister_read(const char *restrict name)
{
	uint32_t value = 0xDEADBEEF;

	if (strcmp(name, "eax") == 0) 
		__asm__ __volatile("movl %%eax, %0" : "=r"(value));
	else if (strcmp(name, "ebx") == 0) 
		__asm__ __volatile("movl %%ebx, %0" : "=r"(value));
	else if (strcmp(name, "ecx") == 0) 
		__asm__ __volatile("movl %%ecx, %0" : "=r"(value));
	else if (strcmp(name, "edx") == 0) 
		__asm__ __volatile("movl %%edx, %0" : "=r"(value));
	else if (strcmp(name, "esi") == 0) 
		__asm__ __volatile("movl %%esi, %0" : "=r"(value));
	else if (strcmp(name, "edi") == 0) 
		__asm__ __volatile("movl %%edi, %0" : "=r"(value));
	else if (strcmp(name, "esp") == 0) 
		__asm__ __volatile("movl %%esp, %0" : "=r"(value));
	else if (strcmp(name, "ebp") == 0) 
		__asm__ __volatile("movl %%ebp, %0" : "=r"(value));
	else if (strcmp(name, "cr0") == 0) 
		__asm__ __volatile("movl %%cr0, %0" : "=r"(value));
	else if (strcmp(name, "cr4") == 0) 
		__asm__ __volatile("movl %%cr4, %0" : "=r"(value));
	else if (strcmp(name, "cr2") == 0) 
		__asm__ __volatile("movl %%cr2, %0" : "=r"(value));
	else if (strcmp(name, "cr3") == 0) 
		__asm__ __volatile("movl %%cr3, %0" : "=r"(value));
	else {
		fprintf(COM1, "Attempted to read unrecognised register: %s\n", name);
	}

	return value;
}