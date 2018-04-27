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
#include <stdio.h>
#include <panic.h>
#include <sema.h>

#define kPAGE_SIZE	0x1000

#define kHEAP_COLLECT_FAIL	0
#define kHEAP_COLLECT_OK 	1

#define kHEAP_SPLIT_FAIL	0
#define kHEAP_SPLIT_OK 		1

////////////////////////////////////////////////////////////////////////////////

static struct kheap_block *kheap_expand(size_t size);
static struct kheap_block *kheap_expand_pages(uint32_t pages);
static struct kheap_block *kheap_make_block(
	uintptr_t raw_address, 
	size_t raw_size
);
static void kheap_describe_block(struct kheap_block *block);
static void kheap_describe_structure(void);
static int kheap_collect_block(
	struct kheap_block *dst, 
	struct kheap_block *src
);
static int kheap_split_block(
	struct kheap_block *block, 
	size_t real_size
);
static struct kheap_block *kheap_allocate_block(size_t size);

////////////////////////////////////////////////////////////////////////////////

static struct kheap_block *heap_first = NULL;
static struct kheap_block *heap_last = NULL;
static uint32_t heap_page_count = 0;

////////////////////////////////////////////////////////////////////////////////

void kheap_dump_structure(void)
{
	kheap_describe_structure();
}

////////////////////////////////////////////////////////////////////////////////

void *kalloc(size_t size)
{
	struct kheap_block *block = kheap_allocate_block(size);
	if (!block) {
		struct panic_info info = (struct panic_info) {
			panic_memory,
			"UNABLE TO ALLOCATE MEMORY",
			"The kernel was unable to allocate the requested amount of memory "
			"on the kernel heap.\n"
		};
		panic(&info, NULL);
	}

	// Calculate the absolute start of the allocated memory.
	uintptr_t address = (uintptr_t)block + sizeof(*block);
	// fprintf(COM1, "Allocated memory (%d bytes) at %p\n", size, address);

	return (void *)address;
}

void kfree(void *ptr)
{
	// fprintf(COM1, "Attempting to free memory at pointer: %p\n", ptr);

	// Make sure we're attempting to free a valid memory pointer. Warn if we're
	// not.
	uintptr_t address = (uintptr_t)ptr - sizeof(struct kheap_block);
	struct kheap_block *block = (void *)address;

	if (block->magic != kHEAP_ALLOC_MAGIC) {
		fprintf(COM1, "WARNING: Attempting to free an invalid pointer.\n");
		return;
	}

	// Mark the region as free, and then attempt to collect it with the previous
	// region and next region.
	block->magic = kHEAP_AVAIL_MAGIC;
	void *prev = block->prev;
	void *next = block->next;

	kheap_collect_block(prev, block);
	kheap_collect_block(prev ?: block, next);
}

////////////////////////////////////////////////////////////////////////////////

static struct kheap_block *kheap_expand(size_t size)
{
	uint32_t pages = ((size + kPAGE_SIZE) & ~(kPAGE_SIZE - 1)) >> 12;
	return kheap_expand_pages(pages);
}

static struct kheap_block *kheap_expand_pages(uint32_t pages)
{
	// fprintf(COM1, "Expanding kernel heap by %d page(s).\n", pages);

	uintptr_t first_page = find_available_contiguous_kernel_pages(pages);
	// fprintf(COM1, "	* Starting at page address %p\n", first_page);

	// Prepare to allocate each of the pages.
	for (uint32_t n = 0; n < pages; ++n) {
		uintptr_t address = first_page + (kPAGE_SIZE * n);
		// fprintf(COM1, "      * allocating %p\n", address);

		if (kpage_alloc(address) == kPAGE_ALLOC_ERROR) {
			struct panic_info info = (struct panic_info) {
				panic_memory,
				"UNABLE TO EXPAND KERNEL HEAP",
				"The kernel heap could not be expanded correctly."
			};
			panic(&info, NULL);
		}
	}

	// Configure the newly allocated pages correctly.
	// fprintf(COM1, "   * Configuring newly allocated pages for heap use.\n");

	uintptr_t raw_address = first_page;
	size_t raw_size = pages * kPAGE_SIZE;
	// fprintf(COM1, "      Raw address: %p, Raw size: %d bytes\n",
	// 	raw_address, raw_size);

	struct kheap_block *block = kheap_make_block(raw_address, raw_size);

	// Add the newly constructed block into the heap chain. If this is the first
	// block, then mark it as such.
	if (!heap_first) {
		// fprintf(COM1, "   * No first block in kernel heap. Setting it.\n");
		heap_first = heap_last = block;
	}

	// Attempt to collect the block into the current last block. If the 
	// operation is successful, the report it and finish.
	else if (kheap_collect_block(heap_last, block) == kHEAP_COLLECT_OK) {
		// fprintf(COM1, "   * Collecting block into previous last block.\n");
		block = heap_last;
	}

	// If we hit this point, then we have a new last block in the heap chain.
	// Update accordingly.
	else {
		// The previous block link should be to the current last block in the
		// chain, and the next should be NULL.
		block->prev = heap_last;
		block->next = NULL;

		// If there is a current last block in the chain, then update its next
		// link to this new block.
		if (heap_last)
			heap_last->next = block;

		// Mark the current last block as the new block.
		heap_last = block;
	}
	
	// Return the block to the caller.
	// fprintf(COM1, "***\n");
	return block;
}

////////////////////////////////////////////////////////////////////////////////

static void kheap_describe_block(struct kheap_block *block)
{
	printf("HEAP-BLOCK:: %08X | %p <- %p -> %p | %d bytes\n",
		block->magic, block->prev, block, block->next, block->size);
}

