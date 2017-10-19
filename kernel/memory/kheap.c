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
// static struct kheap_block *kheap_first_block = NULL;
static struct kheap_block *kheap_last_block = NULL;


////////////////////////////////////////////////////////////////////////////////

void kheap_expand_by_single_page()
{
	kprint("Expanding kernel heap by a single page.\n");

	// Make sure we have the first page allocated.
	uintptr_t address = (kheap_end == kheap_start) ? kheap_end : kheap_end + 1;
	if (kpage_alloc(address) == kPAGE_ALLOC_ERROR) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"UNABLE TO EXPAND KERNEL HEAP",
			"The kernel heap could not be expanded correctly."
		};
		panic(&info);
	}

	// We can get the ending address by asking for the next available address.
	kheap_end = first_available_kernel_page() - 1;
	kprint("Kernel heap now finishes at %p\n", kheap_end);

	// Ensure that the page is marked as "free".
	struct kheap_block *block = (struct kheap_block *)address;
	block->magic = kHEAP_AVAIL_MAGIC;
	block->prev = kheap_last_block;
	block->next = NULL;
	block->size = 0x1000 - sizeof(*block);

	block->prev->next = block;

	// The last block is now this new one.
	kheap_last_block = block;
}

void kheap_coalesce_free_blocks()
{
	kprint("Searching for contiguous free blocks in the kernel heap...\n");

	// We need to start with the second to last block (it it exists) as nothing
	// can be merged into the last block.
	uint32_t merge_count = 0;
	struct kheap_block *block = kheap_last_block->prev;
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

		// Increment the merge counter.
		++merge_count;

	COALESCE_NEXT_BLOCK:
		block = block->prev;
	}

	kprint("Kernel heap coalesce completed with %d merge(s)\n", merge_count);
}

void kheap_describe()
{
	kprint("KERNEL HEAP:\n");
	struct kheap_block *block = (struct kheap_block *)kheap_start;
	while (block) {
		kprint("  %p:: %08x | %p | %p | %d bytes\n",
			block,
			block->magic,
			block->prev,
			block->next,
			block->size);
		block = block->next;
	}
}


////////////////////////////////////////////////////////////////////////////////

void kheap_prepare()
{
	kprint("Preparing kernel heap.\n");

	// Get the starting point of the kernel heap.
	kheap_start = kheap_end = first_available_kernel_page();
	kprint("Kernel heap starting at %p\n", kheap_start);
	kprint("Kernel heap finishes at %p\n", kheap_end);
}
