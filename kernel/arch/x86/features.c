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

#include <arch/x86/features.h>
#include <stdint.h>

#define CPUID_GETFEATURES	1

////////////////////////////////////////////////////////////////////////////////

enum cpuid_features
{
	CPUID_FEATURE_EDX_MMX	= 1 << 23,
	CPUID_FEATURE_EDX_SSE 	= 1 << 25,
	CPUID_FEATURE_EDX_SSE2	= 1 << 26,
};

////////////////////////////////////////////////////////////////////////////////

static inline void cpuid(
	int code, 
	uint32_t *a, 
	uint32_t *b, 
	uint32_t *c, 
	uint32_t *d
) {
	__asm__ __volatile__(
		"cpuid"
		: "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
		: "a"(code)
	);
}

////////////////////////////////////////////////////////////////////////////////

int cpu_sse_available(void)
{
	return 0;
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuid(CPUID_GETFEATURES, &eax, &ebx, &ecx, &edx);
	return ((edx & CPUID_FEATURE_EDX_SSE) == 1);
}

int cpu_mmx_available(void)
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuid(CPUID_GETFEATURES, &eax, &ebx, &ecx, &edx);
	return ((edx & CPUID_FEATURE_EDX_MMX) == 1);
}