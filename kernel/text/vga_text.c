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
#include <null.h>
#include <vga_text.h>
#include <serial.h>
#include <ascii.h>
#include <memory.h>

#define TAB_WIDTH 4

static struct {
	uint16_t *buffer;
	uint8_t cols;
	uint8_t rows;
	uint8_t attribute;
	uint8_t x;
	uint8_t y; 
} vga_text;

void vga_text_clear(uint8_t attribute)
{
	uint32_t len = vga_text.cols * vga_text.rows;
	uint16_t cell = (attribute << 8) | ' ';

	memsetw(vga_text.buffer, cell, len);

	vga_text.x = 0;
	vga_text.y = 0;
	vga_text.attribute = attribute;
}

void vga_text_scroll()
{
	if (vga_text.y >= vga_text.rows) {
		uint16_t tmp = vga_text.y - vga_text.rows + 1;
		memcpy(
			vga_text.buffer, 
			vga_text.buffer + tmp * vga_text.cols, 
			(vga_text.rows - tmp) * vga_text.cols * sizeof(uint16_t)
		);
		memsetw(
			vga_text.buffer + (vga_text.rows - tmp) * vga_text.cols,
			(vga_text.attribute << 8) | ' ',
			vga_text.cols
		);
		vga_text.y = vga_text.rows - 1;
	}
}

void vga_text_prepare(struct boot_config *config)
{
	kputs_serial("Preparing vga text mode for kernel use...");

	// Make sure the configuration is valid/supplied, otherwise assume defaults.
	if (config == NULL) {
		kputs_serial("No valid boot config supplied. Assuming 80x25 screen.\n");
		vga_text.cols = 80;
		vga_text.rows = 25;
		vga_text.buffer = (uint16_t *)0xb8000;
	}
	else {
		// We have a config, so take the required values.
		vga_text.cols = config->screen_width;
		vga_text.rows = config->screen_height;
		vga_text.buffer = config->linear_frame_buffer;
	}

	// We're ready to clear the screen for future use. Black background with a
	// light grey text color.
	vga_text_clear(0x07);

	kputs_serial("done.\n");
}

void vga_text_control_code(const char c)
{
	switch (c) {
		case ASCII_HT:
			vga_text.x = (vga_text.x + TAB_WIDTH) & ~(TAB_WIDTH - 1);
			break;
		case ASCII_BS:
			if (vga_text.x)
				vga_text.x--;
			break;
		case ASCII_LF:
			++vga_text.y;
		case ASCII_CR:
			vga_text.x = 0;
			break;
		default:
			break;
	}
}

void kputc_vga_text(const char c __attribute__((unused)))
{
	if (c <= ASCII_US || c == ASCII_DEL) {
		// This is a control code and should be treated as such.
		vga_text_control_code(c);
	}
	else {
		// This is a printable character and should be displayed on screen.
		uint32_t offset = (vga_text.y * vga_text.cols) + vga_text.x;
		vga_text.buffer[offset] = (vga_text.attribute << 8) | c;
		++vga_text.x;
	}

	// If we've gone off the edge of the screen then wrap to the following line.
	if (vga_text.x >= vga_text.cols) {
		vga_text.x = 0;
		vga_text.y++;
	}

	// If we've gone off the end of the screen then scroll the contents up so
	// we're back on screen.
	if (vga_text.y >= vga_text.rows) {
		vga_text_scroll();
	}
}

void kputs_vga_text(const char *restrict str)
{
	while (str && *str)
		kputc_vga_text(*str++);
}
