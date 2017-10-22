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

////////////////////////////////////////////////////////////////////////////////

struct scancode_info default_scancode_info[] = 
{
	{
		.index = 0,
		.scancode = 0x01,
		.name = "ESC",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x02,
		.name = "1",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x03,
		.name = "2",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x04,
		.name = "3",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x05,
		.name = "4",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x06,
		.name = "5",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x07,
		.name = "6",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x08,
		.name = "7",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x09,
		.name = "8",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x0A,
		.name = "9",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x0B,
		.name = "0",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x0C,
		.name = "-",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x0D,
		.name = "=",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x0E,
		.name = "Backspace",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x0F,
		.name = "Tab",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x10,
		.name = "q",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x11,
		.name = "w",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x12,
		.name = "e",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x13,
		.name = "r",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x14,
		.name = "t",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x15,
		.name = "y",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x16,
		.name = "u",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x17,
		.name = "i",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x18,
		.name = "o",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x19,
		.name = "p",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x1A,
		.name = "[",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x1B,
		.name = "]",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x1C,
		.name = "New Line",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x1D,
		.name = "Left Control",
		.pressed = 1,
		.modifier = keyboard_mod_left_control,
	},
	{
		.index = 0,
		.scancode = 0x1E,
		.name = "a",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x1F,
		.name = "s",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x20,
		.name = "d",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x21,
		.name = "f",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x22,
		.name = "g",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x23,
		.name = "h",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x24,
		.name = "j",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x25,
		.name = "k",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x26,
		.name = "l",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.scancode = 0x27,
		.name = ";",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x28,
		.name = "'",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x29,
		.name = "`",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x2A,
		.name = "Left Shift",
		.pressed = 1,
		.modifier = keyboard_mod_left_shift,
	},
	{
		.index = 0,
		.scancode = 0x2B,
		.name = "\\",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x2C,
		.name = "z",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x2D,
		.name = "x",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x2E,
		.name = "c",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x2F,
		.name = "v",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x30,
		.name = "b",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x31,
		.name = "n",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x32,
		.name = "m",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x33,
		.name = ",",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x34,
		.name = ".",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x35,
		.name = "/",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x36,
		.name = "Right Shift",
		.pressed = 1,
		.modifier = keyboard_mod_right_shift,
	},
	{
		.index = 0,
		.scancode = 0x37,
		.name = "Keypad *",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x38,
		.name = "Left Alt",
		.pressed = 1,
		.modifier = keyboard_mod_left_alt,
	},
	{
		.index = 0,
		.scancode = 0x39,
		.name = "Space",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x3A,
		.name = "Capslock",
		.pressed = 1,
		.modifier = keyboard_mod_capslock,
	},
	{
		.index = 0,
		.scancode = 0x3B,
		.name = "F1",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x3C,
		.name = "F2",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x3D,
		.name = "F3",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x3E,
		.name = "F4",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x3F,
		.name = "F5",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x40,
		.name = "F6",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x41,
		.name = "F7",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x42,
		.name = "F8",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x43,
		.name = "F9",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x44,
		.name = "F10",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x45,
		.name = "Numlock",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x46,
		.name = "Scroll Lock",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x47,
		.name = "Keypad 7",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x48,
		.name = "Keypad 8",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x49,
		.name = "Keypad 9",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x4A,
		.name = "Keypad -",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x4B,
		.name = "Keypad 4",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x4C,
		.name = "Keypad 5",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x4D,
		.name = "Keypad 6",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x4E,
		.name = "Keypad +",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x4F,
		.name = "Keypad 1",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x50,
		.name = "Keypad 2",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x51,
		.name = "Keypad 3",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x52,
		.name = "Keypad 0",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x53,
		.name = "Keypad .",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x57,
		.name = "F11",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x58,
		.name = "F12",
		.pressed = 1,
		.modifier = keyboard_mod_none,
	},
	{
		.index = 0,
		.scancode = 0x9D,
		.name = "Left Control",
		.pressed = 0,
		.modifier = keyboard_mod_left_control,
	},
	{
		.index = 0,
		.scancode = 0xAA,
		.name = "Left Shift",
		.pressed = 0,
		.modifier = keyboard_mod_left_shift,
	},
	{
		.index = 0,
		.scancode = 0xB6,
		.name = "Right Shift",
		.pressed = 0,
		.modifier = keyboard_mod_right_shift,
	},
	{
		.index = 0,
		.scancode = 0xB8,
		.name = "Left Alt",
		.pressed = 0,
		.modifier = keyboard_mod_left_alt,
	},
	{
		.index = 0,
		.scancode = 0xBA,
		.name = "Capslock",
		.pressed = 0,
		.modifier = keyboard_mod_capslock,
	},
};

////////////////////////////////////////////////////////////////////////////////

