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

#include <kheap.h>
#include <drawing/base.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>

////////////////////////////////////////////////////////////////////////////////

static uint32_t text_console_width = 0;
static uint32_t text_console_height = 0;
static uint32_t text_cell_width = 0;
static uint32_t text_cell_height = 0;
static uint16_t *console_buffer = NULL;
static uint16_t *console_mirror = NULL;
static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static uint8_t cursor_blink = 0;

// CGA
static uint32_t color_map[] = {
	0x292E3D, 0x0000AA, 0x00AA00, 0x00AAAA,
	0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
	0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
	0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
};

////////////////////////////////////////////////////////////////////////////////

void vesa_cell_invalidate(uint32_t x, uint32_t y)
{
	invalidate_region(
		x * text_cell_width, 
		y * text_cell_height,
		text_cell_width,
		text_cell_height
	);
}

////////////////////////////////////////////////////////////////////////////////

void vesa_console_redraw(void)
{
	if (console_mirror == NULL)
		return;

	uint32_t cell_count = text_console_width * text_console_height;
	for (uint32_t cell = 0; cell < cell_count; ++cell) {
		if (console_mirror[cell] == console_buffer[cell]) 
			continue;

		uint16_t value = console_buffer[cell];
		uint32_t x = (cell % text_console_width) * text_cell_width;
		uint32_t y = (cell / text_console_width) * text_cell_height;

		draw_char_bmp(
			value & 0xFF, 
			x, 
			y, 
			color_map[(value >> 8) & 0x0F],
			color_map[(value >> 12) & 0x0F]
		);
		console_mirror[cell] = value;
	}
}

void vesa_text_setpos(
	uint32_t x, 
	uint32_t y, 
	uint32_t width __attribute__((unused))
) {
	vesa_cell_invalidate(cursor_x, cursor_y);
	cursor_x = x;
	cursor_y = y;
	cursor_blink = 25;
}

void vesa_text_update_cursor()
{
	if (console_mirror == NULL)
		return;

	uint16_t value = console_buffer[cursor_y * text_console_width + cursor_x];
	draw_char_bmp(
		value & 0xFF, 
		cursor_x * text_cell_width, 
		cursor_y * text_cell_height, 
		color_map[(value >> 8) & 0x0F],
		color_map[(value >> 12) & 0x0F]
	);

	cursor_blink++;
	if ((cursor_blink >= 25) && (cursor_blink <= 50)) {
		fill_rect(
			cursor_x * text_cell_width, 
			(cursor_y * text_cell_height) + (text_cell_height - 2),
			text_cell_width - 1, 
			2,
			color_map[(value >> 8) & 0x0F]
		);
		vesa_cell_invalidate(cursor_x, cursor_y);
	}
	else if (cursor_blink == 51) {
		cursor_blink = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////

void vesa_console_prepare(uint16_t *buffer, uint32_t cols, uint32_t rows)
{
	text_console_width = cols;
	text_console_height = rows;
	text_cell_width = 9;
	text_cell_height = 16;
	console_buffer = buffer;

	uint32_t console_size = text_console_width * text_console_height;
	console_mirror = kalloc(console_size * 2);
	memsetw(console_mirror, 0, console_size);

	clear_screen(color_map[0]);
}