static void kheap_describe_structure(void)
{
	fprintf(COM1, "===============\n");
	struct kheap_block *block = heap_first;
	while (block) {
		kheap_describe_block(block);
		block = block->next;
	}
	fprintf(COM1, "===============\n");
}

////////////////////////////////////////////////////////////////////////////////

static struct kheap_block *kheap_make_block(
	uintptr_t raw_address, 
	size_t raw_size
) {
	struct kheap_block *block = (void *)raw_address;
	size_t block_header_size = sizeof(*block);
	size_t block_real_size = raw_size - block_header_size;

	// fprintf(COM1, "Making kernel heap block [header-size: %d bytes]\n",
	// 	block_header_size);
	// fprintf(COM1, "   * Raw size: %d bytes, Real size: %d\n",
	// 	raw_size, block_real_size);

	block->magic = kHEAP_AVAIL_MAGIC;
	block->prev = NULL;
	block->next = NULL;
	block->size = block_real_size;

	// fprintf(COM1, "   * Block constructed\n");

	return block;
}

////////////////////////////////////////////////////////////////////////////////

static int kheap_collect_block(
	struct kheap_block *dst, 
	struct kheap_block *src
) {
	// We absolutely must have both the destination and source blocks in order
	// to attempt collection.
	if (!dst || !src)
		return kHEAP_COLLECT_FAIL;

	// Both block must be marked as "free".
	else if (dst->magic != kHEAP_AVAIL_MAGIC || src->magic != kHEAP_AVAIL_MAGIC)
		return kHEAP_COLLECT_FAIL;

	// Ensure the destination and source blocks are in the correct order. We
	// can just fail if they are not.
	else if (dst >= src)
		return kHEAP_COLLECT_FAIL;

	// We can now calculate the distance between the end of the destination 
	// block and the start of the source block. If they are physcially adjacent
	// in memory, then we can merge them together.
	uintptr_t end_of_dst = (uintptr_t)dst + dst->size + sizeof(*dst);
	uintptr_t start_of_src = (uintptr_t)src;
	int32_t distance = start_of_src - end_of_dst;

	if (distance != 0)
		return kHEAP_COLLECT_FAIL;

	// At this point we know it is possible to collect the blocks. Clear the
	// meta data of the source block, and increase the allocation size of the
	// destination block.
	dst->size += src->size + sizeof(*src);

	dst->next = src->next;
	if (src->next)
		src->next->prev = dst;
	if (src == heap_last)
		heap_last = dst;

	memset(src, 0, sizeof(*src));

	return kHEAP_COLLECT_OK;
}

static int kheap_split_block(
	struct kheap_block *block, 
	size_t real_size
) {
	// Perform checks to ensure that the specified block is real, free and large
	// enough to be split. The size of the block must be at least real_size +
	// twice the block_header_size.
	if (!block)
		return kHEAP_SPLIT_FAIL;
	else if (block->magic != kHEAP_AVAIL_MAGIC)
		return kHEAP_SPLIT_FAIL;
	else if (block->size < (sizeof(*block) * 2) + real_size)
		return kHEAP_SPLIT_FAIL;

	// Splitting the block is relatively straight forwards. The existing block
	// needs to be shrunk to the specified real size, and then immediately
	// following it a new block header constructed. This new block will occupy
	// the remaining space of the old block.
	uintptr_t new_block_raw_address = (uintptr_t)block + real_size 
									+ sizeof(*block);
	size_t new_block_raw_size = block->size - real_size;
	
	struct kheap_block *new_block = kheap_make_block(
		new_block_raw_address, 
		new_block_raw_size
	);

	block->size = real_size;

	// Updating the previous heap chain is the most tricky aspect. This needs
	// to insert a new block between the old one and the current "next" block.
	struct kheap_block *next_block = block->next;

	new_block->prev = block;
	new_block->next = next_block;
	block->next = new_block;

	if (next_block)
		next_block->prev = new_block;

	// The old block was the previous last block in the chain, then update the
	// last block reference to be the new last block.
	if (block == heap_last)
		heap_last = new_block;

	return kHEAP_SPLIT_OK;
}

////////////////////////////////////////////////////////////////////////////////

static struct kheap_block *kheap_allocate_block(size_t size)
{
	size_t required_split_size = size + (sizeof(struct kheap_block) * 2);
	// fprintf(COM1, "Finding heap block for allocation of %d bytes.\n", size);

	// The first step is to search all current blocks for the first available
	// block that is large enough to accomodate the requested allocation, and
	// is not already in use.
	struct kheap_block *block = heap_first;
	while (block) {
		if (block->magic == kHEAP_AVAIL_MAGIC && block->size >= size)
			goto BLOCK_FOUND;
		block = block->next;
	}

	// Failed to find a suitable block in the heap. Attempt to expand the heap
	// in order to get one.
	// fprintf(COM1, "Failed to find suitable heap block. Expanding heap.\n");
	block = kheap_expand(size + sizeof(*block));

	// Perform a sanity check to ensure the reported block is actually suitable.
	if (block->magic != kHEAP_AVAIL_MAGIC || block->size < size) {
		// fprintf(COM1, "   * Serious error. Failed to make suitable block!\n");
		return NULL;
	}

BLOCK_FOUND:
	// If the block is far larger than needed then we need to split it, so that
	// we do not waste too much space.
	if (block->size > required_split_size) {
		// fprintf(COM1, "   * Block too large (%d>%d). Attempting to split.\n",
		// 	block->size, required_split_size);
		if (kheap_split_block(block, size) != kHEAP_SPLIT_OK) {
			// fprintf(COM1, "   * Failed to split heap block. Leaving.\n");
		}
	}

	// The block reference is now of the requested size and needs to be marked
	// as allocated and returned the caller.
	block->magic = kHEAP_ALLOC_MAGIC;
	// fprintf(COM1, "***\n");

	return block;
}
