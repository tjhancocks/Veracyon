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

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

typedef unsigned char 			__uint8_t;
typedef signed char 			__int8_t;
typedef unsigned short 			__uint16_t;
typedef signed short 			__int16_t;
typedef unsigned int 			__uint32_t;
typedef signed int 				__int32_t;
typedef unsigned long long 		__uint64_t;
typedef signed long long 		__int64_t;

#ifndef __SIZE_TYPE__
#	define __SIZE_TYPE__ 		__uint32_t 
#endif
typedef __SIZE_TYPE__ 			__veracyon_size_t;

typedef __int32_t 				__veracyon_ssize_t;
typedef __int32_t 				__veracyon_time_t;

typedef __int32_t 				__veracyon_native_t;
typedef __uint32_t 				__veracyon_unative_t;

#endif