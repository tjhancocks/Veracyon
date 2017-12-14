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

#include <modules/shell.h>
#include <kern_types.h>
#include <read.h>
#include <kprint.h>
#include <string.h>
#include <kheap.h>

////////////////////////////////////////////////////////////////////////////////

static uint8_t shell_terminated = 0;

////////////////////////////////////////////////////////////////////////////////

static void kernel_about(void)
{
	kprint("Veracyon OS %s\n", __BUILD_VERSION__);
	kprint("%8s: %s\n", "Kernel", "vkernel");
	kprint("%8s: %s\n", "Built", __BUILD_TIME__);
	kprint("%8s: %s\n", "Commit", __BUILD_COMMIT__);
	kprint("\n");
}


////////////////////////////////////////////////////////////////////////////////

void shell_prompt(void)
{
	kdprint(krnout, "> ");

	const char *input = read_user_input();

	if (strcmp(input, "exit") == 0) {
		shell_terminated = 1;
	}
	else if (strcmp(input, "about") == 0) {
		kernel_about();
	}

	kfree((void *)input);
}


void shell_main(void)
{
	while (shell_terminated == 0) {
		shell_prompt();
	}
}


////////////////////////////////////////////////////////////////////////////////

void init_shell(void)
{
	// Ensure that the shell termination flag is not set.
	shell_terminated = 0;

	// Launch it!
	shell_main();
}