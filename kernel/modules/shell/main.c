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
#include <modules/shell.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <kheap.h>
#include <process.h>
#include <macro.h>
#include <atomic.h>

////////////////////////////////////////////////////////////////////////////////

static uint8_t shell_terminated = 0;

////////////////////////////////////////////////////////////////////////////////

static void kernel_about(void)
{
	printf("Veracyon OS %s\n", __BUILD_VERSION__);
	printf("%8s: %s\n", "Kernel", "vkernel");
	printf("%8s: %s\n", "Built", __BUILD_TIME__);
	printf("%8s: %s\n", "Commit", __BUILD_COMMIT__);
	printf("\n");
}

static void kernel_uptime(void)
{
	sysinfo_t info;
	get_sysinfo(&info);
	uint64_t time_ms = info.uptime_ms;
	uint64_t time_s = info.uptime_s;
	uint64_t time_m = time_s / 60;
	uint64_t time_h = time_m / 60;
	uint64_t time_d = time_h / 24;

	time_ms %= 1000;
	time_s %= 60;
	time_h %= 60;
	time_d %= 24;

	if (time_d) {
		printf("\t%02dd, %02dm, %02dm, %02ds, %03dms\n", 
			time_d, time_h, time_m, time_s, time_ms);
	}
	else if (time_h) {
		printf("\t%02dm, %02dm, %02ds, %03dms\n", 
			time_h, time_m, time_s, time_ms);
	}
	else if (time_m) {
		printf("\t%02dm, %02ds, %03dms\n", time_m, time_s, time_ms);
	}
	else if (time_s) {
		printf("\t%02ds, %03dms\n", time_s, time_ms);
	}
	else {
		printf("\t%03dms\n", time_ms);
	}
}


////////////////////////////////////////////////////////////////////////////////

void shell_prompt(void)
{
	fprintf(VT100, "> ");

	const char *input = NULL;//read_user_input();

	if (strcmp(input, "exit") == 0) {
		shell_terminated = 1;
	}
	else if (strcmp(input, "about") == 0) {
		kernel_about();
	}
	else if (strcmp(input, "uptime") == 0) {
		kernel_uptime();
	}

	kfree((void *)input);
}


int shell_main(void)
{
	while (shell_terminated == 0) {
		shell_prompt();
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////

void init_shell(void)
{
	// Ensure that the shell termination flag is not set.
	shell_terminated = 0;
	process_launch("root-shell", shell_main, P_UI);
}