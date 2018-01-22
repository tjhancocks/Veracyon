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
#include <kprint.h>

////////////////////////////////////////////////////////////////////////////////

static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static uint32_t screen_depth = 0;
static uint32_t screen_pitch = 0;
static uint32_t screen_size = 0;
static uint32_t screen_bpp = 0;
static uint32_t *vesa_buffer = NULL;
static uint32_t *buffer = NULL;

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

	kdprint(COM1, "Screen is %dx%d %d-bpp\n", 
		screen_width, screen_height, screen_bpp);

	vesa_buffer = config->front_buffer;
	buffer = config->back_buffer;
}

////////////////////////////////////////////////////////////////////////////////

static inline void _flush(void)
{
	atom_t atom;
	atomic_start(atom);
	mmx_memcpy(vesa_buffer, buffer, screen_size);
	atomic_end(atom);
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
		uint32_t offset = ((y + n) * screen_pitch + (x * (screen_bpp / 8)));
		uint32_t start = ((uint32_t)buffer) + offset;
		memsetd((void *)start, clr, screen_width);
	}
	_flush();
}

////////////////////////////////////////////////////////////////////////////////

void clear_screen(uint32_t color)
{
	_fill_rect(0, 0, screen_width, screen_height, color);
}
