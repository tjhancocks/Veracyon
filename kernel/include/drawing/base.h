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

#ifndef __VKERNEL_DRAWING_BASE__
#define __VKERNEL_DRAWING_BASE__

#include <stdint.h>
#include <boot_config.h>

/**

 */
void drawing_prepare(struct boot_config *config);

/**

 */
void clear_screen(uint32_t color);

/**

 */
void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t clr);

/**

 */
void blit(void);

/**
 Invalidate the specified region of the screen.
 */
void invalidate_region(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

void draw_char_bmp(uint8_t c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);

#endif