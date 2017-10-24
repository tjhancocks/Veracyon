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

#include <ansi.h>
#include <ascii.h>
#include <kern_types.h>
#include <kheap.h>
#include <kprint.h>

static uint8_t ansi_color_map[] = {
	0x88,
	0xCC,
	0xAA,
	0xEE,
	0x99,
	0xDD,
	0xBB,
	0xFF
};

void ansi_configure_graphic_rendition(
	struct term_interface *term,
	uint32_t *values, 
	uint32_t count
) {
	if (count == 1 && values[0] == 0) {
		if (term->restore_default_attribute)
			term->restore_default_attribute();
		return;
	}

	// Get the current attribute. We'll need it for updating.
	uint8_t attribute = 0;
	!term->get_attribute ? (0) : term->get_attribute(&attribute);

	// Step through the values and parse them appropriately.
	for (uint32_t n = 0; n < count; ++n) {
		if (values[n] >= 30 && values[n] <= 37) {
			// Dim Foreground Color
			uint8_t v = ansi_color_map[values[n] - 30];
			attribute = (attribute & 0xF0) | (v & 0x07);
		}
		else if (values[n] >= 40 && values[n] <= 47) {
			// Dim Background Color
			uint8_t v = ansi_color_map[values[n] - 40];
			attribute = (attribute & 0x0F) | (v & 0x70);
		}
		else if (values[n] >= 90 && values[n] <= 97) {
			// Bright Foreground Color
			uint8_t v = ansi_color_map[values[n] - 90];
			attribute = (attribute & 0xF0) | (v & 0x0F);
		}
		else if (values[n] >= 100 && values[n] <= 107) {
			// Dim Background Color
			uint8_t v = ansi_color_map[values[n] - 100];
			attribute = (attribute & 0x0F) | (v & 0xF0);
		}
	}

	!term->set_attribute ? (0) : term->set_attribute(attribute);
}

const char *ansi_determine_sequence(
	struct term_interface *term,
	const char *str
) {
	const char *ptr = str;
	uint32_t values[32] = { 0 };
	uint32_t value_count = 0;

	// While we're looking at invalid characters
	while (1) {
		// Is the character of the sequence valid? If not drop out and
		// send a debug error.
		if ((*ptr >= '0' && *ptr <= ';') == 0) {
			break;
		}

		// This is valid character. Is it a ";" ":" or "0-9"?
		if (*ptr == ':') {
			// Reserved currently - ignore it.
		}
		else if (*ptr == ';') {
			// Seperator
			++value_count;
		}
		else {
			// Number
			values[value_count] *= 10;
			values[value_count] += *ptr - '0';
		}

		// Move to the next character
		++ptr;
	}

	// We now need to determine the code to use to handle the escape sequence.
	// If there isn't a valid code found, then report an error and skip over to
	// the next character
	switch (*ptr) {
	case 'A':
		// Cursor Up.
		break;
	case 'B':
		// Cursor Down.
		break;
	case 'C':
		// Cursor Forward
		break;
	case 'D':
		// Cursor Backward
		break;
	case 'E':
		// Cursor Next Line
		break;
	case 'F':
		// Cursor Previous Line
		break;
	case 'G':
		// Cursor Horizontal Absolute
		break;
	case 'H':
		// Cursor Position
		break;
	case 'J':
		// Erase Display
		break;
	case 'K':
		// Erase Line
		break;
	case 'm':
		ansi_configure_graphic_rendition(term, values, value_count + 1);
		break;
	default:
		// Invalid escape code encounted.
		kdprint(dbgout, "Invalid ANSI escape code encountered: '%c'\n", *ptr);
		break;
	}

	return ++ptr;
}

void ansi_parse_and_display_string(
	struct term_interface *term, 
	const char *restrict str
) {
	if (!term)
		return;

	const char *ptr = str;

	// Step through each character of the string. If we encounter an escape code
	// then we need to handle it, otherwise just put the character.
	while (*ptr) {
		char c = *ptr;
		char next_c = *(ptr + 1);

		// Check for an escape code, followed by a '['
		if (c == ASCII_ESC && next_c == '[') {
			// Skip over the escap code sequence and handle the escape sequence
			// proper.
			ptr = ansi_determine_sequence(term, ptr + 2);
		}
		else {
			!term->putc ? (0) : term->putc(*ptr++);
		}
	}

	// Make sure the cursor is in the correct place at the end of this.
	!term->update_cursor ? (0) : term->update_cursor();
}
