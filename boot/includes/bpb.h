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

#ifndef __BIOS_PARAMETER_BLOCK__
#define __BIOS_PARAMETER_BLOCK__

struct bios_parameter_block
{
	uint8_t jmp_code[3];			// 0x7c00
	uint8_t oem[8];					// 0x7c00 + 3
	uint16_t bytes_per_sector;		// 0x7c00 + 11
	uint8_t sectors_per_cluster;	// 0x7c00 + 13
	uint16_t reserved_sectors;		// 0x7c00 + 14
	uint8_t number_of_fats;			// 0x7c00 + 16
	uint16_t root_entries;			// 0x7c00 + 17
	uint16_t total_sectors;			// 0x7c00 + 19
	uint8_t media;					// 0x7c00 + 21
	uint16_t sectors_per_fat;		// 0x7c00 + 22
	uint16_t sectors_per_track;		// 0x7c00 + 24
	uint16_t heads_per_cylinder;	// 0x7c00 + 26
	uint32_t hidden_sectors;		// 0x7c00 + 28
	uint32_t total_sectors_big;		// 0x7c00 + 32
	uint8_t drive_number;			// 0x7c00 + 36
	uint8_t unused;					// 0x7c00 + 37
	uint8_t boot_signature;			// 0x7c00 + 38
	uint32_t serial_number;			// 0x7c00 + 39
	uint8_t volume_label[11];		// 0x7c00 + 43
	uint8_t file_system[8];			// 0x7c00 + 54
} __attribute__((packed));

#endif