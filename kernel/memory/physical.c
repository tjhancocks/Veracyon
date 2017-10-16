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

// Working memory begins 8MiB from the end of the Kernel.
static uintptr_t working_memory = 0;
static uint32_t frame_size = 0x1000; // 4 KiB
static uint32_t frame_total = 0;
static uint32_t frame_free = 0;
static uint32_t *available_frame = NULL;

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

static void frame_stack_prepare(struct boot_config *config)
{
	kprint("Setting up stack for free/available frames (");
	kprint("%dKiB, ", config->upper_memory);

	frame_total = config->upper_memory / 4;
	kprint("%d)\n", frame_total);

	available_frame = (uint32_t *)working_memory;
	available_frame += frame_total;
	kprint("available_frame stack bottom: %p\n", available_frame);
}

static void push_free_frame(uint32_t frame __attribute__((unused)))
{
	*available_frame-- = frame;
	frame_free++;
}

static void push_free_frames(uint32_t first, uint32_t last)
{
	kprint("\tpush_free_frames(%08x, %08x)\n", first, last);
	for (uint32_t n = first; n < last; n += frame_size)
		push_free_frame(n);

	kprint("\tavailable_frame is now: %p\n", available_frame);
	kprint("\tframe_free is %d\n", frame_free);
}

static void search_physical_frames(struct boot_config *config)
{
	kprint("Searching for frames allocated by bootloader...\n");

	// The bootloader should have passed us information about all the frames
	// it has allocated. The next_frame field should therefore be the first
	// available frame to us.
	unsigned int first_free_frame = config->next_frame;
	kprint("first unused frame: %08x\n", first_free_frame);

	// We need to work through the memory map in order to determine what holes
	// are present in physical memory. From this we can know how what regions
	// are available.
	struct mmap_entry *entry = config->mmap;
	kprint("mmap_count = %d\n", config->mmap_count);
	for (unsigned short n = 0; n < config->mmap_count; entry++, ++n) {
		kprint("%p | %p (%08x), present: %d\n", 
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

	kprint("Total number of free/available frames: %d\n", frame_free);
}

void physical_memory_prepare(struct boot_config *config)
{
	kprint("Preparing Physical Memory Manager...\n");

	working_memory = kernel_end_addr - 0x800000;
	kprint("End of kernel is located at: %p\n", kernel_end_addr);
	kprint("Working memory is located at: %p\n", working_memory);

	frame_stack_prepare(config);
	search_physical_frames(config);
}

uintptr_t kalloc_frame()
{
	if (frame_free == 0) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"PHYSICAL MEMORY EXHAUSTED",
			"The system has run out of physical page frames to allocate,"
			"and must therefore terminate."
		};
		panic(&info);
	}

	available_frame++;
	frame_free--;

	kprint("Allocated physical frame: %p (%d)\n",
		*available_frame, frame_free - 1);

	return *available_frame;
}

void kfree_frame(uintptr_t frame)
{
	// TODO: There should really be some verification about frame validity here.
	push_free_frame(frame);
	kprint("Freed physical frame: %p\n", frame);
}
