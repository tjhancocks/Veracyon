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

#ifndef __VKERNEL_STD_MEMORY__
#define __VKERNEL_STD_MEMORY__

#include <stdint.h>

/**
 Copy the specified number of bytes of memory, byte by byte from the source to
 the destination.

 	- dst: The destination of the copy operation.
 	- src: The source of the copy operation.
 	- n: The number of bytes to copy.

 RETURNS:
 	A pointer to the copy destination.
 */
void *mmx_memcpy(void *restrict dst, const void *restrict src, size_t n);

/**
 Copy the specified number of dwords of memory, dword by dwords from the 
 source to the destination.

 	- dst: The destination of the copy operation.
 	- src: The source of the copy operation.
 	- n: The number of double words to copy.

 RETURNS:
 	A pointer to the copy destination.
 */
void *memcpyd(void *restrict dst, const void *restrict src, size_t n);

/**
 Copy the specified number of qwords of memory, qword by qword from the 
 source to the destination.

 	- dst: The destination of the copy operation.
 	- src: The source of the copy operation.
 	- n: The number of quad words to copy.

 RETURNS:
 	A pointer to the copy destination.
 */
void *memcpyq(void *restrict dst, const void *restrict src, size_t n);

/**
 Perform an SSE memory copy if SSE functionality is available.

 	- dst: The destination of the copy operation.
 	- src: The source of the copy operation.
 	- n: The number of double quad words to copy.
 */
extern int sse_memcpy(void *dst, void *src, size_t count);

/**
 Write the value into each word starting at the specified destination, repeating
 for the required number of words.

 	- dst: The destination of the set operation.
 	- value: The word value to write into memory.
 	- n: The number of words to iterate over.

 RETURNS:
 	A pointer to the copy destination.
 */
void *memsetw(void *restrict dst, uint16_t value, size_t n);

/**
 Write the value into each double word starting at the specified destination, 
 repeating for the required number of double words.

 	- dst: The destination of the set operation.
 	- value: The double word value to write into memory.
 	- n: The number of double words to iterate over.

 RETURNS:
 	A pointer to the copy destination.
 */
void *memsetd(void *restrict dst, uint32_t value, size_t n);

#endif