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

#include <physical.h>
#include <kprint.h>
#include <device/io/file.h>
#include <null.h>
#include <panic.h>

extern uintptr_t *kernel_end;
extern uintptr_t *kernel_start;
static uintptr_t kernel_end_addr = (uintptr_t)&kernel_end;

static uintptr_t working_memory = 0;

static const uint32_t frame_size = 0x1000; // 4 KiB Frames
static uint32_t frame_total = 0;
static uint32_t free_frame_count = 0;
static uint32_t *free_frame_stack = NULL;

enum mmap_entry_type
{
	mmap_usable = 1,
	mmap_reserved = 2,
	mmap_acpi_reclaimable = 3,
	mmap_acpi_nvs = 4,
	mmap_bad = 5
};

struct mmap_entry 
{
	uint64_t offset;
	uint64_t size;
	uint32_t type;
	uint32_t attributes;
} __attribute__((packed));

uintptr_t kernel_end_address()
{
	return kernel_end_addr;
}

uintptr_t reserve_kernel_working_memory(uint32_t length)
{
	if ((working_memory + length) > kernel_end_address()) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"UNABLE TO ALLOCATE WORKING MEMORY",
			"The Kernel has been unable to allocate working memory to itself."
		};
		panic(&info, NULL);
	}

	kdprint(dbgout, "Reserving %d bytes of kernel working memory at %p\n",
		length, working_memory);
	
	uintptr_t address = working_memory;
	working_memory += length;

	kdprint(dbgout, 
		"There is now %d bytes of kernel working memory remaining.\n",
		kernel_end_address() - working_memory);

	return address;
}

static void frame_stack_prepare(struct boot_config *config)
{
	kdprint(dbgout, "Setting up stack for free/available frames (");
	kdprint(dbgout, "%dKiB, ", config->upper_memory);

	frame_total = config->upper_memory / 4;
	kdprint(dbgout, "%d)\n", frame_total);

	free_frame_stack = (uint32_t *)reserve_kernel_working_memory(frame_total);
	free_frame_stack += frame_total;
	kdprint(dbgout, "free_frame_stack stack bottom: %p\n", free_frame_stack);
}

static void push_free_frame(uint32_t frame __attribute__((unused)))
{
	*free_frame_stack-- = frame;
	free_frame_count++;
}

static void push_free_frames(uint32_t first, uint32_t last)
{
	kdprint(dbgout, "\tpush_free_frames(%08x, %08x)\n", first, last);
	for (uint32_t n = first; n < last; n += frame_size)
		push_free_frame(n);

	kdprint(dbgout, "\tfree_frame_stack is now: %p\n", free_frame_stack);
	kdprint(dbgout, "\tfree_frame_count is %d\n", free_frame_count);
}

static void search_physical_frames(struct boot_config *config)
{
	kdprint(dbgout, "Searching for frames allocated by bootloader...\n");

	// The bootloader should have passed us information about all the frames
	// it has allocated. The next_frame field should therefore be the first
	// available frame to us.
	unsigned int first_free_frame = config->next_frame;
	kdprint(dbgout, "first unused frame: %08x\n", first_free_frame);

	// We need to work through the memory map in order to determine what holes
	// are present in physical memory. From this we can know how what regions
	// are available.
	struct mmap_entry *entry = config->mmap;
	kdprint(dbgout, "mmap_count = %d\n", config->mmap_count);
	for (unsigned short n = 0; n < config->mmap_count; entry++, ++n) {
		kdprint(dbgout, "%p | %p (%08x), present: %d\n", 
			(uint32_t)entry->offset, 
			(uint32_t)(entry->offset + entry->size), 
			(uint32_t)entry->size,
			(uint32_t)entry->type);

		// If the region is not usable, then skip over it.
		if (entry->type != mmap_usable)
			continue;	

		// Is the first free frame after the end of the region. If so then skip
		// the region.
		if (first_free_frame >= (entry->offset + entry->size))
			continue;

		// Is the first free frame before the region? If so, then pull it
		// forwards to the beginning of the region.
		if (first_free_frame < entry->offset)
			first_free_frame = entry->offset;

		// Determine the last frame...
		uint32_t last_frame = (entry->offset + entry->size) - frame_size;

		// ...and then add the frames for the region
		push_free_frames(first_free_frame, last_frame);
	}

	kdprint(dbgout, "Total number of free/available frames: %d\n", 
		free_frame_count);
}

void physical_memory_prepare(struct boot_config *config)
{
	kdprint(dbgout, "Preparing Physical Memory Manager...\n");

	working_memory = kernel_end_addr - (4 * 1024 * 1024);
	kdprint(dbgout, "End of kernel is located at: %p\n", kernel_end_addr);
	kdprint(dbgout, "Working memory is located at: %p\n", working_memory);

	frame_stack_prepare(config);
	search_physical_frames(config);
}

uintptr_t kframe_alloc()
{
	if (free_frame_count == 0) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"PHYSICAL MEMORY EXHAUSTED",
			"The system has run out of physical page frames to allocate,"
			"and must therefore terminate."
		};
		panic(&info, NULL);
	}

	free_frame_stack++;
	free_frame_count--;

	kdprint(dbgout, "Allocated physical frame: %p (%d)\n",
		*free_frame_stack, free_frame_count - 1);

	return *free_frame_stack;
}

void kframe_free(uintptr_t frame)
{
	// TODO: There should really be some verification about frame validity here.
	push_free_frame(frame);
	kdprint(dbgout, "Freed physical frame: %p\n", frame);
}
