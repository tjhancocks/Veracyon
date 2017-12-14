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

#include <kern_types.h>
#include <drawing/base.h>
#include <memory.h>
#include <kprint.h>
#include <biosfont.h>
#include <sema.h>

uint32_t drawing_vga_rgb_colors[16] = {
	0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
	0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
	0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
	0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
};

struct {
	void *base_buffer;
	void *backing_buffer;
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t fg_pen_color;
	uint32_t bg_pen_color;
	uint32_t cursor_x;
	uint32_t cursor_y;
} drawing_info;

static spin_lock_t draw_lock = { 0 };

////////////////////////////////////////////////////////////////////////////////

void drawing_base_prepare(
	void *front_buffer,
	void *backing_buffer,
	uint32_t size,
	uint32_t width, 
	uint32_t height, 
	uint32_t depth
) {
	spin_init(draw_lock);

	drawing_info.base_buffer = front_buffer;
	drawing_info.backing_buffer = backing_buffer;
	drawing_info.size = size;
	drawing_info.width = width;
	drawing_info.height = height;
	drawing_info.depth = depth;
	drawing_info.bg_pen_color = 0x0;
	drawing_info.fg_pen_color = 0x7;
	kdprint(dbgout, "Base Drawing Library prepared.\n");
	kdprint(dbgout, "  Main Buffer: %p\n", drawing_info.base_buffer);
	kdprint(dbgout, "  Backing Buffer: %p\n", drawing_info.backing_buffer);
}


////////////////////////////////////////////////////////////////////////////////

void drawing_set_pen_vga(uint8_t attribute)
{
	spin_lock(draw_lock);
	drawing_info.fg_pen_color = drawing_vga_rgb_colors[attribute & 0xF];
	drawing_info.bg_pen_color = drawing_vga_rgb_colors[(attribute >> 4) & 0xF];
	spin_unlock(draw_lock);
}

void drawing_set_cursor(uint32_t x, uint32_t y)
{
	spin_lock(draw_lock);
	drawing_info.cursor_x = x;
	drawing_info.cursor_y = y - 2;
	spin_unlock(draw_lock);
}


////////////////////////////////////////////////////////////////////////////////

void drawing_base_clear(void)
{
	uint32_t length = (drawing_info.width * drawing_info.height);
	spin_lock(draw_lock);
	memsetd(drawing_info.backing_buffer, drawing_info.bg_pen_color, length);
	spin_unlock(draw_lock);
}

void drawing_base_render_char(const char c, uint32_t x, uint32_t y)
{
	uint32_t *buffer = (uint32_t *)drawing_info.backing_buffer;

	// Render the character to the screen buffer.
	spin_lock(draw_lock);
	for (uint8_t cy = 0; cy < 16; ++cy) {
		
		uint8_t row = bios_font[(c * 16) + cy];
		
		for (uint8_t cx = 0; cx < 8; ++cx) {
			uint32_t offset = (drawing_info.width * (cy + y)) + (x + cx);
			buffer[offset] = ((row << cx) & 0x80) ? drawing_info.fg_pen_color
												  : drawing_info.bg_pen_color;
		}
	}
	spin_unlock(draw_lock);
}


////////////////////////////////////////////////////////////////////////////////

void drawing_base_flush(void)
{
	spin_lock(draw_lock);

	register uint32_t n = drawing_info.size / 128;
	register uint64_t *d0 = (uint64_t *)drawing_info.base_buffer;
	register uint64_t *s0 = (uint64_t *)drawing_info.backing_buffer;

	while (n--) {
		// DQWord, (XMM0)
		*d0++ = *s0++;
		*d0++ = *s0++;

		// DQWord, (XMM1)
		*d0++ = *s0++;
		*d0++ = *s0++;

		// DQWord, (XMM2)
		*d0++ = *s0++;
		*d0++ = *s0++;

		// DQWord, (XMM3)
		*d0++ = *s0++;
		*d0++ = *s0++;

		// DQWord, (XMM4)
		*d0++ = *s0++;
		*d0++ = *s0++;

		// DQWord, (XMM5)
		*d0++ = *s0++;
		*d0++ = *s0++;

		// DQWord, (XMM6)
		*d0++ = *s0++;
		*d0++ = *s0++;

		// DQWord, (XMM7)
		*d0++ = *s0++;
		*d0++ = *s0++;
	}

	// Draw the virtual hardware cursor at the specified location
	uint32_t *buffer = (uint32_t *)drawing_info.base_buffer;
	uint32_t x = drawing_info.cursor_x;
	uint32_t y = drawing_info.cursor_y;

	// Render the character to the screen buffer.
	for (uint8_t cy = 14; cy < 16; ++cy) {
		for (uint8_t cx = 0; cx < 8; ++cx) {
			uint32_t offset = (drawing_info.width * (cy + y)) + (x + cx);
			buffer[offset] = drawing_info.fg_pen_color;
		}
	}

	spin_unlock(draw_lock);
}

