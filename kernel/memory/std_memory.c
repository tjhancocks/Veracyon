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

#include <memory.h>

void *memcpy(void *restrict dst, const void *restrict src, uint32_t n)
{
	register uint8_t *d0 = (uint8_t *)dst;
	register uint8_t *s0 = (uint8_t *)src;

	while (n--)
		*d0++ = *s0++;

	return dst;
}

void *memset(void *restrict dst, uint8_t value, uint32_t n)
{
	register uint8_t *d0 = (uint8_t *)dst;

	while (n--)
		*d0++ = value;

	return dst;
}

void *memsetw(void *restrict dst, uint16_t value, uint32_t n)
{
	register uint16_t *d0 = (uint16_t *)dst;

	while (n--)
		*d0++ = value;

	return dst;
}