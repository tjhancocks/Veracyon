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

#include <arch/arch.h>
#include <kprint.h>
#include <memory.h>

////////////////////////////////////////////////////////////////////////////////

static struct gdt_segment _segments[6] = { 0 };
static struct gdt_pointer _gdt = { 0 };
static struct tss _tss = { 0 };

extern void gdt_load(struct gdt_pointer *);

////////////////////////////////////////////////////////////////////////////////

static void gdt_set_descriptor(
	uint8_t n, 
	uint32_t base, 
	uint32_t limit, 
	uint8_t access, 
	uint8_t flags
) {
	kdprint(
		COM1, 
		"Set GDT Descriptor(%d): base=%p limit=%05x access=%02x flags=%02x\n", 
		n, base, limit, access, flags
	);
	_segments[n].base_lo = (base & 0xFFFF);
	_segments[n].base_mid = ((base >> 16) & 0xFF);
	_segments[n].base_hi = ((base >> 24) & 0xFF);
	_segments[n].limit_lo = (limit & 0xFFFF);
	_segments[n].limit_hi = ((limit >> 16) & 0x0F);
	_segments[n].access = access;
	_segments[n].granularity = (flags & 0x0F);
}

void gdt_set_tss_descriptor(uint8_t n, uint32_t ss0, uint32_t esp0)
{
	kdprint(COM1, "Set GDT::TSS Descriptor(%d)\n", n);
	uintptr_t base = (uintptr_t)&_tss;
	uintptr_t limit = (uintptr_t)base + sizeof(_tss);

	gdt_set_descriptor(n, base, limit, 0xE9, 0x00);

	memset(&_tss, 0x00, sizeof(_tss));

	_tss.ss0 = ss0;
	_tss.esp0 = esp0;
	_tss.cs = 0x8;
	_tss.ss = 0x10;
	_tss.ds = 0x10;
	_tss.es = 0x10;
	_tss.fs = 0x10;
	_tss.gs = 0x10;
	_tss.iopb = sizeof(_tss);
}

void gdt_prepare(void)
{
	kdprint(COM1, "Preparing kernel Global Descriptor Table\n");

	// NULL Segment
	gdt_set_descriptor(0, 0, 0, 0, 0);

	// Kernel Code Segment
	gdt_set_descriptor(1, 0, 0xFFFFF, 0x9A, 0xC);

	// Kernel Data Segment
	gdt_set_descriptor(2, 0, 0xFFFFF, 0x92, 0xC);

	// Userland Code Segment
	gdt_set_descriptor(3, 0, 0xFFFFF, 0xFA, 0xC);

	// Userland Data Segment
	gdt_set_descriptor(4, 0, 0xFFFFF, 0xF2, 0xC);

	// TSS
	gdt_set_tss_descriptor(5, 0x10, 0x0);

	// Make sure the pointer is correct, and load it.
	_gdt.size = (sizeof(_segments) - 1);
	_gdt.offset = (uintptr_t)&_segments;

	gdt_load(&_gdt);
}