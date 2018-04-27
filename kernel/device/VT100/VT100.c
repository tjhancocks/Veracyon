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

#include <device/VT100/VT100.h>
#include <driver/vga/text.h>
#include <driver/vesa/console.h>
#include <string.h>
#include <stddef.h>
#include <ascii.h>
#include <atomic.h>
#include <memory.h>
#include <panic.h>
#include <kheap.h>

#define TAB			4
#define MAX_ARGS	32
#define BUFFER_LEN	64
#define MAX_STACK	64

////////////////////////////////////////////////////////////////////////////////

struct VT100_info
{
	struct {
		uint32_t cols;
		uint32_t rows;
		uint32_t width;
		uint32_t height;
	} screen;
	struct {
		uint32_t x;
		uint32_t y;
		uint8_t attr;
		uint32_t x_stack[MAX_STACK];
		uint32_t y_stack[MAX_STACK];
		uint8_t attr_stack[MAX_STACK];
		uint32_t idx;
		void(*update)(uint32_t, uint32_t, uint32_t);
	} cursor;
	struct {
		uint16_t *ptr;
		int batching;
		void(*redraw)(void);
	} buffer;
	struct {
		int parsing;
		int bracket_type;
		int idx;
		int dirty_arg;
		uint32_t args[MAX_ARGS];
		uint8_t arg_count;
		char buffer[64];
		char *ptr;
	} escape;
};

static struct device __vt100 = { 0 };
static struct VT100_info __vt100_info = { 0 };

static uint8_t vt100_color_map[] = {
	0x88, 0xCC, 0xAA, 0xEE, 0x99, 0xDD, 0xBB, 0xFF
};

////////////////////////////////////////////////////////////////////////////////

static int vt100_is_esc(const char c);
static void vt100_parse_control_char(struct VT100_info *vt100, const char c);

////////////////////////////////////////////////////////////////////////////////

static void vt100_update_cursor(struct VT100_info *vt100)
{
	if (vt100->buffer.batching)
		return;

	if (vt100->cursor.update) {
		vt100->cursor.update(
			vt100->cursor.x, 
			vt100->cursor.y, 
			vt100->screen.cols
		);
	}

	if (vt100->buffer.redraw)
			vt100->buffer.redraw();
}

static void vt100_ascii_control_code(struct VT100_info *vt100, const char c)
{
	switch (c) {
		case kASCII_HT:
			vt100->cursor.x = (vt100->cursor.x + TAB) & ~(TAB - 1);
			break;
		case kASCII_BS:
			if (vt100->cursor.x)
				vt100->cursor.x--;
			break;
		case kASCII_LF:
			++vt100->cursor.y;
		case kASCII_CR:
			vt100->cursor.x = 0;
			break;
		default:
			break;
	}
}

static void vt100_setpos(struct VT100_info *vt100, uint32_t x, uint32_t y)
{
	if (x < 1) x = 1;
	if (x >= vt100->screen.cols) x = vt100->screen.cols;
	if (y < 1) y = 1;
	if (y >= vt100->screen.rows) y = vt100->screen.rows;

	vt100->cursor.x = x - 1;
	vt100->cursor.y = y - 1;
	vt100_update_cursor(vt100);
}

static void vt100_clear(struct VT100_info *vt100)
{
	memsetw(
		vt100->buffer.ptr,
		(vt100->cursor.attr << 8) | ' ',
		vt100->screen.rows * vt100->screen.cols
	);

	vt100->cursor.y = 0;
	vt100->cursor.x = 0;
	vt100_update_cursor(vt100);
}

