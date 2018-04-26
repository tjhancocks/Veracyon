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

#ifndef _STDIO_H
#define _STDIO_H

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __vFILE;
typedef struct __vFILE FILE;

void printf(const char *restrict, ...);
void fprintf(FILE *, const char *restrict, ...);
void vfprintf(FILE *, const char *restrict, va_list);
void vprintf(const char *restrict, va_list);
void vsprintf(char *, const char *restrict, va_list);
void vsnprintf(char *, size_t, const char *restrict, va_list);
void sprintf(char *, const char *restrict, ...);
void snprintf(char *, size_t, const char *restrict, ...);

FILE *stdin;
FILE *stdout;
FILE *stderr;

FILE *COM1;
FILE *VT100;

#ifdef __cplusplus
}
#endif

#endif