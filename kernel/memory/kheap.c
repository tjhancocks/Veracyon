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

#include <kheap.h>
#include <virtual.h>
#include <memory.h>
#include <kprint.h>
#include <panic.h>

static uintptr_t kheap_start = 0;
static uintptr_t kheap_end = 0;
static struct kheap_block *kheap_first_block = NULL;
static struct kheap_block *kheap_last_block = NULL;
static uint8_t should_coalesce = 0;


////////////////////////////////////////////////////////////////////////////////

void kheap_coalesce_free_blocks();

void kheap_expand_by_single_page()
{
	// kdprint(dbgout, "Expanding kernel heap by a single page.\n");

	// Make sure we have the first page allocated.
	uintptr_t address = (kheap_end == kheap_start) ? kheap_end : kheap_end + 1;
	if (kpage_alloc(address) == kPAGE_ALLOC_ERROR) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"UNABLE TO EXPAND KERNEL HEAP",
			"The kernel heap could not be expanded correctly."
		};
		panic(&info, NULL);
	}

	// We can get the ending address by asking for the next available address.
	kheap_end = first_available_kernel_page() - 1;
	// kdprint(dbgout, "Kernel heap now finishes at %p\n", kheap_end);

	// Ensure that the page is marked as "free".
	struct kheap_block *block = (struct kheap_block *)address;
	block->magic = kHEAP_AVAIL_MAGIC;
	block->prev = kheap_last_block;
	block->next = NULL;
	block->size = 0x1000 - sizeof(*block);

	block->prev->next = block;

	// The last block is now this new one. Also set the first block if
	// appropriate.
	kheap_last_block = block;
	kheap_first_block = kheap_first_block ?: block;
}

void kheap_expand_by_required_length(uint32_t length)
{
	uint32_t required_pages = (length / 0x1000) + 1;
	kdprint(dbgout, "Expanding kernel heap by %d page(s)\n", required_pages);

	// Allocate all the required pages.
	for (uint32_t n = 0; n < required_pages; ++n)
		kheap_expand_by_single_page();
	
	kheap_coalesce_free_blocks();
	kdprint(dbgout, "Kernel heap expanded.\n");
}

void kheap_divide_block(struct kheap_block *block, uint32_t size)
{
	// kdprint(dbgout, "Dividing kernel heap block into two smaller blocks\n");

	uint32_t required = (size + (sizeof(*block) * 2));
	if (block->size < required) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"HEAP BLOCK TOO SMALL",
			"Attempted to divide a kernel block heap that was too small to be "
			"divided."
		};
		panic(&info, NULL);
	}

	// Determine where the next block is actually going to start, and then
	// configure it.
	struct kheap_block *new_block = (void *)(
		(uintptr_t)block + size + sizeof(*block)
	);
	new_block->magic = kHEAP_AVAIL_MAGIC;
	new_block->next = block->next;
	block->next->prev = new_block;
	block->next = new_block;
	new_block->prev = block;
	new_block->size = block->size - (size + sizeof(*block));
	block->size = size;

	if (kheap_last_block == block)
		kheap_last_block = new_block;

	// kdprint(dbgout, 
	// 	"Kernel heap block (%p) has been divided into two blocks\n", 
	// 	block);
}

void kheap_allocate_block(struct kheap_block *block)
{
	// kdprint(dbgout, "Marking kernel heap block %p as allocated.\n", block);
	block->magic = kHEAP_ALLOC_MAGIC;
}

