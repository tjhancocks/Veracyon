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
#include <physical.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <macro.h>
#include <panic.h>
#include <memory.h>

#define PAGE_MAX_ENTRIES 	1024

#define kPAGE_ALLOCATED 			1
#define kNO_PAGE_ALLOCATED 			0
#define kNO_PAGE_TABLE_ALLOCATED 	-1

static struct virtual_address_space *kernel_address_space = NULL;
static uintptr_t first_available_kernel_address = 0;
static uintptr_t first_kernel_address = 0;
static uint32_t page_size = 0x1000;

static uintptr_t first_page_table_address = 0;
static uintptr_t last_page_table_address = 0;
static uint32_t total_page_tables = 1024;


////////////////////////////////////////////////////////////////////////////////

static uint32_t page_table_for_address(uintptr_t address)
{
	return (address >> 22) & 0x3FF;
}

static uint32_t page_for_address(uintptr_t address)
{
	return (address >> 12) & 0x3FF;
}

int is_page_allocated(uintptr_t address)
{
	uint32_t table = page_table_for_address(address);
	uint32_t page = page_for_address(address);

	// Check to see if the required page table exists.
	struct page_table *directory = (void *)(
		kernel_address_space->directory_address
	);

	if (directory[table].present == 0)
		return kNO_PAGE_TABLE_ALLOCATED;

	// Check to see if the required page is present. For this we need to find
	// the linear address of the page table.
	struct page *page_table = (void *)(
		kernel_address_space->page_table_address[table]
	);

	if (page_table[page].present == 0)
		return kNO_PAGE_ALLOCATED;

	// At this point we can assume that the page is allocated.
	return kPAGE_ALLOCATED;
}

uintptr_t first_available_kernel_page(void)
{
	// Step through kernel pages to try and find the first available one.
	for (
		uintptr_t address = first_available_kernel_address;
		address < 0xFFFFFFFF;
		address += page_size
	) {
		switch (is_page_allocated(address)) {
		case kPAGE_ALLOCATED:
			break;

		case kNO_PAGE_TABLE_ALLOCATED:
		case kNO_PAGE_ALLOCATED:
			return address;
		}
	}

	// At this point we can safely assume that there are _no_ available kernel
	// space addresses left.
	struct panic_info info = (struct panic_info) {
		panic_memory,
		"KERNEL ADDRESS SPACE EXHAUSTED",
		"The kernel has exhausted all of the address space assigned to it. "
		"No more memory can be allocated to the kernel."
	};
	panic(&info, NULL);

	// Keep the compiler happy.
	return 0;
}

uintptr_t find_available_contiguous_kernel_pages(uint32_t count)
{
	uintptr_t start_page_address = 0;
	uint32_t available_page_count = 0;

	for (
		uintptr_t address = first_available_kernel_address;
		address < 0xFFFFFFFF;
		address += page_size
	) {
		switch (is_page_allocated(address)) {
			case kPAGE_ALLOCATED: {
				start_page_address = 0;
				available_page_count = 0;
				break;
			}

			case kNO_PAGE_TABLE_ALLOCATED:
			case kNO_PAGE_ALLOCATED: {
				if (start_page_address == 0) 
					start_page_address = address;

				if (++available_page_count >= count)
					return start_page_address;
			}
		}
	}

	// At this point we can safely assume that there are not _enough_ available 
	// kernel space addresses left.
	struct panic_info info = (struct panic_info) {
		panic_memory,
		"INSUFFICIENT KERNEL ADDRESS SPACE",
		"The kernel has exhausted all of the address space assigned to it. "
		"No more memory can be allocated to the kernel."
	};
	panic(&info, NULL);

	// Keep the compiler happy.
	return 0;
}


////////////////////////////////////////////////////////////////////////////////

void kpage_table_alloc(uintptr_t address)
{
	// Make sure that there is no page table already allocated!
	if (is_page_allocated(address) != kNO_PAGE_TABLE_ALLOCATED)
		return;

	uint32_t page_table = page_table_for_address(address);

	struct virtual_address_space *address_space = kernel_address_space;

	// We need to take a chunk of the reserved space and claim it as the new
	// page table linear address.
	if (address_space->next_page_table > last_page_table_address) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"UNABLE TO ALLOCATE PAGE TABLE FOR VIRTUAL ADDRESS SPACE",
			"The specified address space has exhausted all memory reserved for "
			"page table allocation."
		};
		panic(&info, NULL);
	}

	uintptr_t pt_linear = address_space->next_page_table;
	address_space->next_page_table += page_size;

	if (is_page_allocated(pt_linear) == kNO_PAGE_TABLE_ALLOCATED) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"SYNCHRONISATION ISSUE IN VIRTUAL ADDRESS SPACE",
			"Unable to allocate a new page table due to bad management of the "
			"address space."
		};
		panic(&info, NULL);
	}

	// We also need to request a physical frame for the page table.
	uintptr_t pt_frame = kframe_alloc();

	// Setup the page table mapping so that we can find the association of the
	// linear address and frame in the future.
	address_space->page_table_address[page_table] = pt_linear;

	// Add add it to the appropriate page table.
	uint32_t pt_page_table = page_table_for_address(pt_linear);
	uint32_t pt_page = page_for_address(pt_linear);
	struct page_table *directory = (void *)(address_space->directory_address);
	struct page *table = (void *)(
		address_space->page_table_address[pt_page_table]
	);
	table[pt_page].frame = pt_frame >> 12;
	table[pt_page].present = 1;
	table[pt_page].readwrite = 1;

	// Invalidate the page so that we can write to it.
	__asm__ __volatile__("invlpg (%0)" :: "r"(pt_linear) : "memory");

	// Clear the page so that the MMU doesn't become corrupted with garbage data
	memset((void *)pt_linear, 0, page_size);

	// Install the page table and then flush the entire TLB.
	// fprintf(dbgout, "Installing page table %d with frame %p\n", 
	// 	page_table, pt_frame);
	directory[page_table].frame = pt_frame >> 12;
	directory[page_table].present = 1;
	directory[page_table].readwrite = 1;

	__asm__ __volatile__(
		"movl %%cr3, %%eax\n"
		"movl %%eax, %%cr3"
		::: "%eax"
	);
	// fprintf(dbgout, "Page table %d is now installed!\n", page_table);
}

