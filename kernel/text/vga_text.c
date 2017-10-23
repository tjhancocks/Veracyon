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
#include <device/io/file.h>
#include <kprint.h>
#include <panic.h>
#include <arch/x86/port.h>
#include <term.h>

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

	vga_text_setpos(0, 0);
	vga_text.attribute = attribute;

	kdprint(dbgout, "\n-- CLEARED VGA TEXT SCREEN --\n\n");
}

void vga_text_setpos(uint32_t x, uint32_t y)
{

	vga_text.x = (uint8_t)(x & 0xFF);
	vga_text.y = (uint8_t)(y & 0xFF);
	kdprint(dbgout, "setting x: %d, y: %d\n", vga_text.x, vga_text.y);
	vga_update_cursor();
}

void vga_text_getpos(uint32_t *x, uint32_t *y)
{
	if (x) *x = vga_text.x;
	if (y) *y = vga_text.y;
}

void vga_update_cursor()
{
	uint16_t pos = (vga_text.cols * vga_text.y) + vga_text.x;
	outb(0x3d4, 0x0f);
	outb(0x3d5, (uint8_t)(pos & 0xFF));
	outb(0x3d4, 0x0e);
	outb(0x3d5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_text_setattr(uint8_t attribute)
{
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
	kdprint(dbgout, "Preparing vga text mode for kernel use...\n");

	// Make sure the configuration is valid/supplied, otherwise assume defaults.
	if (config == NULL) {
		kdprint(
			dbgout, 
			"No valid boot config supplied. Assuming 80x25 screen.\n"
		);
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

	devio_bind_putc(__kKRNOUT, kputc_vga_text);
	devio_bind_puts(__kKRNOUT, kputs_vga_text);

	term_bind_get_cursor(vga_text_getpos);
	term_bind_set_cursor(vga_text_setpos);

	kdprint(dbgout, "VGA text screen resolution: %dx%d\n", 
		vga_text.cols, vga_text.rows);
	kdprint(dbgout, "VGA text buffer located at: %p\n", vga_text.buffer);

	prepare_text_panic(config);

	// Disable text blinking in VGA Text Mode, so that bright backgrounds can
	// be used.
	(void)inb(0x3da);
	outb(0x3c0, 0x30);
	outb(0x3c0, inb(0x3c1) & 0xF7);

	// We're ready to clear the screen for future use. Black background with a
	// light grey text color.
	vga_text_clear(0x07);
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
	vga_update_cursor();
}