void kheap_coalesce_free_blocks()
{
	if (++should_coalesce < 5)
		return;
	should_coalesce = 0;


	// We need to start with the second to last block (it it exists) as nothing
	// can be merged into the last block.
	uint32_t merge_count = 0;
	struct kheap_block *block = kheap_last_block->prev;
	// kdprint(dbgout, "Starting at block: %p\n", block);
	// kdprint(dbgout, "  (%p --> %p)\n", kheap_first_block, kheap_last_block);
	while (block) {

		// Check conditions for the block being included in the merge.
		if (block->magic != kHEAP_AVAIL_MAGIC) 
			goto COALESCE_NEXT_BLOCK;
		else if (block->next->magic != kHEAP_AVAIL_MAGIC)
			goto COALESCE_NEXT_BLOCK;

		// Get the space between the two blocks.
		uintptr_t end_of_block = (
			(uintptr_t)block + block->size + sizeof(*block)
		);
		uint32_t distance = (uintptr_t)block->next - end_of_block;

		// If the distance is _anything_ other than zero, then the two blocks
		// are not contiguous.
		if (distance != 0)
			goto COALESCE_NEXT_BLOCK;

		// The two blocks are free, and next to each other! This is slightly
		// complex. If the "next" block is the last block, then the current
		// block _must_ replace it as last block. The sizes of the two blocks
		// must be combined, plus the size of the block header. The next pointer
		// of the current block must also be replaced with the next pointer
		// of the next block.
		// TODO: Explain the other in a more clear way.
		if (block->next == kheap_last_block)
			kheap_last_block = block;

		block->size += block->next->size + sizeof(*block);
		block->next = block->next->next;
		block->next->prev = block;

		// Increment the merge counter.
		++merge_count;

	COALESCE_NEXT_BLOCK:
		block = block->prev;
	}
}

void kheap_describe()
{
	kdprint(dbgout, "KERNEL HEAP:\n");
	struct kheap_block *block = (struct kheap_block *)kheap_start;
	while (block) {
		kdprint(dbgout, "  %p:: %08x | %p | %p | %d bytes\n",
			block,
			block->magic,
			block->prev,
			block->next,
			block->size);
		block = block->next;
	}
}


////////////////////////////////////////////////////////////////////////////////

struct kheap_block *kheap_find_block_of_size(uint32_t minimum)
{
	// Calculate how space is actually going to be needed for the block
	// entry.
	struct kheap_block *block = kheap_first_block;
	uint32_t required = (minimum + (sizeof(*block) * 2));
	while (block) {

		// Is the block available?
		if (block->magic != kHEAP_AVAIL_MAGIC)
			goto CHECK_SIZE_OF_NEXT_BLOCK;

		// Does the block have at least the minimum space mentioned plus the 
		// twice the size of the block header?
		if (block->size >= required) {
			goto ALLOCATE_OVERSIZED_BLOCK;
		}
		else if (block->size >= minimum && block->size < required) {
			kheap_allocate_block(block);
			return block;
		}
		else if (block->size < minimum) {
			goto CHECK_SIZE_OF_NEXT_BLOCK;
		}

		// We've found a suitable block. 
		return block;

	CHECK_SIZE_OF_NEXT_BLOCK:
		block = block->next;
	}

	// If we've reach this point then we've exhausted all available blocks in
	// the heap, and will need more!
	kheap_expand_by_required_length(required);

	// The last block in the heap is now large enough...
	block = kheap_last_block;

ALLOCATE_OVERSIZED_BLOCK:
	kheap_divide_block(block, minimum);
	kheap_allocate_block(block);

	// Return the final block to the caller.
	return block;
}

void *kalloc(uint32_t length)
{
	struct kheap_block *block = kheap_find_block_of_size(length);
	if (!block) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"UNABLE TO ALLOCATE MEMORY ON KERNEL HEAP",
			"Was unable to allocate memory on the kernel heap."
		};
		panic(&info, NULL);
	}

	uintptr_t memory_address = (uintptr_t)block + sizeof(*block);
	return (void *)memory_address;
}

void kfree(void *ptr)
{
	if (!ptr)
		return;

	// Check to ensure the ptr is for a valid block on the heap.
	struct kheap_block *block = (struct kheap_block *)(
		(uintptr_t)ptr - sizeof(*block)
	);
	if (block->magic != kHEAP_ALLOC_MAGIC) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"ATTEMPT TO FREE ILLEGAL KERNEL HEAP MEMORY",
			"The memory that was attempted to be free'd was not a valid "
			"allocation on the kernel heap."
		};
		panic(&info, NULL);
	}

	// Mark as free and then coalesce memory
	block->magic = kHEAP_AVAIL_MAGIC;
	kheap_coalesce_free_blocks();
}


////////////////////////////////////////////////////////////////////////////////

void kheap_prepare()
{
	kdprint(dbgout, "Preparing kernel heap.\n");

	// Get the starting point of the kernel heap.
	kheap_start = kheap_end = first_available_kernel_page();
	kdprint(dbgout, "Kernel heap starting at %p\n", kheap_start);
	kdprint(dbgout, "Kernel heap finishes at %p\n", kheap_end);
}
