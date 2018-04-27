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

#ifndef _STDINT_H
#define _STDINT_H

#include <sys/cdefs.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef __uint8_t				uint8_t;
typedef __int8_t				int8_t;
typedef __uint16_t				uint16_t;
typedef __int16_t				int16_t;
typedef __uint32_t				uint32_t;
typedef __int32_t				int32_t;
typedef __uint64_t				uint64_t;
typedef __int64_t				int64_t;

typedef __veracyon_unative_t	uintptr_t;
typedef __veracyon_native_t 	intptr_t;

typedef __veracyon_size_t 		size_t;
typedef __veracyon_ssize_t 		ssize_t;

typedef __veracyon_time_t 		time_t;
typedef __veracyon_useconds_t 	useconds_t;
typedef __veracyon_suseconds_t	suseconds_t;

typedef uint32_t 				id_t;
typedef id_t	 				pid_t;
typedef id_t 	 				dev_t;

#define INT8_MAX				0x7F
#define INT16_MAX				0x7FFF
#define INT32_MAX				0x7FFFFFFF
#define INT64_MAX				0x7FFFFFFFFFFFFFFFLL
#define INT8_MIN				(-INT8_MAX-1)
#define INT16_MIN				(-INT16_MAX-1)
#define INT32_MIN				(-INT32_MAX-1)
#define INT64_MIN				(-INT64_MAX-1)
#define UINT8_MAX				0xFF
#define UINT16_MAX				0xFFFF
#define UINT32_MAX				0xFFFFFFFF
#define UINT64_MAX				0xFFFFFFFFFFFFFFFFLL

#define INTPTR_MAX				INT32_MAX
#define INTPTR_MIN				INT32_MIN
#define UINTPTR_MAX				UINT32_MAX
#define SIZE_MAX				UINT32_MAX

#ifdef __cplusplus
}
#endif

#endif