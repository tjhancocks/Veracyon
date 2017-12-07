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

#ifndef __VKERNEL_DRAWING_BASE__
#define __VKERNEL_DRAWING_BASE__

#include <kern_types.h>

/**
 Configure the basic backing buffer for the drawing library. This needs to keep
 track of the screen buffer, resolution, etc.

 	- buffer: The base address of the screen buffer.
 	- size: The number of bytes that comprise the screen buffer.
 	- width: The width of the screen in pixels.
	- height: The height of the screen in pixels.
	- depth: The number of bits in a pixel.
 */
void drawing_base_prepare(
	void *front_buffer,
	void *backing_buffer,
	uint32_t size,
	uint32_t width, 
	uint32_t height, 
	uint32_t depth
);

/**
 Set the pen color using a VGA attribute.

 	- attribute: VGA Attribute Code (0x0 - 0xF)
 */
void drawing_set_pen_vga(uint8_t attribute);

/**
 Clear the base drawing buffer with the current pen color.
 */
void drawing_base_clear(void);

/**
 Render the specified character at the specified location.

 	- c: The character to draw
 	- x: The x location to draw the character
 	- y: The y location to draw the character
 */
void drawing_base_render_char(const char c, uint32_t x, uint32_t y);

#endif