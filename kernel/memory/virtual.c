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

#include <virtual.h>
#include <kprint.h>
#include <physical.h>
#include <panic.h>
#include <macro.h>
#include <memory.h>

struct page_table_ref
{
	uint32_t table;
	uintptr_t frame;
	uintptr_t linear;
	uintptr_t directory;
};

static uint32_t kernel_directory = 0;
static uint32_t page_size = 0x1000;
static uint32_t maximum_page_tables = (1024 * 8); // Allow 32GiB via page tables
static struct page_table_ref *page_table_lookup;
static uint32_t next_page_table_ref = 0;
static uintptr_t next_kernel_page = 0;

static uint32_t page_table_for_linear(uintptr_t address)
{
	return (address >> 22) & 0x3FF;
}

static uint32_t page_for_linear(uintptr_t address)
{
	return (address >> 12) & 0x3FF;
}

void reserve_page_table_lookup_memory()
{
	kprint("Reserving kernel working memory for page table lookup...\n");

	uint32_t page_table_ref_size = sizeof(struct page_table_ref);
	page_table_lookup = (void *)reserve_kernel_working_memory(
		page_table_ref_size * maximum_page_tables
	);
	memset(page_table_lookup, 0, page_table_ref_size * maximum_page_tables);

	kprint("Page table lookup is located at %p (%d entries)\n",
		page_table_lookup, maximum_page_tables);
}

static void push_page_table_ref(
	uint32_t table, 
	uintptr_t linear, 
	uintptr_t frame,
	uintptr_t directory
) {
	// TODO: Search for the next available table ref and loop to the start at
	// end of the list. If we manage to get back to the starting point, then
	// panic.

	page_table_lookup[next_page_table_ref].table = table;
	page_table_lookup[next_page_table_ref].directory = directory;
	page_table_lookup[next_page_table_ref].frame = frame;
	page_table_lookup[next_page_table_ref].linear = linear;

	kprint("Adding page table (%d) ref (%p :: %p)\n",
		table, frame, linear);

	next_page_table_ref++;
}

void identify_current_page_tables()
{
	kprint("Identifying all current page tables created by the bootloader\n");

	uint32_t *dir = (uint32_t *)REGISTER(cr3);

	if (!dir || ((uintptr_t)dir & 0xFFF)) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"INVALID PAGE DIRECTORY (CR3)",
			"Invalid cr3 value encountered whilst trying to identify existing "
			"page tables."
		};
		panic(&info);
	}

	// At this point _ALL_ page tables _should_ be identity mapped. This is an
	// assumption that is going to be made, as the alternative should 
	// technically be considered broken, and thus supplied by an invalid 
	// bootloader.
	for (uint32_t table = 0; table < 1024; ++table) {
		uint32_t frame = dir[table];

		// There is no page table at this entry, so skip.
		if ((frame & 0x1) == 0)
			continue;

		frame = frame & ~0xFFF;
		push_page_table_ref(table, frame, frame, (uintptr_t)dir);
	}
}

void determine_next_kernel_page()
{
	kprint("Determining the next available page for the kernel...\n");
	
	uintptr_t kernel_end = kernel_end_address();
	kprint("Kernel ends at %p\n", kernel_end);

	next_kernel_page = (kernel_end + page_size) & ~(page_size - 1);
	kprint("Page aligned kernel end is: %p\n", kernel_end);

	uint32_t page_table = page_table_for_linear(next_kernel_page);
	uint32_t page = page_for_linear(next_kernel_page);
	kprint("  -> page table %d, page %d\n", page_table, page);
}

void map_frame_to_page(uint32_t *dir, uintptr_t frame, uintptr_t linear)
{
	uint32_t page_table = page_table_for_linear(linear);
	uint32_t page = page_for_linear(linear);

	kprint("  -> map_frame_to_page(%p, %p, %p)\n", 
		dir, frame, linear);
	kprint("  -> page table %d, page %d\n", page_table, page);

	// Check if the page table exists.
	if ((dir[page_table] & 0x1) == 0) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"MISSING PAGE TABLE",
			"There was no valid page table present in the specified page "
			"directory."
		};
		panic(&info);
	}

	uint32_t *table = (uint32_t *)(dir[page_table] & ~0xFFF);
	if (table[page] & 0x1) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"ATTEMPT TO DUPLICATE PAGE",
			"There was an attempt to allocate an already in use page."
		};
		panic(&info);
	}

	table[page] = (frame & ~0xFFF) | 0x3;

	// Invalidate the address to flush the appropriate TLB cache.
	__asm__ __volatile__(
		"invlpg (%0)" :: "r"(linear) : "memory"
	);

	kprint("  -> mapping completed.\n");
}

uintptr_t kpage_map_to_kernel(uintptr_t frame)
{
	kprint("Mapping the frame %p into the kernel at %p\n", 
		frame, next_kernel_page);

	// Get information about the next available page in kernel space.
	uintptr_t linear = next_kernel_page;
	
	// Advance to the next available frame.
	next_kernel_page += page_size;

	// Map the frame accordingly in the kernel page directory
	map_frame_to_page(
		(uint32_t *)kernel_directory,
		frame,
		linear
	);

	return linear;
}

void kpage_table_allocate(uint32_t table)
{
	uint32_t *dir = (uint32_t *)REGISTER(cr3);
	kprint("Allocating page table %d in %p\n", table, (uintptr_t)dir);

	if (dir[table] & 0x1) {
		kprint("Page table %d is already allocated in %p\n", 
			table, (uintptr_t)dir);
		return;
	}

	uintptr_t frame = kframe_alloc();
	kprint("Assigning frame %p to page table %d\n", frame, table);

	// Map the frame into kernel space so that we can actually operate upon it,
	// and then clear it.
	uintptr_t linear = kpage_map_to_kernel(frame);
	memset(linear, 0, 0x1000);

	kprint("New page table cleared successfully. Linear address: %p\n", linear);

	// Now the page table needs to be put in place, by adding it to the page
	// directory. It is the frame that should be placed into the directory, as
	// the memory management unit only works on physical addresses.
	dir[table] = (frame & ~0xFFF) | 0x3;

	// Flush the entire TLB.
	__asm__ __volatile__(
		"movl %%cr3, %%eax\n"
		"movl %%eax, %%cr3"
		::: "%eax"
	);

	// Finally, push the new page table information
	push_page_table_ref(table, linear, frame, dir);

	kprint("New page table (%d, %p) successfully added\n", table, frame);
}

void virtual_memory_prepare(struct boot_config *config)
{
	kprint("Preparing Virtual Memory Manager...\n");

	// Check to ensure the root page directory is what is expected!
	uint32_t cr3 = REGISTER(cr3);
	if (cr3 != (uintptr_t)(config->root_page_directory)) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"VIRTUAL MEMORY MANAGER CORRUPTED",
			"The root page directory setup by the bootloader is in an"
			" unexpected location."
		};
		panic(&info);
	}

	kernel_directory = config->root_page_directory;
	kprint("Root page directory setup by boot loader is at %p\n",
		kernel_directory);

	reserve_page_table_lookup_memory();
	identify_current_page_tables();
	determine_next_kernel_page();

	kpage_table_allocate(1);
}
