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

#include <term.h>
#include <ansi.h>

const uint32_t krnout = 0x00000001;
const uint32_t dbgout = 0x00000002;
const uint32_t allout = 0xFFFFFFFF; 

#define kMAX_TERM_BINDINGS	32

static struct term_interface term_bindings[kMAX_TERM_BINDINGS];

void term_use_ansi(uint32_t handle, uint8_t use_ansi)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].use_ansi = use_ansi;
}

void term_bind_set_cursor(uint32_t handle, void(*fn)(uint32_t, uint32_t))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].set_cursor = fn;
}

void term_bind_get_cursor(uint32_t handle, void(*fn)(uint32_t *, uint32_t *))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].get_cursor = fn;
}

void term_bind_update_cursor(uint32_t handle, void(*fn)())
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].update_cursor = fn;
}

void term_bind_puts(uint32_t handle, void(*fn)(const char *restrict))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].puts = fn;
}

void term_bind_putc(uint32_t handle, void(*fn)(const char))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].putc = fn;
}

void term_bind_clear(uint32_t handle, void(*fn)(uint8_t))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].clear = fn;
}

void term_bind_set_attribute(uint32_t handle, void(*fn)(uint8_t))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].set_attribute = fn;
}

void term_bind_get_attribute(uint32_t handle, void(*fn)(uint8_t *))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].get_attribute = fn;
}

void term_bind_set_default_attribute(uint32_t handle, void(*fn)(uint8_t))
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].set_default_attribute = fn;
}

void term_bind_restore_default_attribute(uint32_t handle, void(*fn)())
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n))
			term_bindings[n].restore_default_attribute = fn;
}


////////////////////////////////////////////////////////////////////////////////

void term_set_cursor(uint32_t handle, uint32_t x, uint32_t y)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].set_cursor)
			term_bindings[n].set_cursor(x, y);
}

void term_get_cursor(uint32_t handle, uint32_t *x, uint32_t *y)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].get_cursor)
			term_bindings[n].get_cursor(x, y);
}

void term_update_cursor(uint32_t handle)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].update_cursor)
			term_bindings[n].update_cursor();
}

void term_puts(uint32_t handle, const char *restrict str)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n) {
		if (handle & (1 << n) && term_bindings[n].puts) {
			if (term_bindings[n].use_ansi)
				ansi_parse_and_display_string(&term_bindings[n], str);
			else
				term_bindings[n].puts(str);
		}
	}
}

void term_putc(uint32_t handle, const char c)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].putc)
			term_bindings[n].putc(c);
}

void term_clear(uint32_t handle, uint8_t attribute)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].clear)
			term_bindings[n].clear(attribute);
}

void term_set_attribute(uint32_t handle, uint8_t attribute)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].set_attribute)
			term_bindings[n].set_attribute(attribute);
}

void term_get_attribute(uint32_t handle, uint8_t *attribute)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].get_attribute)
			term_bindings[n].get_attribute(attribute);
}

void term_set_default_attribute(uint32_t handle, uint8_t attribute)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].set_default_attribute)
			term_bindings[n].set_default_attribute(attribute);
}

void term_restore_default_attribute(uint32_t handle)
{
	for (uint8_t n = 0; n < kMAX_TERM_BINDINGS; ++n)
		if (handle & (1 << n) && term_bindings[n].restore_default_attribute)
			term_bindings[n].restore_default_attribute();
}

