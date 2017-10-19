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

#ifndef __VKERNEL_VIRTUAL_MEMORY_MANAGER__
#define __VKERNEL_VIRTUAL_MEMORY_MANAGER__

#include <kern_types.h>
#include <boot_config.h>

// TODO: This structure needs to be fully/correctly laid out
struct page {
    uint32_t present: 1;
    uint32_t readwrite: 1;
    uint32_t unused: 10;
    uint32_t frame: 20;
} __attribute__((packed));

// TODO: This structure needs to be fully/correctly laid out
struct page_table {
    uint32_t present: 1;
    uint32_t readwrite: 1;
    uint32_t unused: 10;
    uint32_t frame: 20;
} __attribute__((packed));

struct virtual_address_space {
    uintptr_t directory_frame;
    uintptr_t directory_address;
    uintptr_t next_page_table;
    uintptr_t page_table_address[1024];
};

void virtual_memory_prepare(struct boot_config *config);

#endif