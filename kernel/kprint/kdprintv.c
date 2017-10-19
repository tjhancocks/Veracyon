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

#include <kprint.h>
#include <memory.h>
#include <device/io/file.h>

#define BUFFER_LEN 1024
#define DIGIT_SOURCE "0123456789ABCDEF"

static uint32_t __kd_usn_to_str(char *restrict, uint32_t, uint8_t);
static uint32_t __kd_sn_to_str(char *restrict, int32_t, uint8_t);

const uint32_t krnout = __kKRNOUT;
const uint32_t dbgout = __kDBGOUT;
const uint32_t allout = __kALLOUT;

enum token 
{
	token_align_left = 1 << 1,
	token_zero_pad = 1 << 2,
	token_positive = 1 << 3,
	token_hex_prefix = 1 << 4,
	token_type_char = 1 << 5,
	token_type_signed = 1 << 6,
	token_type_unsigned = 1 << 7,
	token_type_hex = 1 << 8,
	token_type_string = 1 << 9,
	token_upper = 1 << 10
};

void kdprintv(uint32_t handle, const char *restrict fmt, va_list args)
{
	char buffer[BUFFER_LEN + 1] = { 0 };
	register char *out = buffer;
	register const char *in = fmt;

	while (*in && ((out - buffer) < BUFFER_LEN)) {

		// Check for a format marker.
		if (*in++ != '%') {
			*out++ = *(in - 1);
			continue;
		}

		// Check for an escaped %.
		if (*in == '%') {
			*out++ = *in++;
			continue;
		}

		// We know we're parsing a token.
		enum token mask = 0;
		uint32_t width = 0;

		// The basic syntax of a kprint token is:
		//	%[-][+][0][#][d|i|s|c|x|X|p|u]
		// The initial '-' indicates left alignment (default is right)
		// The next '+' indicates a leading + should be shown for +ve numbers
		// The 0 says the padding should be with 0's for numbers, not spaces.
		// The # is the number of characters wide the token should be rendered.
		//	Note: it must not have a leading a zero. See previous.
		// The final character is the type of data to be shown.
		if (*in == '-') {
			in++;
			mask |= token_align_left;
		}

		if (*in == '+') {
			in++;
			mask |= token_positive;
		}

		if (*in == '0') {
			in++;
			mask |= token_zero_pad;
		}

		while (*in >= '0' && *in <= '9') {
			uint32_t digit = (uint32_t)(*in++ - '0');
			width = (width == 0) ? digit : (width * 10) + digit;
		}

		switch (*in++) {
			case 'd':
			case 'i':
				mask |= token_type_signed;
				break;

			case 'u':
				mask |= token_type_unsigned;
				break;

			case 'p':
				width = 8;
				mask |= token_hex_prefix | token_zero_pad;
			case 'X':
				mask |= token_upper;
			case 'x':
				mask |= token_type_hex;
				break;

			case 'c':
				mask |= token_type_char;
				break;

			case 's':
				mask |= token_type_string;
				break;

			default:
				mask |= token_type_signed;
				break;
		}

		// We're now at a point where we can interpret the mask and print out
		// the information.
		char sub_buffer[BUFFER_LEN + 1] = { 0 };
		char tmp[BUFFER_LEN + 1] = { 0 };
		uint32_t tmp_len = 0;
		uint32_t sub_len = 0;
		uint32_t positive = 0;

		if (mask & token_hex_prefix) {
			sub_buffer[sub_len++] = '0';
			sub_buffer[sub_len++] = 'x';
		}

		if (mask & token_type_signed) {
			int32_t number = (int32_t)va_arg(args, int32_t);
			positive = 1;
			tmp_len = __kd_sn_to_str(tmp, number, 10);
		}
		else if (mask & token_type_unsigned) {
			uint32_t number = (int32_t)va_arg(args, uint32_t);
			positive = 1;
			tmp_len = __kd_usn_to_str(tmp, number, 10);
		}
		else if (mask & token_type_hex) {
			uint32_t number = (int32_t)va_arg(args, uint32_t);
			tmp_len = __kd_usn_to_str(tmp, number, 16);
		}
		else if (mask & token_type_string) {
			const char *str = (const char *)va_arg(args, uintptr_t);
			while (*str && tmp_len < BUFFER_LEN)
				tmp[tmp_len++] = *str++;
		}
		else if (mask & token_type_char) {
			tmp[tmp_len++] = (char)va_arg(args, uint32_t);
		}

		// Calculate the padding. If there is padding, then apply it.
		int32_t pad_len = width - tmp_len;
		pad_len = (mask & token_positive) ? pad_len - 1 : pad_len;

		while (pad_len-- > 0 && (mask & token_align_left) == 0)
			sub_buffer[sub_len++] = (mask & token_zero_pad) ? '0' : ' ';

		// Should we show the positive indicator?
		if (positive && (mask & token_positive))
			sub_buffer[sub_len++] = '+';

		// Print out the actual content.
		char *ptr = tmp;
		while (*ptr)
			sub_buffer[sub_len++] = *ptr++;

		// Trailing padding if required.
		while (pad_len-- > 0 && (mask & token_align_left))
			sub_buffer[sub_len++] = (mask & token_zero_pad) ? '0' : ' ';

		// The sub buffer is completed. Write it to the main buffer.
		ptr = sub_buffer;
		while (*ptr)
			*out++ = *ptr++;
	}

	// Finished constructing the output. Send it to the requested device.
	devio_puts(handle, buffer);
}

static uint32_t __kd_usn_to_str(char *restrict str, uint32_t num, uint8_t base)
{
	const char *digits = DIGIT_SOURCE;

	if (num == 0) {
		*str = *digits;
		return 1;
	}

	register uint32_t v = num;
	register uint32_t dV = 0;
	char buffer[32] = { 0 };
	register char *ptr = buffer + 30;
	register uint32_t len = 1;

	while (v >= (uint32_t)base) {
		dV = v % base;
		v /= base;
		*ptr-- = *(digits + dV);
		len++;
	}

	if (v > 0) {
		dV = v % base;
		*ptr-- = *(digits + dV);
		len++;
	}

	memcpy(str, ptr + 1, len);
	return len - 1;
}


static uint32_t __kd_sn_to_str(char *restrict str, int32_t num, uint8_t base)
{
	const char *digits = DIGIT_SOURCE;

	if (num == 0) {
		*str = *digits;
		return 1;
	}

	uint8_t neg = (num < 0);
	register uint32_t v = num * (neg ? -1 : 1);
	register uint32_t dV = 0;
	char buffer[32] = { 0 };
	register char *ptr = buffer + 30;
	register uint32_t len = 1;

	while (v >= (uint32_t)base) {
		dV = v % base;
		v /= base;
		*ptr-- = *(digits + dV);
		len++;
	}

	if (v > 0) {
		dV = v % base;
		*ptr-- = *(digits + dV);
		len++;
	}

	if (neg) {
		*ptr-- = '-';
		len++;
	}

	memcpy(str, ptr + 1, len);
	return len - 1;
}