static void vt100_scroll(struct VT100_info *vt100)
{
	if (vt100->cursor.y >= vt100->screen.rows) {
		uint16_t tmp = vt100->cursor.y - vt100->screen.rows + 1;
		memcpy(
			vt100->buffer.ptr, 
			vt100->buffer.ptr + tmp * vt100->screen.cols, 
			(vt100->screen.rows - tmp) * vt100->screen.cols * sizeof(uint16_t)
		);
		memsetw(
			vt100->buffer.ptr + (vt100->screen.rows - tmp) * vt100->screen.cols,
			(vt100->cursor.attr << 8) | ' ',
			vt100->screen.cols
		);
		vt100->cursor.y = vt100->screen.rows - 1;
	}
	vt100_update_cursor(vt100);
}

static void vt100_wrap(struct VT100_info *vt100)
{
	if (vt100->cursor.x >= vt100->screen.cols) {
		vt100->cursor.x = 0;
		++vt100->cursor.y;
	}

	if (vt100->cursor.y >= vt100->screen.rows) {
		vt100_scroll(vt100);
	}
}

static void vt100_setattr(struct VT100_info *vt100, uint8_t value, uint8_t mask)
{
	uint8_t old_mask = ((mask & 0x01) == 1) ? 0xF0 : 0x0F;
	uint8_t attribute = vt100->cursor.attr;
	attribute = (attribute & old_mask) | (vt100_color_map[value] & mask);
	vt100->cursor.attr = attribute;
}

static void vt100_setattr_with_esccode(struct VT100_info *vt100)
{
	// We need to step through each of the arguments given in the escape
	// sequence and determine what we should do with it. The changes should be
	// applied to the cursor, and not the current character!
	for (uint32_t n = 0; n < vt100->escape.arg_count; ++n) {
		uint32_t arg = vt100->escape.args[n];

		if (arg == 0) {
			vt100_setattr(vt100, 7, 0x07);
			vt100_setattr(vt100, 0, 0x70);
		}
		else if (arg >= 30 && arg <= 37) {
			// Dark text colours
			vt100_setattr(vt100, arg - 30, 0x07);
		}
		else if (arg >= 40 && arg <= 47) {
			// Dark background colours
			vt100_setattr(vt100, arg - 40, 0x70);
		}
		else if (arg >= 90 && arg <= 97) {
			// Bright text colours
			vt100_setattr(vt100, arg - 90, 0x0F);
		}
		else if (arg >= 100 && arg <= 107) {
			// Bright background colours
			vt100_setattr(vt100, arg - 100, 0xF0);
		}
	}
}

static void vt100_putc(struct VT100_info *vt100, const char c)
{
	atom_t putc;
	atomic_start(putc);

	if (c <= kASCII_US || c == kASCII_DEL) {
		// Handle this as a control code.
		vt100_ascii_control_code(vt100, c);
	}
	else {
		// This is a printable character.
		uint32_t off = (vt100->cursor.y * vt100->screen.cols) + vt100->cursor.x;
		vt100->buffer.ptr[off] = (vt100->cursor.attr << 8) | c;
		++vt100->cursor.x;
	}

	vt100_wrap(vt100);
	vt100_update_cursor(vt100);

	atomic_end(putc);
}

static void vt100_restore(struct VT100_info *info)
{
	info->cursor.attr = 0x07;
	info->cursor.x = 0;
	info->cursor.y = 0;
	vt100_clear(info);
}

////////////////////////////////////////////////////////////////////////////////

static void vt100_write(struct device *dev, uint8_t c)
{
	if (!dev || !dev->info)
		return;
	struct VT100_info *vt100 = dev->info;

	if (vt100_is_esc(c)) {
		// We've been given an escape code. Escape codes are handled by a state
		// machine. Move into the appropriate mode. This will suppress visual
		// printing of characters until the escape/control code is fully 
		// handled.
		vt100->escape.parsing = 1;
		vt100->escape.idx = 0;
		vt100->escape.dirty_arg = 0;
		vt100->escape.bracket_type = 0;
		vt100->escape.ptr = vt100->escape.buffer;
		memset(vt100->escape.buffer, 0, BUFFER_LEN);
		memset(vt100->escape.args, 0, MAX_ARGS * sizeof(*vt100->escape.args));
		vt100->escape.arg_count = 0;
		
	}
	else if (vt100->escape.parsing) {
		// We're currently trying to parse an escape/control code.
		vt100_parse_control_char(vt100, c);
		vt100->escape.idx++;
	}
	else {
		// Nothing interesting to see here... print the character.
		vt100_putc(vt100, c);
	}
}

