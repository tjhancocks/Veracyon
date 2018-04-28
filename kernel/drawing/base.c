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

#include <drawing/base.h>
#include <memory.h>
#include <atomic.h>
#include <stdio.h>
#include <stddef.h>
#include <uptime.h>

#define BLIT_WIDTH	16
#define BLIT_HEIGHT	16

////////////////////////////////////////////////////////////////////////////////

static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static uint32_t screen_depth = 0;
static uint32_t screen_pitch = 0;
static uint32_t screen_size = 0;
static uint32_t screen_bpp = 0;
static uint32_t *vesa_buffer = NULL;
static uint32_t *buffer = NULL;
static uint8_t blit_mask[BLIT_WIDTH * BLIT_HEIGHT] = { 1 };
static uint32_t blit_rect_width;
static uint32_t blit_rect_height;
static uint32_t blit_count;
static suseconds_t next_blit_time_ms = 0;

extern uint8_t bios_font[0x1000];

////////////////////////////////////////////////////////////////////////////////

void drawing_prepare(struct boot_config *config)
{
	if (!config || config->vesa_mode == vga_mode_text)
		return;

	screen_width = config->screen_width;
	screen_height = config->screen_height;
	screen_depth = config->screen_depth;
	screen_pitch = config->bytes_per_line;
	screen_size = config->screen_size;
	screen_bpp = config->bytes_per_pixel;

	fprintf(COM1, "Screen is %dx%d %d-bpp\n", 
		screen_width, screen_height, screen_bpp);

	vesa_buffer = config->front_buffer;
	buffer = config->back_buffer;

	blit_rect_width = screen_width / BLIT_WIDTH;
	blit_rect_height = screen_height / BLIT_HEIGHT;
	blit_count = BLIT_WIDTH * BLIT_HEIGHT;
	next_blit_time_ms = 1;
}

////////////////////////////////////////////////////////////////////////////////

static inline void _mark_blit(uint32_t x, uint32_t y)
{
	uint32_t bx = (uint32_t)(x / blit_rect_width);
	uint32_t by = (uint32_t)(y / blit_rect_height);
	blit_mask[by * BLIT_WIDTH + bx] = 1;
}

static inline void _blit_rect(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2)
{
	atom_t atom;
	atomic_start(atom);

	uint32_t *source = buffer + (y*(screen_pitch/screen_bpp)) + x;
	uint32_t *dest = vesa_buffer + (y*(screen_pitch/screen_bpp)) + x;
	uint32_t length = (x2 - x) * screen_bpp;

	for (uint32_t yy = y; yy < y2; ++yy) {
		mmx_memcpy(dest, source, length);
		dest += (screen_pitch / screen_bpp);
		source += (screen_pitch / screen_bpp);
	}

	atomic_end(atom);
}

static inline void _blit(void)
{
	suseconds_t time = get_uptime_ms();
	if (time < next_blit_time_ms || next_blit_time_ms == 0)
		return;
	next_blit_time_ms = time + (1000/60);

	for (uint32_t blit_region = 0; blit_region < blit_count; ++blit_region) {
		if (blit_mask[blit_region] == 0) continue;
		uint32_t bx = ((uint32_t)(blit_region % BLIT_WIDTH)) * blit_rect_width;
		uint32_t by = ((uint32_t)(blit_region / BLIT_WIDTH)) * blit_rect_height;
		_blit_rect(bx, by, bx + blit_rect_width, by + blit_rect_height);
		blit_mask[blit_region] = 0;
	}
}

static inline void _fill_rect(
	uint32_t x, 
	uint32_t y, 
	uint32_t w, 
	uint32_t h, 
	uint32_t clr
) {
	for (uint32_t n = 0; n < h; ++n) {
		// Work out the starting address.
		uint32_t offset = ((y + n) * screen_pitch + (x * screen_bpp));
		uint32_t start = ((uint32_t)buffer) + offset;
		memsetd((void *)start, clr, w);
	}
}

////////////////////////////////////////////////////////////////////////////////

void invalidate_region(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	for (uint32_t yy = 0; yy < height; ++yy)
		for (uint32_t xx = 0; xx < width; ++xx)
			_mark_blit(x + xx, y + yy);
}

void draw_char_bmp(uint8_t c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg)
{
	uint32_t rx = x;
	uint32_t ry = y;
	uint8_t mask[] = { 1, 2, 4, 8, 16, 32, 64, 128};
	uint8_t *glyph = bios_font + (int)c * 16;

	uint32_t *ptr = buffer + (ry * (screen_pitch / screen_bpp)) + (rx + 8);
	for (uint8_t cy = 0; cy < 16; ++cy) {
		*(ptr--) = bg;
		_mark_blit(rx+8, ry+cy);
		for (uint8_t cx = 0; cx < 8; ++cx) {
			*(ptr--) = (glyph[cy] & mask[cx]) ? fg : bg;
			_mark_blit(rx+7-cx, ry+cy);
		}
		ptr += (screen_pitch / screen_bpp) + 9;
	}
}


////////////////////////////////////////////////////////////////////////////////

void blit(void)
{
	_blit();
}

void clear_screen(uint32_t color)
{
	memset(blit_mask, 1, sizeof(blit_mask));
	_fill_rect(0, 0, screen_width, screen_height, color);
}

void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t clr) 
{
	_fill_rect(x, y, w, h, clr);
}