int kpage_alloc(uintptr_t address)
{
	// Check to see if the page is already allocated in some capacity
	int result = is_page_allocated(address);
	if (result == kPAGE_ALLOCATED)
		return kPAGE_ALLOC_ERROR;

	// Check to see if we need to allocate the appropriate page table for the
	// address.
	if (result == kNO_PAGE_TABLE_ALLOCATED)
		kpage_table_alloc(address);

	// At this point we can safely allocate a frame to the page. 
	uintptr_t frame_address = kframe_alloc();

	uint32_t page_table = page_table_for_address(address);
	uint32_t page = page_for_address(address);

	struct virtual_address_space *address_space = kernel_address_space;
	struct page *table = (void *)(
		address_space->page_table_address[page_table]
	);
	table[page].frame = frame_address >> 12;
	table[page].present = 1;
	table[page].readwrite = 1;

	// Finally invalidate the page in the TLB.
	__asm__ __volatile__("invlpg (%0)" :: "r"(address) : "memory");

	return kPAGE_ALLOC_OK;
}

void kpage_free(uintptr_t address)
{
	fprintf(COM1, "Freeing page for linear address %p\n", address);
}


////////////////////////////////////////////////////////////////////////////////

void validate_kernel_address_space(struct boot_config *config)
{
	// Ensure that the current page directory is what the boot loader reported.
	if (REGISTER(cr3) != (uint32_t)(config->root_page_directory)) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"INVALID KERNEL ADDRESS SPACE",
			"The bootloader has incorrectly setup the paging environment. "
			"Unable to continue."
		};
		panic(&info, NULL);
	}
}

void prepare_kernel_address_space(void)
{
	// Setup the virtual address space structure for the kernel and get all
	// required information populated.
	// All of this information _should_ be identity mapped, and should be 
	// assumed to be so.
	fprintf(COM1, "Preparing kernel address space\n");

	kernel_address_space = (void *)reserve_kernel_working_memory(
		sizeof(*kernel_address_space)
	);
	memset(kernel_address_space, 0, sizeof(*kernel_address_space));

	kernel_address_space->directory_frame = REGISTER(cr3) & ~0xFFF;
	kernel_address_space->directory_address = REGISTER(cr3) & ~0xFFF;

	struct page_table *directory = (void *)(
		kernel_address_space->directory_address
	);
	for (uint32_t n = 0; n < PAGE_MAX_ENTRIES; ++n) {
		if (!directory[n].present)
			continue;

		kernel_address_space->page_table_address[n] = directory[n].frame << 12;
		fprintf(COM1, "  Page table %d has an address of %p\n", 
			n, kernel_address_space->page_table_address[n]);
	}

	// Determine what the first page-aligned address is after the kernel. This
	// is where we'll allocate pages for the page tables.
	first_available_kernel_address = first_kernel_address = (
		(kernel_end_address() + page_size) & ~(page_size - 1)
	);
	fprintf(COM1, "First available kernel space address is %p\n", 
		first_available_kernel_address);

	// The next job is to reserve enough space for the page tables to be 
	// allocated. This is a range of memory that will be used by each address
	// space to house their own page tables. Shared memory will be mapped 
	// between each.
	first_page_table_address = first_available_kernel_address;
	kernel_address_space->next_page_table = first_page_table_address;
	first_available_kernel_address += page_size * total_page_tables;
	last_page_table_address = first_available_kernel_address - page_size;
	fprintf(COM1, "Reserved 4MiB for virtual address space page tables\n");
	fprintf(COM1, "Revised first available kernel space address is %p\n", 
		first_available_kernel_address);

	// Check to see if we need to allocate any page tables for reserved memory.
	for (uint32_t n = 0; n < total_page_tables; ++n) {
		uintptr_t address = (
			(n * page_size) + kernel_address_space->next_page_table
		);
		if (is_page_allocated(address) == kNO_PAGE_TABLE_ALLOCATED) {
			kpage_table_alloc(address);
		}
	}

	fprintf(COM1, "Kernel virtual address space is now ready for use.\n");
}


////////////////////////////////////////////////////////////////////////////////

void virtual_memory_prepare(struct boot_config *config)
{
	validate_kernel_address_space(config);
	prepare_kernel_address_space();
}
