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

#ifndef __VKERNEL_KEYBOARD_SCANCODE__
#define __VKERNEL_KEYBOARD_SCANCODE__

#include <kern_types.h>

enum keyboard_modifier
{
	keyboard_mod_none = 1 << 0,
	keyboard_mod_left_shift = 1 << 1,
	keyboard_mod_right_shift = 1 << 2,
	keyboard_mod_left_control = 1 << 3,
	keyboard_mod_right_control = 1 << 4,
	keyboard_mod_left_alt = 1 << 5,
	keyboard_mod_right_alt = 1 << 6,
	keyboard_mod_capslock = 1<< 7
};

struct scancode_info
{
	uint8_t index;
	uint8_t scancode;
	const char *name;
	uint8_t pressed;
	enum keyboard_modifier modifier;
};

struct scancode_translation_unit
{
	uint8_t base;
	uint8_t left_shift;
	uint8_t right_shift;
	uint8_t left_control;
	uint8_t right_control;
	uint8_t left_alt;
	uint8_t right_alt;
};

struct scancode_info scancode_info_make(uint8_t raw_code);
uint8_t translate_scancode(struct scancode_info info, uint8_t modifiers);

#endif