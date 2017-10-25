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

#ifndef __VKERNEL_TERM__
#define __VKERNEL_TERM__

#include <kern_types.h>

struct term_interface {
	uint8_t use_ansi;
	void(*set_cursor)(uint32_t, uint32_t);
	void(*get_cursor)(uint32_t *, uint32_t *);
	void(*update_cursor)(void);
	void(*puts)(const char *restrict);
	void(*putc)(const char);
	void(*clear)(uint8_t);
	void(*set_attribute)(uint8_t);
	void(*get_attribute)(uint8_t *);
	void(*set_default_attribute)(uint8_t);
	void(*restore_default_attribute)(void);
};

extern const uint32_t krnout;
extern const uint32_t dbgout;
extern const uint32_t allout;

void term_use_ansi(uint32_t handle, uint8_t use_ansi);

void term_bind_set_cursor(uint32_t handle, void(*fn)(uint32_t, uint32_t));
void term_bind_get_cursor(uint32_t handle, void(*fn)(uint32_t *, uint32_t *));
void term_bind_update_cursor(uint32_t handle, void(*fn)());
void term_bind_puts(uint32_t handle, void(*fn)(const char *restrict));
void term_bind_putc(uint32_t handle, void(*fn)(const char));
void term_bind_clear(uint32_t handle, void(*fn)(uint8_t));
void term_bind_set_attribute(uint32_t handle, void(*fn)(uint8_t));
void term_bind_get_attribute(uint32_t handle, void(*fn)(uint8_t *));
void term_bind_set_default_attribute(uint32_t handle, void(*fn)(uint8_t));
void term_bind_restore_default_attribute(uint32_t handle, void(*fn)());

void term_set_cursor(uint32_t handle, uint32_t x, uint32_t y);
void term_get_cursor(uint32_t handle, uint32_t *x, uint32_t *y);
void term_update_cursor(uint32_t handle);
void term_puts(uint32_t handle, const char *restrict str);
void term_putc(uint32_t handle, const char c);
void term_clear(uint32_t handle, uint8_t attribute);
void term_set_attribute(uint32_t handle, uint8_t attribute);
void term_get_attribute(uint32_t handle, uint8_t *attribute);
void term_set_default_attribute(uint32_t handle, uint8_t attribute);
void term_restore_default_attribute(uint32_t handle);

#endif