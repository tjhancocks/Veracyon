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
#include <vesa_text.h>
#include <serial.h>
#include <ascii.h>
#include <memory.h>
#include <kprint.h>
#include <arch/arch.h>
#include <term.h>
#include <drawing/base.h>
#include <biosfont.h>
#include <sema.h>

#define TAB_WIDTH 	4
#define CHAR_WIDTH 	(BIOS_FONT_WIDTH + 1)
#define CHAR_HEIGHT (BIOS_FONT_HEIGHT)

static struct {
	uint16_t width;
	uint16_t height;
	void *buffer;
	void *backing_buffer;
	uint8_t cols;
	uint8_t rows;
	uint8_t attribute;
	uint8_t default_attribute;
	uint8_t x;
	uint8_t y; 
} vesa_text;

static spin_lock_t vesa_lock = { 0 };

////////////////////////////////////////////////////////////////////////////////

void kputc_vesa_text(const char c);
void kputs_vesa_text(const char *restrict str);
void vesa_text_setpos(uint32_t x, uint32_t y);
void vesa_text_setattr(uint8_t attribute);
void vesa_text_clear(uint8_t attribute);
void vesa_update_cursor(void);

////////////////////////////////////////////////////////////////////////////////

void vesa_text_clear(uint8_t attribute)
{
	drawing_set_pen_vga(attribute);

	spin_lock(vesa_lock);
	drawing_base_clear();
	spin_unlock(vesa_lock);

	vesa_text_setpos(0, 0);
	vesa_text.attribute = attribute;
	vesa_text.default_attribute = attribute;

	kdprint(dbgout, "\n-- CLEARED VESA TEXT SCREEN --\n\n");
}

void vesa_text_setpos(uint32_t x, uint32_t y)
{
	vesa_text.x = (uint8_t)(x & 0xFF);
	vesa_text.y = (uint8_t)(y & 0xFF);
	vesa_update_cursor();
}

void vesa_text_getpos(uint32_t *x, uint32_t *y)
{
	if (x) *x = vesa_text.x;
	if (y) *y = vesa_text.y;
}

void vesa_update_cursor(void)
{
	spin_lock(vesa_lock);
	drawing_set_cursor(vesa_text.x * CHAR_WIDTH, vesa_text.y * CHAR_HEIGHT);
	drawing_base_flush();
	spin_unlock(vesa_lock);
}

void vesa_text_setattr(uint8_t attribute)
{
	vesa_text.attribute = attribute;
	drawing_set_pen_vga(attribute);
}

void vesa_text_getattr(uint8_t *attribute)
{
	if (attribute) *attribute = vesa_text.attribute;
}

void vesa_text_set_default_attribute(uint8_t attribute)
{
	vesa_text.default_attribute = attribute;
}

void vesa_text_restore_default_attribute(void)
{
	vesa_text.attribute = vesa_text.default_attribute;
	drawing_set_pen_vga(vesa_text.attribute);
}

void vesa_text_scroll(void)
{
	// TODO: Implement VESA text scrolling. Manipulate the screen buffer to do 
	// so.
}

void vesa_text_prepare(struct boot_config *config)
{
	spin_init(vesa_lock);

	kdprint(dbgout, "Preparing VESA Text Mode for kernel use...\n");

	// Make sure the configuration is valid/supplied, otherwise assume defaults.
	uint32_t buffer_size = 0;
	if (config == NULL) {
		kdprint(
			dbgout, 
			"No valid boot config supplied. Assuming 800x600 (100x37) screen.\n"
		);
		vesa_text.width = 800;
		vesa_text.height = 600;
		vesa_text.buffer = NULL;
		vesa_text.backing_buffer = NULL;
	}
	else {
		// We have a config, so take the required values.
		vesa_text.width = config->screen_width;
		vesa_text.height = config->screen_height;
		vesa_text.buffer = config->front_buffer;
		vesa_text.backing_buffer = config->back_buffer;
		buffer_size = config->screen_size;
	}

	drawing_base_prepare(
		vesa_text.buffer,
		vesa_text.backing_buffer,
		buffer_size,
		vesa_text.width,
		vesa_text.height,
		32
	);

	vesa_text.cols = vesa_text.width / CHAR_WIDTH;
	vesa_text.rows = vesa_text.height / CHAR_HEIGHT;

	// Make sure the KRNOUT mode of the terminal is going to be parsing ANSI
	// escape codes.
	term_use_ansi(krnout, 1);

	// Bind all interface functions so that the terminal can drive the VGA Text
	// display.
	term_bind_putc(krnout, kputc_vesa_text);
	term_bind_puts(krnout, kputs_vesa_text);
	term_bind_get_cursor(krnout, vesa_text_getpos);
	term_bind_set_cursor(krnout, vesa_text_setpos);
	term_bind_update_cursor(krnout, vesa_update_cursor);
	term_bind_set_attribute(krnout, vesa_text_setattr);
	term_bind_get_attribute(krnout, vesa_text_getattr);
	term_bind_set_default_attribute(krnout, vesa_text_set_default_attribute);
	term_bind_restore_default_attribute(
		krnout, vesa_text_restore_default_attribute
	);
	term_bind_clear(krnout, vesa_text_clear);

	kdprint(dbgout, "VESA text screen resolution: %dx%d", 
		vesa_text.width, vesa_text.height);
	kdprint(dbgout, " (%dx%d)\n", 
		vesa_text.cols, vesa_text.rows);
	kdprint(dbgout, "VESA text buffer located at: %p\n", vesa_text.buffer);

	// We're ready to clear the screen for future use. Black background with a
	// light grey text color.
	vesa_text_clear(0x07);
}

void vesa_text_control_code(const char c)
{
	switch (c) {
		case kASCII_HT:
			vesa_text.x = (vesa_text.x + TAB_WIDTH) & ~(TAB_WIDTH - 1);
			break;
		case kASCII_BS:
			if (vesa_text.x)
				vesa_text.x--;
			break;
		case kASCII_LF:
			++vesa_text.y;
		case kASCII_CR:
			vesa_text.x = 0;
			break;
		default:
			break;
	}
}

void kputc_vesa_text(const char c)
{
	spin_lock(vesa_lock);
	if (c <= kASCII_US || c == kASCII_DEL) {
		// This is a control code and should be treated as such.
		vesa_text_control_code(c);
	}
	else {
		// This is a printable character and should be displayed on screen.
		drawing_base_render_char(
			c, 
			vesa_text.x * CHAR_WIDTH, 
			vesa_text.y * CHAR_HEIGHT
		);
		++vesa_text.x;
	}

	// If we've gone off the edge of the screen then wrap to the following line.
	if (vesa_text.x >= vesa_text.cols) {
		vesa_text.x = 0;
		vesa_text.y++;
	}
	spin_unlock(vesa_lock);

	// If we've gone off the end of the screen then scroll the contents up so
	// we're back on screen.
	if (vesa_text.y >= vesa_text.rows) {
		vesa_text_scroll();
	}
}

void kputs_vesa_text(const char *restrict str)
{
	while (str && *str)
		kputc_vesa_text(*str++);
	vesa_update_cursor();
}