static int vt100_ready(struct device *dev __attribute__((unused)))
{
	return 1;
}

static void vt100_start_batch(struct device *dev)
{
	if (!dev || !dev->info)
		return;
	struct VT100_info *vt100 = dev->info;

	vt100->buffer.batching = 1;
} 

static void vt100_batch_commit(struct device *dev)
{
	if (!dev || !dev->info)
		return;
	struct VT100_info *vt100 = dev->info;
	
	vt100->buffer.batching = 0;
	vt100_update_cursor(vt100);
} 

////////////////////////////////////////////////////////////////////////////////

void VT100_prepare(struct boot_config *config)
{
	// Determine what display mode we're operating under and configure the
	// appropriate driver.
	if (config->vesa_mode == vga_mode_text) {
		__vt100_info.buffer.ptr = config->front_buffer;
		__vt100_info.screen.cols = config->screen_width;
		__vt100_info.screen.rows = config->screen_height;

		// The following is not strictly required, but is useful to have
		// calculated.
		__vt100_info.screen.width = __vt100_info.screen.cols * 9;
		__vt100_info.screen.height = __vt100_info.screen.rows * 16;

		__vt100_info.buffer.redraw = NULL;
		__vt100_info.cursor.update = vga_text_setpos;
	}
	else if (config->vesa_mode == vesa_mode_text) {
		uint32_t buffer_size = config->screen_width * config->screen_height;
		__vt100_info.buffer.ptr = kalloc(buffer_size * 2);
		memsetw(__vt100_info.buffer.ptr, 0, buffer_size);

		__vt100_info.buffer.redraw = vesa_console_redraw;
		__vt100_info.screen.width = config->screen_width;
		__vt100_info.screen.height = config->screen_height;
		__vt100_info.screen.cols = __vt100_info.screen.width / 9;
		__vt100_info.screen.rows = __vt100_info.screen.height / 16;
		__vt100_info.cursor.update = vesa_text_setpos;
		
		vesa_console_prepare(
			__vt100_info.buffer.ptr, 
			__vt100_info.screen.cols,
			__vt100_info.screen.rows
		);
	}
	else {
		struct panic_info info = (struct panic_info) {
			panic_general,
			"UNRECOGNISED DISPLAY CONFIGURATION",
			"The specified type of display configuration is not recognised."
		};
		panic(&info, NULL);
	}

	vt100_restore(&__vt100_info);

	// Setup the VT100 device.
	__vt100.dev_id = __VT100_ID;
	__vt100.name = "VT100";
	__vt100.kind = device_VT100;
	__vt100.opts = DP_WRITE | DP_ATOMIC_WRITE | DP_ALLOWS_ANSI;
	__vt100.write_byte = vt100_write;
	__vt100.can_write = vt100_ready;
	__vt100.start_batch = vt100_start_batch;
	__vt100.batch_commit = vt100_batch_commit;
	__vt100.info = &__vt100_info;

	// Bind the device to the appropriate handle.
	device_bind(&_VT100, &__vt100);
}

////////////////////////////////////////////////////////////////////////////////

static int vt100_is_esc(const char c)
{
	return (c == kASCII_ESC);
}

