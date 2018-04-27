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

#ifndef __VKERNEL_KEYCODE__
#define __VKERNEL_KEYCODE__

#include <device/keyboard/scancode.h>

#define kKC_ANSI_ESC					0x00
#define kKC_ANSI_F1						0x01
#define kKC_ANSI_F2						0x02
#define kKC_ANSI_F3						0x03
#define kKC_ANSI_F4						0x04
#define kKC_ANSI_F5						0x05
#define kKC_ANSI_F6						0x06
#define kKC_ANSI_F7						0x07
#define kKC_ANSI_F8						0x08
#define kKC_ANSI_F9						0x09
#define kKC_ANSI_F10					0x0A
#define kKC_ANSI_F11					0x0B
#define kKC_ANSI_F12					0x0C
#define kKC_ANSI_F13					0x0D
#define kKC_ANSI_F14					0x0E
#define kKC_ANSI_F15					0x0F
#define kKC_ANSI_PRINT_SCR				0x10
#define kKC_ANSI_SCROLL_LOCK			0x11
#define kKC_ANSI_PAUSE					0x12
#define kKC_ANSI_BK_TICK				0x13
#define kKC_ANSI_1						0x14
#define kKC_ANSI_2						0x15
#define kKC_ANSI_3						0x16
#define kKC_ANSI_4						0x17
#define kKC_ANSI_5						0x18
#define kKC_ANSI_6						0x19
#define kKC_ANSI_7						0x1A
#define kKC_ANSI_8						0x1B
#define kKC_ANSI_9						0x1C
#define kKC_ANSI_0						0x1D
#define kKC_ANSI_MINUS 					0x1E
#define kKC_ANSI_EQUALS					0x1F
#define kKC_ANSI_BACKSPACE				0x20
#define kKC_ANSI_INSERT					0x21
#define kKC_ANSI_HOME					0x22
#define kKC_ANSI_PAGE_UP				0x23
#define kKC_ANSI_NUM_LOCK				0x24
#define kKC_ANSI_NUM_SLASH				0x25
#define kKC_ANSI_NUM_STAR				0x26
#define kKC_ANSI_NUM_MINUS				0x27
#define kKC_ANSI_TAB					0x28
#define kKC_ANSI_Q						0x29
#define kKC_ANSI_W						0x2A
#define kKC_ANSI_E						0x2B
#define kKC_ANSI_R						0x2C
#define kKC_ANSI_T						0x2D
#define kKC_ANSI_Y						0x2E
#define kKC_ANSI_U						0x2F
#define kKC_ANSI_I						0x30
#define kKC_ANSI_O						0x31
#define kKC_ANSI_P						0x32
#define kKC_ANSI_LEFT_BRACKET			0x33
#define kKC_ANSI_RIGHT_BRACKET			0x34
#define kKC_ANSI_ENTER 					0x35
#define kKC_ANSI_DEL					0x36
#define kKC_ANSI_END					0x37
#define kKC_ANSI_PAGE_DOWN				0x38
#define kKC_ANSI_NUM_7					0x39
#define kKC_ANSI_NUM_8					0x3A
#define kKC_ANSI_NUM_9					0x3B
#define kKC_ANSI_NUM_PLUS				0x3C
#define kKC_ANSI_CAPS_LOCK				0x3D
#define kKC_ANSI_A						0x3E
#define kKC_ANSI_S						0x3F
#define kKC_ANSI_D						0x40
#define kKC_ANSI_F						0x41
#define kKC_ANSI_G						0x42
#define kKC_ANSI_H						0x43
#define kKC_ANSI_J						0x44
#define kKC_ANSI_K						0x45
#define kKC_ANSI_L						0x46
#define kKC_ANSI_SEMI_COLON				0x47
#define kKC_ANSI_QUOTE					0x48
#define kKC_ANSI_HASH					0x49
#define kKC_ANSI_NUM_4					0x4A
#define kKC_ANSI_NUM_5					0x4B
#define kKC_ANSI_NUM_6					0x4C
#define kKC_ANSI_NUM_ENTER				0x4D
#define kKC_ANSI_LEFT_SHIFT				0x4E
#define kKC_ANSI_BACKSLASH				0x4F
#define kKC_ANSI_Z						0x50
#define kKC_ANSI_X						0x51
#define kKC_ANSI_C						0x52
#define kKC_ANSI_V						0x53
#define kKC_ANSI_B						0x54
#define kKC_ANSI_N						0x55
#define kKC_ANSI_M						0x56
#define kKC_ANSI_COMMA					0x57
#define kKC_ANSI_PERIOD					0x58
#define kKC_ANSI_SLASH					0x59
#define kKC_ANSI_RIGHT_SHIFT			0x5A
#define kKC_ANSI_NUM_1					0x5B
#define kKC_ANSI_NUM_2					0x5C
#define kKC_ANSI_NUM_3					0x5D
#define kKC_ANSI_LEFT_CTRL				0x5E
#define kKC_ANSI_LEFT_ALT				0x5F
#define kKC_ANSI_LEFT_SUPER				0x60
#define kKC_ANSI_SPACE					0x61
#define kKC_ANSI_RIGHT_SUPER			0x62
#define kKC_ANSI_RIGHT_ALT				0x63
#define kKC_ANSI_MENU					0x64
#define kKC_ANSI_RIGHT_CTRL				0x65
#define kKC_ANSI_UP_CURSOR				0x66
#define kKC_ANSI_DOWN_CURSOR			0x67
#define kKC_ANSI_LEFT_CURSOR			0x68
#define kKC_ANSI_RIGHT_CURSOR			0x69
#define kKC_ANSI_NUM_0					0x6A
#define kKC_ANSI_NUM_PERIOD				0x6B

#define kKC_ANSI_ESCAPE_CODE			0xE0
#define kKC_ANSI_UNKNOWN				0xFF

struct keycode_ascii_mapping 
{
	uint8_t keycode;
	char base;
	char left_shift;
	char right_shift;
	char left_control;
	char right_control;
	char left_alt;
	char right_alt;
};

/**
 Translate the specified keycode and modifier keys to an ASCII character. There
 are a number of mappings that result in NULL being return, but this is due to
 them not being printable characters, such as kKC_ANSI_F1 or kKC_ANSI_ESC.

 	- keycode: The keycode that should be translated.
 	- modifiers: The contextual state of modifier keys on the keyboard at the 
 		time of the key event.

 Returns:
 	A character.
 */
char keycode_to_ascii(
	uint8_t keycode, 
	enum key_modifiers modifiers
);

#endif