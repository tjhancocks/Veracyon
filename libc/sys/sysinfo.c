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

#include <sys/info.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#if __libk__
#include <arch/arch.h>
#include <panic.h>
#endif

void get_sysinfo(sysinfo_t *info)
{
	if (!info) {
#if __libk__
		struct panic_info info = (struct panic_info) {
			panic_general,
			"NULL argument on get_sysinfo()",
			"get_sysinfo() must be provided a valid sysinfo_t structure."
		};
		panic(&info, NULL);
#endif
		return;
	}

	// Get the time information
#if __libk__
	uint64_t ticks = 0;
	uint64_t subticks = 0;
	uint64_t phase = 0;
	
	arch_get_ticks(&ticks, &subticks, &phase);

	// Get the microseconds, assuming a phase of 1000Hz
	info->uptime_s = (subticks >= 500) ? ticks + 1 : ticks;
	info->uptime_u = ((ticks * phase) + subticks) * 1000;
#endif
}