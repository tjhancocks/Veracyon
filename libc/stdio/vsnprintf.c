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

#if __libk__
#include <device/device.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#ifndef __DIGIT_SOURCE
#	define __DIGIT_SOURCE "0123456789ABCDEF"
#endif

#ifndef __PRINTF_BUFFER_LEN
#	define __PRINTF_BUFFER_LEN 1024
#endif

static uint32_t __kd_usn_to_str(char *restrict, uint64_t, uint8_t);
static uint32_t __kd_sn_to_str(char *restrict, int64_t, uint8_t);

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
	token_upper = 1 << 10,
	token_length_promote_char = 1 << 11,
	token_length_promote_short = 1 << 12,
	token_length_long = 1 << 13,
	token_length_long_long = 1 << 14,
	token_length_size = 1 << 15,
};

void vsnprintf(char *buffer, size_t n, const char *restrict fmt, va_list args)
{
	uint32_t buffer_len = n;
	register char *out = buffer;
	register const char *in = fmt;

	while (*in && ((uint32_t)(out - buffer) < buffer_len)) {

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

		switch (*in) {
			case 'h': {
				in++;
				if (*in == 'h') {
					in++;
					mask |= token_length_promote_char;
				}
				else {
					mask |= token_length_promote_short;
				}
				break;
			}
			case 'l': {
				in++;
				if (*in == 'l') {
					in++;
					mask |= token_length_long_long;
				}
				else {
					mask |= token_length_long;
				}
				break;
			}
			case 'z': {
				in++;
				mask |= token_length_size;
				break;
			}
			default:
				break;
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
		char sub_buffer[__PRINTF_BUFFER_LEN + 1] = { 0 };
		char tmp[__PRINTF_BUFFER_LEN + 1] = { 0 };
		uint32_t tmp_len = 0;
		uint32_t sub_len = 0;
		uint32_t positive = 0;

		if (mask & token_hex_prefix) {
			sub_buffer[sub_len++] = '0';
			sub_buffer[sub_len++] = 'x';
		}

		uint64_t value = 0;
		const char *string = NULL;
		char char_value = 0;

		if (mask & token_length_long_long) {
			value = (uint64_t)va_arg(args, uint64_t);
		}
		else if (mask & token_length_long) {
			value = (uint64_t)va_arg(args, uint32_t);
		}
		else if (mask & token_length_size) {
			value = (uint64_t)va_arg(args, size_t);
		}
		else if (mask & token_type_string) {
			string = (const char *)va_arg(args, uintptr_t);
		}
		else if (mask & token_type_string) {
			char_value = (char)va_arg(args, uint32_t);
		}
		else {
			value = (uint64_t)va_arg(args, uint32_t);
		}

		if (mask & token_type_signed) {
			positive = 1;
			tmp_len = __kd_sn_to_str(tmp, (int64_t)value, 10);
		}
		else if (mask & token_type_unsigned) {
			positive = 1;
			tmp_len = __kd_usn_to_str(tmp, value, 10);
		}
		else if (mask & token_type_hex) {
			tmp_len = __kd_usn_to_str(tmp, value, 16);
		}
		else if (mask & token_type_string) {
			while (*string && tmp_len < buffer_len)
				tmp[tmp_len++] = *string++;
		}
		else if (mask & token_type_char) {
			tmp[tmp_len++] = char_value;
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
}

static uint32_t __kd_usn_to_str(char *restrict str, uint64_t num, uint8_t base)
{
	const char *digits = __DIGIT_SOURCE;

	if (num == 0) {
		*str = *digits;
		return 1;
	}

	register uint64_t v = num;
	register uint64_t dV = 0;
	char buffer[64] = { 0 };
	register char *ptr = buffer + 62;
	register uint32_t len = 1;

	while (v >= (uint64_t)base) {
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


static uint32_t __kd_sn_to_str(char *restrict str, int64_t num, uint8_t base)
{
	const char *digits = __DIGIT_SOURCE;

	if (num == 0) {
		*str = *digits;
		return 1;
	}

	uint8_t neg = (num < 0);
	register uint64_t v = num * (neg ? -1 : 1);
	register uint64_t dV = 0;
	char buffer[64] = { 0 };
	register char *ptr = buffer + 62;
	register uint32_t len = 1;

	while (v >= (uint64_t)base) {
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