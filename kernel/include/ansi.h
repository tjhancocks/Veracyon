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

#ifndef __VKERNEL_ANSI__
#define __VKERNEL_ANSI__

#include <term.h>

/**
 Parse and render the specified string within the context of the provided 
 terminal interface. The rendition will be constructed using a subset of the
 standard ANSI Escape Codes.

 The currently provided escape codes are:

 	- ESC[i;jm	- Set graphic rendition, Colours: 30-37, 40-47, 90-97, 100-107
 	- ESC[iJ	- Erase in Display
 	- ESC[i,jH	- Set cursor position

 	- term: The terminal interface to use. This provides all functions for
 		driving terminal output.
 	- str: The ANSI string to render and display in the terminal.
 */
void ansi_parse_and_display_string(
	struct term_interface *term, 
	const char *restrict str
);

#endif