static void vt100_parse_control_char(struct VT100_info *vt100, const char c)
{
	// Is this a bracket type escape code?
	if (c == '[' && vt100->escape.idx == 0) {
		vt100->escape.bracket_type = 1;
		return;
	}

	// Does this character act as part of the arguments list, or delimiter?
	else if (vt100->escape.bracket_type && (c >= '0' && c <= ';')) {
		if (c == ':') {
			// Reserved. Ignore it.
			return;
		}
		else if (c == ';') {
			// Next parameter
			vt100->escape.arg_count++;
			vt100->escape.dirty_arg = 1;
		}
		else {
			// Part of the parameter value.
			if (vt100->escape.dirty_arg == 0) {
				vt100->escape.arg_count++;
			}
			vt100->escape.dirty_arg = 1;

			vt100->escape.args[vt100->escape.arg_count - 1] *= 10;
			vt100->escape.args[vt100->escape.arg_count - 1] += c - '0';

		}
		return;
	}

	// If not, is it an escape function code?
	// This first block is for Cursor Control.
	else if (vt100->escape.bracket_type && c == 'A') {
		// Cursor Up
		if (vt100->escape.arg_count == 0)
			vt100->escape.args[0] = 1;
		
		if (vt100->escape.args[0] >= vt100->cursor.y)
			vt100->cursor.y = 0;
		else
			vt100->cursor.y -= vt100->escape.args[0];
		
		vt100_update_cursor(vt100);
	}
	else if (vt100->escape.bracket_type && c == 'B') {
		// Cursor Down
		if (vt100->escape.arg_count == 0)
			vt100->escape.args[0] = 1;
		
		vt100->cursor.y += vt100->escape.args[0];
		vt100_wrap(vt100);
		
		vt100_update_cursor(vt100);
	}
	else if (vt100->escape.bracket_type && c == 'C') {
		// Cursor Forwards
		if (vt100->escape.arg_count == 0)
			vt100->escape.args[0] = 1;
		
		vt100->cursor.x += vt100->escape.args[0];
		vt100_wrap(vt100);
		
		vt100_update_cursor(vt100);
	}
	else if (vt100->escape.bracket_type && c == 'D') {
		// Cursor Backwards
		if (vt100->escape.arg_count == 0)
			vt100->escape.args[0] = 1;

		if (vt100->escape.args[0] >= vt100->cursor.x)
			vt100->cursor.x = 0;
		else
			vt100->cursor.x -= vt100->escape.args[0];
		
		vt100_update_cursor(vt100);
	}
	else if (vt100->escape.bracket_type && (c == 'H' || c == 'f')) {
		// Cursor Home / Force Cursor Position
		if (vt100->escape.arg_count < 2)
			return;
		
		vt100_setpos(vt100, vt100->escape.args[1], vt100->escape.args[0]);
	}
	else if (vt100->escape.bracket_type && c == 's') {
		// Save the current cursor position
		if (vt100->cursor.idx == MAX_STACK)
			return;

		vt100->cursor.x_stack[vt100->cursor.idx] = vt100->cursor.x;
		vt100->cursor.y_stack[vt100->cursor.idx] = vt100->cursor.y;
		vt100->cursor.attr_stack[vt100->cursor.idx] = vt100->cursor.attr;
		vt100->cursor.idx++;
	}
	else if (vt100->escape.bracket_type && c == 'u') {
		// Restore the current cursor position
		if (vt100->cursor.idx == 0)
			return;

		vt100->cursor.idx--;
		vt100->cursor.x = vt100->cursor.x_stack[vt100->cursor.idx];
		vt100->cursor.y = vt100->cursor.y_stack[vt100->cursor.idx];
		vt100->cursor.attr = vt100->cursor.attr_stack[vt100->cursor.idx];
		
		vt100_update_cursor(vt100);
	}

	// This second block is for Text Coloring and attribuites.
	else if (vt100->escape.bracket_type && c == 'm') {
		vt100_setattr_with_esccode(vt100);
	}

	// This third block is for screen management.
	else if (vt100->escape.bracket_type && c == 'J') {
		if (vt100->escape.args[0] == 2) {
			vt100_clear(vt100);
		}
	}

	// End the control sequence
	vt100->escape.parsing = 0;
}

