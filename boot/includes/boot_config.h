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

#ifndef __CORELOADER_BOOT_CONFIG__
#define __CORELOADER_BOOT_CONFIG__

enum filesystem_type
{
    filesystem_type_fat12 = 0x10,
    filesystem_type_fat16 = 0x11,
    filesystem_type_fat32 = 0x12,
    filesystem_type_ntfs = 0x20,
    filesystem_type_ext2 = 0x30,
    filesystem_type_ext3 = 0x31,
    filesystem_type_ext4 = 0x32,
};

enum vesa_mode
{
    vga_mode_text = 0,
    vesa_mode_text = 1,
    vesa_mode_gui = 2,
};

struct boot_config
{
    unsigned char filesystem;
    unsigned char vesa_mode;
    unsigned char kernel_name[31];
    unsigned short screen_width;
    unsigned short screen_height;
    unsigned char screen_depth;
    void *front_buffer;
    void *back_buffer;
    unsigned int background_color;
    unsigned int bytes_per_pixel;
    unsigned int bytes_per_line;
    unsigned int screen_size;
    unsigned int screen_x_max;
    unsigned int screen_y_max;
    unsigned int screen_x;
    unsigned int screen_y;
    unsigned short gdt_size;
    void *gdt_base;
    unsigned short idt_size;
    void *idt_base;
    unsigned short lower_memory;
    unsigned int upper_memory;
    void *mmap;
    unsigned short mmap_count;
    unsigned int next_page_table_frame;
    unsigned int next_frame;
    void *root_page_directory;
    void *panic_handler;
    void *interrupt_stubs;
} __attribute__((packed));

#endif