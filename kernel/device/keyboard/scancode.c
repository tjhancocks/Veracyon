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

#include <device/keyboard/scancode.h>
#include <device/keyboard/keycode.h>
#include <kheap.h>
#include <memory.h>

////////////////////////////////////////////////////////////////////////////////

static struct scancode_info default_set[] = {
	{0x01, kKC_ANSI_ESC, 0x00, 0x00, "escape pressed"},
	{0x02, kKC_ANSI_1, 0x00, 0x00, "1 pressed"},
	{0x03, kKC_ANSI_2, 0x00, 0x00, "2 pressed"},
	{0x04, kKC_ANSI_3, 0x00, 0x00, "3 pressed"},
	{0x05, kKC_ANSI_4, 0x00, 0x00, "4 pressed"},
	{0x06, kKC_ANSI_5, 0x00, 0x00, "5 pressed"},
	{0x07, kKC_ANSI_6, 0x00, 0x00, "6 pressed"},
	{0x08, kKC_ANSI_7, 0x00, 0x00, "7 pressed"},
	{0x09, kKC_ANSI_8, 0x00, 0x00, "8 pressed"},
	{0x0A, kKC_ANSI_9, 0x00, 0x00, "9 pressed"},
	{0x0B, kKC_ANSI_0, 0x00, 0x00, "0 pressed"},
	{0x0C, kKC_ANSI_MINUS, 0x00, 0x00, "minus pressed"},
	{0x0D, kKC_ANSI_EQUALS, 0x00, 0x00, "equals pressed"},
	{0x0E, kKC_ANSI_BACKSPACE, 0x00, 0x00, "backspace pressed"},
	{0x0F, kKC_ANSI_TAB, 0x00, 0x00, "tab pressed"},
	{0x10, kKC_ANSI_Q, 0x00, 0x00, "Q pressed"},
	{0x11, kKC_ANSI_W, 0x00, 0x00, "W pressed"},
	{0x12, kKC_ANSI_E, 0x00, 0x00, "E pressed"},
	{0x13, kKC_ANSI_R, 0x00, 0x00, "R pressed"},
	{0x14, kKC_ANSI_T, 0x00, 0x00, "T pressed"},
	{0x15, kKC_ANSI_Y, 0x00, 0x00, "Y pressed"},
	{0x16, kKC_ANSI_U, 0x00, 0x00, "U pressed"},
	{0x17, kKC_ANSI_I, 0x00, 0x00, "I pressed"},
	{0x18, kKC_ANSI_O, 0x00, 0x00, "O pressed"},
	{0x19, kKC_ANSI_P, 0x00, 0x00, "P pressed"},
	{0x1A, kKC_ANSI_LEFT_BRACKET, 0x00, 0x00, "left bracket pressed"},
	{0x1B, kKC_ANSI_RIGHT_BRACKET, 0x00, 0x00, "right bracket pressed"},
	{0x1C, kKC_ANSI_ENTER, 0x00, 0x00, "enter pressed"},
	{0x1D, kKC_ANSI_LEFT_CTRL, 0x00, 0x00, "left control pressed"},
	{0x1E, kKC_ANSI_A, 0x00, 0x00, "A pressed"},
	{0x1F, kKC_ANSI_S, 0x00, 0x00, "S pressed"},
	{0x20, kKC_ANSI_D, 0x00, 0x00, "D pressed"},
	{0x21, kKC_ANSI_F, 0x00, 0x00, "F pressed"},
	{0x22, kKC_ANSI_G, 0x00, 0x00, "G pressed"},
	{0x23, kKC_ANSI_H, 0x00, 0x00, "H pressed"},
	{0x24, kKC_ANSI_J, 0x00, 0x00, "J pressed"},
	{0x25, kKC_ANSI_K, 0x00, 0x00, "K pressed"},
	{0x26, kKC_ANSI_L, 0x00, 0x00, "L pressed"},
	{0x27, kKC_ANSI_SEMI_COLON, 0x00, 0x00, "semi colon pressed"},
	{0x28, kKC_ANSI_QUOTE, 0x00, 0x00, "quote pressed"},
	{0x29, kKC_ANSI_BK_TICK, 0x00, 0x00, "back tick pressed"},
	{0x2A, kKC_ANSI_LEFT_SHIFT, 0x00, 0x00, "left shift pressed"},
	{0x2B, kKC_ANSI_BACKSLASH, 0x00, 0x00, "backslash pressed"},
	{0x2C, kKC_ANSI_Z, 0x00, 0x00, "Z pressed"},
	{0x2D, kKC_ANSI_X, 0x00, 0x00, "X pressed"},
	{0x2E, kKC_ANSI_C, 0x00, 0x00, "C pressed"},
	{0x2F, kKC_ANSI_V, 0x00, 0x00, "V pressed"},
	{0x30, kKC_ANSI_B, 0x00, 0x00, "B pressed"},
	{0x31, kKC_ANSI_N, 0x00, 0x00, "N pressed"},
	{0x32, kKC_ANSI_M, 0x00, 0x00, "M pressed"},
	{0x33, kKC_ANSI_COMMA, 0x00, 0x00, "comma pressed"},
	{0x34, kKC_ANSI_PERIOD, 0x00, 0x00, "period pressed"},
	{0x35, kKC_ANSI_SLASH, 0x00, 0x00, "slash pressed"},
	{0x36, kKC_ANSI_RIGHT_SHIFT, 0x00, 0x00, "right shift pressed"},
	{0x37, kKC_ANSI_NUM_STAR, 0x00, 0x00, "keypad star pressed"},
	{0x38, kKC_ANSI_LEFT_ALT, 0x00, 0x00, "left alt pressed"},
	{0x39, kKC_ANSI_SPACE, 0x00, 0x00, "space pressed"},
	{0x3A, kKC_ANSI_CAPS_LOCK, 0x00, 0x00, "caps lock pressed"},
	{0x3B, kKC_ANSI_F1, 0x00, 0x00, "F1 pressed"},
	{0x3C, kKC_ANSI_F2, 0x00, 0x00, "F2 pressed"},
	{0x3D, kKC_ANSI_F3, 0x00, 0x00, "F3 pressed"},
	{0x3E, kKC_ANSI_F4, 0x00, 0x00, "F4 pressed"},
	{0x3F, kKC_ANSI_F5, 0x00, 0x00, "F5 pressed"},
	{0x40, kKC_ANSI_F6, 0x00, 0x00, "F6 pressed"},
	{0x41, kKC_ANSI_F7, 0x00, 0x00, "F7 pressed"},
	{0x42, kKC_ANSI_F8, 0x00, 0x00, "F8 pressed"},
	{0x43, kKC_ANSI_F9, 0x00, 0x00, "F9 pressed"},
	{0x44, kKC_ANSI_F10, 0x00, 0x00, "F10 pressed"},
	{0x45, kKC_ANSI_NUM_LOCK, 0x00, 0x00, "number lock pressed"},
	{0x46, kKC_ANSI_SCROLL_LOCK, 0x00, 0x00, "scroll lock pressed"},
	{0x47, kKC_ANSI_NUM_7, 0x00, 0x00, "keypad 7 pressed"},
	{0x48, kKC_ANSI_NUM_8, 0x00, 0x00, "keypad 8 pressed"},
	{0x49, kKC_ANSI_NUM_9, 0x00, 0x00, "keypad 9 pressed"},
	{0x4A, kKC_ANSI_NUM_MINUS, 0x00, 0x00, "keypad minus pressed"},
	{0x4B, kKC_ANSI_NUM_4, 0x00, 0x00, "keypad 4 pressed"},
	{0x4C, kKC_ANSI_NUM_5, 0x00, 0x00, "keypad 5 pressed"},
	{0x4D, kKC_ANSI_NUM_9, 0x00, 0x00, "keypad 6 pressed"},
	{0x4E, kKC_ANSI_NUM_PLUS, 0x00, 0x00, "keypad plus pressed"},
	{0x4F, kKC_ANSI_NUM_1, 0x00, 0x00, "keypad 1 pressed"},
	{0x50, kKC_ANSI_NUM_2, 0x00, 0x00, "keypad 2 pressed"},
	{0x51, kKC_ANSI_NUM_3, 0x00, 0x00, "keypad 3 pressed"},
	{0x52, kKC_ANSI_NUM_0, 0x00, 0x00, "keypad 0 pressed"},
	{0x53, kKC_ANSI_NUM_PERIOD, 0x00, 0x00, "keypad dot pressed"},
	{0x57, kKC_ANSI_F11, 0x00, 0x00, "F11 pressed"},
	{0x58, kKC_ANSI_F12, 0x00, 0x00, "F12 pressed"},
	{0xE0, kKC_ANSI_ESCAPE_CODE, 0x00, 0xE0, "(escape code)"},
};

