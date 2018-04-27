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

#include <device/keyboard/keyboard.h>
#include <device/keyboard/scancode.h>
#include <device/keyboard/keycode.h>
#include <ascii.h>

////////////////////////////////////////////////////////////////////////////////

struct keycode_ascii_mapping keycode_map[] = 
{
	{kKC_ANSI_1, '1', '!', '!', '1', '1', '1', '1'},
	{kKC_ANSI_2, '2', '@', '@', '2', '2', '2', '2'},
	{kKC_ANSI_3, '3', '#', '#', '3', '3', '3', '3'},
	{kKC_ANSI_4, '4', '$', '$', '4', '4', '4', '4'},
	{kKC_ANSI_5, '5', '%', '%', '5', '5', '5', '5'},
	{kKC_ANSI_6, '6', '^', '^', '6', '6', '6', '6'},
	{kKC_ANSI_7, '7', '&', '&', '7', '7', '7', '7'},
	{kKC_ANSI_8, '8', '*', '*', '8', '8', '8', '8'},
	{kKC_ANSI_9, '9', '(', '(', '9', '9', '9', '9'},
	{kKC_ANSI_0, '0', ')', ')', '0', '0', '0', '0'},
	{kKC_ANSI_MINUS, '-', '_', '_', '-', '-', '-', '-'},
	{kKC_ANSI_EQUALS, '=', '+', '+', '=', '=', '=', '='},
	{kKC_ANSI_BACKSPACE, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20},
	{kKC_ANSI_NUM_SLASH, '/', '/', '/', '/', '/', '/', '/'},
	{kKC_ANSI_NUM_STAR, '*', '*', '*', '*', '*', '*', '*'},
	{kKC_ANSI_NUM_MINUS, '-', '-', '-', '-', '-', '-', '-'},
	{kKC_ANSI_TAB, '\t', '\t', '\t', '\t', '\t', '\t', '\t'},
	{kKC_ANSI_Q, 'q', 'Q', 'Q', 'q', 'q', 'q', 'q'},
	{kKC_ANSI_W, 'w', 'W', 'W', 'w', 'w', 'w', 'w'},
	{kKC_ANSI_E, 'e', 'E', 'E', 'e', 'e', 'e', 'e'},
	{kKC_ANSI_R, 'r', 'R', 'R', 'r', 'r', 'r', 'r'},
	{kKC_ANSI_T, 't', 'T', 'T', 't', 't', 't', 't'},
	{kKC_ANSI_Y, 'y', 'Y', 'Y', 'y', 'y', 'y', 'y'},
	{kKC_ANSI_U, 'u', 'U', 'U', 'u', 'u', 'u', 'u'},
	{kKC_ANSI_I, 'i', 'I', 'I', 'i', 'i', 'i', 'i'},
	{kKC_ANSI_O, 'o', 'O', 'O', 'o', 'o', 'o', 'o'},
	{kKC_ANSI_P, 'p', 'P', 'P', 'p', 'p', 'p', 'p'},
	{kKC_ANSI_LEFT_BRACKET, '[', '{', '{', '[', '[', '[', '['},
	{kKC_ANSI_RIGHT_BRACKET, ']', '}', '}', ']', ']', ']', ']'},
	{kKC_ANSI_NUM_ENTER, 0xA, 0xA, 0xA, 0xA, 0xA, 0xA, 0xA},
	{kKC_ANSI_DEL, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F},
	{kKC_ANSI_NUM_7, '7', '7', '7', '7', '7', '7', '7'},
	{kKC_ANSI_NUM_8, '8', '8', '8', '8', '8', '8', '8'},
	{kKC_ANSI_NUM_9, '9', '9', '9', '9', '9', '9', '9'},
	{kKC_ANSI_NUM_PLUS, '+', '+', '+', '+', '+', '+', '+'},
	{kKC_ANSI_A, 'a', 'A', 'A', 'a', 'a', 'a', 'a'},
	{kKC_ANSI_S, 's', 'S', 'S', 's', 's', 's', 's'},
	{kKC_ANSI_D, 'd', 'D', 'D', 'd', 'd', 'd', 'd'},
	{kKC_ANSI_F, 'f', 'F', 'F', 'f', 'f', 'f', 'f'},
	{kKC_ANSI_G, 'g', 'G', 'G', 'g', 'g', 'g', 'g'},
	{kKC_ANSI_H, 'h', 'H', 'H', 'h', 'h', 'h', 'h'},
	{kKC_ANSI_J, 'j', 'J', 'J', 'j', 'j', 'j', 'j'},
	{kKC_ANSI_K, 'k', 'K', 'K', 'k', 'k', 'k', 'k'},
	{kKC_ANSI_L, 'l', 'L', 'L', 'l', 'l', 'l', 'l'},
	{kKC_ANSI_SEMI_COLON, ';', ':', ':', ';', ';', ';', ';'},
	{kKC_ANSI_QUOTE, '\'', '"', '"', '\'', '\'', '\'', '\''},
	{kKC_ANSI_HASH, '#', '#', '#', '#', '#', '#', '#'},
	{kKC_ANSI_NUM_4, '4', '4', '4', '4', '4', '4', '4'},
	{kKC_ANSI_NUM_5, '5', '5', '5', '5', '5', '5', '5'},
	{kKC_ANSI_NUM_6, '6', '6', '6', '6', '6', '6', '6'},
	{kKC_ANSI_NUM_ENTER, 0xA, 0xA, 0xA, 0xA, 0xA, 0xA, 0xA},
	{kKC_ANSI_BACKSLASH, '\\', '\\', '\\', '\\', '\\', '\\', '\\'},
	{kKC_ANSI_Z, 'z', 'Z', 'Z', 'z', 'z', 'z', 'z'},
	{kKC_ANSI_X, 'x', 'X', 'X', 'x', 'x', 'x', 'x'},
	{kKC_ANSI_C, 'c', 'C', 'C', 'c', 'c', 'c', 'c'},
	{kKC_ANSI_V, 'v', 'V', 'V', 'v', 'v', 'v', 'v'},
	{kKC_ANSI_B, 'b', 'B', 'B', 'b', 'b', 'b', 'b'},
	{kKC_ANSI_N, 'n', 'N', 'N', 'n', 'n', 'n', 'n'},
	{kKC_ANSI_M, 'm', 'M', 'M', 'm', 'm', 'm', 'm'},
	{kKC_ANSI_COMMA, ',', '<', '<', ',', ',', ',', ','},
	{kKC_ANSI_PERIOD, '.', '>', '>', '.', '.', '.', '.'},
	{kKC_ANSI_SLASH, '/', '?', '?', '/', '/', '/', '/'},
	{kKC_ANSI_NUM_1, '1', '1', '1', '1', '1', '1', '1'},
	{kKC_ANSI_NUM_2, '2', '2', '2', '2', '2', '2', '2'},
	{kKC_ANSI_NUM_3, '3', '3', '3', '3', '3', '3', '3'},
	{kKC_ANSI_SPACE, ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{kKC_ANSI_NUM_0, '0', '0', '0', '0', '0', '0', '0'},
	{kKC_ANSI_NUM_PERIOD, '.', '.', '.', '.', '.', '.', '.'},
};

////////////////////////////////////////////////////////////////////////////////

char keycode_to_ascii(
	uint8_t keycode, 
	enum key_modifiers modifiers
) {
	uint32_t count = (uint32_t)(
		sizeof(keycode_map) / sizeof(sizeof(struct keycode_ascii_mapping))
	);
	for (uint32_t n = 0; n < count; ++n) {

		if (keycode_map[n].keycode != keycode) {
			continue;
		}

		if (modifiers & key_modifier_left_shift) {
			return keycode_map[n].left_shift;
		}
		else if (modifiers & key_modifier_right_shift) {
			return keycode_map[n].right_shift;
		}
		else if (modifiers & key_modifier_left_control) {
			return keycode_map[n].left_control;
		}
		else if (modifiers & key_modifier_right_control) {
			return keycode_map[n].right_control;
		}
		else if (modifiers & key_modifier_left_alt) {
			return keycode_map[n].left_alt;
		}
		else if (modifiers & key_modifier_right_alt) {
			return keycode_map[n].right_alt;
		}
		else {
			return keycode_map[n].base;
		}
	}

	return 0x00;
}