struct scancode_translation_unit default_translation[] = 
{
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{'1',  '!',  '!',  '1',  '1',  '1',  '1'},
	{'2',  '@',  '@',  '2',  '2',  '2',  '2'},
	{'3',  '#',  '#',  '3',  '3',  '3',  '3'},
	{'4',  '$',  '$',  '4',  '4',  '4',  '4'},
	{'5',  '%',  '%',  '5',  '5',  '5',  '5'},
	{'6',  '^',  '^',  '6',  '6',  '6',  '6'},
	{'7',  '&',  '&',  '7',  '7',  '7',  '7'},
	{'8',  '*',  '*',  '8',  '8',  '8',  '8'},
	{'9',  '(',  '(',  '9',  '9',  '9',  '9'},
	{'0',  ')',  ')',  '0',  '0',  '0',  '0'},
	{'-',  '_',  '_',  '-',  '-',  '-',  '-'},
	{'=',  '+',  '+',  '=',  '=',  '=',  '='},
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
	{0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09},
	{'q',  'Q',  'Q',  'q',  'q',  'q',  'q'},
	{'w',  'W',  'W',  'w',  'w',  'w',  'w'},
	{'e',  'E',  'E',  'e',  'e',  'e',  'e'},
	{'r',  'R',  'R',  'r',  'r',  'r',  'r'},
	{'t',  'T',  'T',  't',  't',  't',  't'},
	{'y',  'Y',  'Y',  'y',  'y',  'y',  'y'},
	{'u',  'U',  'U',  'u',  'u',  'u',  'u'},
	{'i',  'I',  'I',  'i',  'i',  'i',  'i'},
	{'o',  'O',  'O',  'o',  'o',  'o',  'o'},
	{'p',  'P',  'P',  'p',  'p',  'p',  'p'},
	{'[',  '{',  '{',  '[',  '[',  '[',  '['},
	{']',  '}',  '}',  ']',  ']',  ']',  ']'},
	{'\n', '\n', '\n', '\n', '\n', '\n', '\n'},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{'a',  'A',  'A',  'a',  'a',  'a',  'a'},
	{'s',  'S',  'S',  's',  's',  's',  's'},
	{'d',  'D',  'D',  'd',  'd',  'd',  'd'},
	{'f',  'F',  'F',  'f',  'f',  'f',  'f'},
	{'g',  'G',  'G',  'g',  'g',  'g',  'g'},
	{'h',  'H',  'H',  'h',  'h',  'h',  'h'},
	{'j',  'J',  'J',  'j',  'j',  'j',  'j'},
	{'k',  'K',  'K',  'k',  'k',  'k',  'k'},
	{'l',  'L',  'L',  'l',  'l',  'l',  'l'},
	{';',  ':',  ':',  ';',  ';',  ';',  ';'},
	{'\'',  '"',  '"', '\'', '\'', '\'', '\''},
	{'`',  '~',  '~',  '`',  '`',  '`',  '`'},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{'\\',  '|',  '|', '\\', '\\', '\\', '\\'},
	{'z',  'Z',  'Z',  'z',  'z',  'z',  'z'},
	{'x',  'X',  'X',  'x',  'x',  'x',  'x'},
	{'c',  'C',  'C',  'c',  'c',  'c',  'c'},
	{'v',  'V',  'V',  'v',  'v',  'v',  'v'},
	{'b',  'B',  'B',  'b',  'b',  'b',  'b'},
	{'n',  'N',  'N',  'n',  'n',  'n',  'n'},
	{'m',  'M',  'M',  'm',  'm',  'm',  'm'},
	{',',  '<',  '<',  ',',  ',',  ',',  ','},
	{'.',  '>',  '>',  '.',  '.',  '.',  '.'},
	{'/',  '?',  '?',  '/',  '/',  '/',  '/'},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{'*',  '*',  '*',  '*',  '*',  '*',  '*'},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' '},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{'7',  '7',  '7',  '7',  '7',  '7',  '7'},
	{'8',  '8',  '8',  '8',  '8',  '8',  '8'},
	{'9',  '9',  '9',  '9',  '9',  '9',  '9'},
	{'-',  '-',  '-',  '-',  '-',  '-',  '-'},
	{'4',  '4',  '4',  '4',  '4',  '4',  '4'},
	{'5',  '5',  '5',  '5',  '5',  '5',  '5'},
	{'6',  '6',  '6',  '6',  '6',  '6',  '6'},
	{'+',  '+',  '+',  '+',  '+',  '+',  '+'},
	{'1',  '1',  '1',  '1',  '1',  '1',  '1'},
	{'2',  '2',  '2',  '2',  '2',  '2',  '2'},
	{'3',  '3',  '3',  '3',  '3',  '3',  '3'},
	{'0',  '0',  '0',  '0',  '0',  '0',  '0'},
	{'.',  '.',  '.',  '.',  '.',  '.',  '.'},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

////////////////////////////////////////////////////////////////////////////////

struct scancode_info scancode_info_make(uint8_t raw_code)
{
	uint32_t default_scancode_count = (uint32_t)(
		sizeof(default_scancode_info) / sizeof(struct scancode_info)
	);
	for (uint32_t i = 0; i < default_scancode_count; ++i) {
		struct scancode_info info = default_scancode_info[i];
		if (info.scancode == raw_code) {
			info.index = i;
			return info;
		}
	}

	return (struct scancode_info) {
		.index = 0xFF,
		.scancode = raw_code,
		.name = "Unknown Key",
		.pressed = 0,
		.modifier = keyboard_mod_none,
	};
}

uint8_t translate_scancode(struct scancode_info info, uint8_t modifiers)
{
	uint32_t default_translation_count = (uint32_t)(
		sizeof(default_translation) / sizeof(struct scancode_translation_unit)
	);

	// If we're using an invalid scancode then just return NULL.
	if (info.index >= default_translation_count)
		return 0x0;
	
	// Fetch the appropriate translation unit.
	struct scancode_translation_unit translation = default_translation[
		info.index
	];

	kprint("Translating: %02x\n", info.scancode);

	// Is a modifier in effect? If not just return the base code.
	if (modifiers & keyboard_mod_left_shift)
		return translation.left_shift;
	else if (modifiers & keyboard_mod_right_shift)
		return translation.right_shift;
	else if (modifiers & keyboard_mod_left_control)
		return translation.left_control;
	else if (modifiers & keyboard_mod_right_control)
		return translation.right_control;
	else if (modifiers & keyboard_mod_left_alt)
		return translation.left_alt;
	else if (modifiers & keyboard_mod_right_alt)
		return translation.right_alt;
	else
		return translation.base;
}