////////////////////////////////////////////////////////////////////////////////

static uint8_t _current_state = 0;
static enum key_modifiers _current_modifiers = 0;
static enum key_state _current_keystate = 0;

struct keyevent *keyevent_make(uint8_t scancode)
{
	// Prepare a new keyevent
	struct keyevent *event = kalloc(sizeof(*event));
	memset(event, 0, sizeof(*event));

	// Set some initial defaults
	event->keycode = kKC_ANSI_UNKNOWN;
	event->pressed = scancode & 0x80 ? 0 : 1;

	// Search for the scancode to correctly update the information
	uint32_t count = (uint32_t) (
		sizeof(default_set) / sizeof(struct scancode_info)
	);
	for (uint32_t n = 0; n < count; ++n) {
		// We're looking for scancodes of the appropriate state.
		if (default_set[n].state != _current_state)
			continue;

		// Next we're looking for the particular scancode
		if (default_set[n].raw_code != (scancode & ~0x80))
			continue;

		// We have the correct scancode entry now. Determine what to do.
		event->keycode = default_set[n].keycode;

		// Reset the current state of the state machine.
		_current_state = 0;

		// Certain keys needs to alter the state of the scancode state machine.
		enum key_modifiers new_modifier = 0;
		switch (event->keycode) {
			case kKC_ANSI_LEFT_SHIFT:
				new_modifier = key_modifier_left_shift;
				break;
			case kKC_ANSI_RIGHT_SHIFT:
				new_modifier = key_modifier_right_shift;
				break;
			case kKC_ANSI_LEFT_CTRL:
				new_modifier = key_modifier_left_control;
				break;
			case kKC_ANSI_RIGHT_CTRL:
				new_modifier = key_modifier_right_control;
				break;
			case kKC_ANSI_LEFT_ALT:
				new_modifier = key_modifier_left_alt;
				break;
			case kKC_ANSI_RIGHT_ALT:
				new_modifier = key_modifier_right_alt;
				break;
			case kKC_ANSI_ESCAPE_CODE:
				_current_state = kKC_ANSI_ESCAPE_CODE;
				break;
		}

		// Update the modifiers. We need to either clear or set based on the
		// pressed state.
		if (event->pressed)
			_current_modifiers |= new_modifier;
		else
			_current_modifiers &= ~(new_modifier);

		// We've found what we're looking for.
		break;
	}

	// Ensure the contextual state is known
	event->modifiers = _current_modifiers;
	event->state = _current_keystate;
	return event;
};
