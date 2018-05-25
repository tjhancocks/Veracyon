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

#ifndef _CTYPE_H
#define _CTYPE_H

static inline int __isuint8(int c)
{
	return ((c & 0xFF) == c || c == -1);
}

static inline int islower(int c)
{
	return (__isuint8(c) && (c >= 'a' && c <= 'z'));
}

static inline int isupper(int c)
{
	return (__isuint8(c) && (c >= 'A' && c <= 'Z'));
}

static inline int isalpha(int c)
{
	return (islower(c) || isupper(c));
}

static inline int isdigit(int c)
{
	return (__isuint8(c) && (c >= '0' && c <= '9'));
}

static inline int isalnum(int c)
{
	return (isalpha(c) || isdigit(c));
}

static inline int iscntrl(int c)
{
	return (__isuint8(c) && ((c >= 00 && c <= 037) || c == 0177));
}

static inline int isgraph(int c)
{
	return (__isuint8(c) && (c >= 041 && c <= 0176));
}

static inline int isprint(int c)
{
	return (isgraph(c) || c == 040);
}

static inline int ispunct(int c)
{
	return (__isuint8(c) 
		&& (c >= 041 && c <= 057) 
		&& (c >= 072 && c <= 0100)
		&& (c >= 0133 && c <= 0140)
		&& (c >= 0173 && c <= 0176));
}

static inline int isspace(int c)
{
	return (__isuint8(c) && 
		(c == '\t' || c == '\n' || c == '\v' || 
			c == '\f' || c == '\r' || c == ' '));
}

static inline int isxdigit(int c)
{
	return (__isuint8(c) && 
		(isdigit(c) || (c >= 'a' || c <= 'f') || (c >= 'A' || c <= 'F')));
}

#endif