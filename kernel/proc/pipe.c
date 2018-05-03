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

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <pipe.h>
#include <process.h>

////////////////////////////////////////////////////////////////////////////////

#ifndef KERNEL_PIPE_SIZE
#   define KERNEL_PIPE_SIZE     1024
#endif

#ifndef KERNEL_MAX_PIPE_COUNT
#   define KERNEL_MAX_PIPE_COUNT     8 * 1024
#endif

////////////////////////////////////////////////////////////////////////////////

static struct pipe **_pipe_pool = NULL;

struct pipe *pipe(enum pipe_purpose mask)
{
    if (!_pipe_pool) {
        // Allocate a store for all the system pipes.
        _pipe_pool = calloc(KERNEL_MAX_PIPE_COUNT, sizeof(*_pipe_pool));
        fprintf(dbgout, "Allocation pipe storage pool for %d pipes (%p)\n",
            KERNEL_MAX_PIPE_COUNT, _pipe_pool);
    }

    struct pipe *new_pipe = calloc(1, sizeof(*new_pipe));
    new_pipe->size = KERNEL_PIPE_SIZE;
    new_pipe->data = calloc(new_pipe->size, sizeof(*new_pipe->data));
    new_pipe->purpose = mask;


    for (uint32_t i = 0; i < KERNEL_MAX_PIPE_COUNT; ++i) {
        if (!_pipe_pool[i]) {
            fprintf(dbgout, "Creating pipe #%d -- pipe(%02x)\n", i, mask);
            _pipe_pool[i] = new_pipe;
            goto PIPE_POOL_FOUND;
        }
    }

    // TODO: Handle the failure case for this.
    fprintf(dbgout, "Failed to create pipe -- pipe(%02x)\n", mask);
    free(new_pipe->data);
    free(new_pipe);

PIPE_POOL_FOUND:
    return new_pipe;
}

void pipe_bind(struct pipe *pipe, enum pipe_binding binding, const void *data)
{
    if (!pipe) {
        // TODO: Panic here...
        return;
    }

    switch (binding) {
        case pipe_target_process: {
            if (pipe->purpose & p_recv) {
                pipe->owner = (struct process *)data;
            }
            else if (pipe->purpose & p_send) {
                pipe->target = (struct process *)data;
            }
            break;
        }
        case pipe_source_process: {
            if (pipe->purpose & p_recv) {
                pipe->target = (struct process *)data;
            }
            else if (pipe->purpose & p_send) {
                pipe->owner = (struct process *)data;
            }
            break;
        }
        case pipe_owner_process: {
            pipe->owner = (struct process *)data;
            break;
        }
    }

    fprintf(dbgout, "Pipe <%p | %02x> %s %s %s\n",
        pipe, pipe->purpose, (pipe->owner ? pipe->owner->name : "???"),
        ((pipe->purpose & p_recv) ? "<--" : "-->"),
        (pipe->target ? pipe->target->name : "???"));
}

size_t pipe_count_for_process(struct process *proc)
{
    size_t count = 0;
    for (uint32_t i = 0; i < KERNEL_MAX_PIPE_COUNT; ++i) {
        if (!_pipe_pool[i]) continue;
        if (_pipe_pool[i]->owner == proc || _pipe_pool[i]->target == proc) {
            ++count;
        }
    }
    return count;
}

struct pipe **pipe_get_for_process(
    struct process *proc, 
    enum pipe_purpose mask,
    size_t *count
) {
    size_t total_pipes = 0;
    for (uint32_t i = 0; i < KERNEL_MAX_PIPE_COUNT; ++i) {
        if (!_pipe_pool[i]) continue;
        if (mask == 0 && proc == NULL) {
            ++total_pipes;
            continue;
        }
        if ((_pipe_pool[i]->purpose & mask) != mask) continue;
        if (_pipe_pool[i]->owner == proc || _pipe_pool[i]->target == proc) {
            ++total_pipes;
            continue;
        }
    }

    struct pipe **pipes = NULL;
    if (total_pipes) {
        pipes = calloc(total_pipes, sizeof(*pipes));
        uint32_t pipe_idx = 0;
        for (uint32_t i = 0; i < KERNEL_MAX_PIPE_COUNT; ++i) {
            if (!_pipe_pool[i]) continue;
            if (mask == 0 && proc == NULL) {
                pipes[pipe_idx++] = _pipe_pool[i];
               continue;
             }
            if ((_pipe_pool[i]->purpose & mask) != mask) continue;
            if (_pipe_pool[i]->owner == proc || _pipe_pool[i]->target == proc) {
                pipes[pipe_idx++] = _pipe_pool[i];
                
                continue;
            }
        }
    }
    
    if (count) {
        *count = total_pipes;
    }

    return pipes;
}

struct pipe *pipe_get_best(struct process *process, enum pipe_purpose mask)
{
    struct pipe *pipe = NULL;
    struct pipe **pipes = pipe_get_for_process(process, mask, NULL);
    pipe = pipes ? *pipes : NULL;
    free(pipes);
    return pipe;
}

bool pipe_has_unread(struct pipe *pipe, ssize_t *count)
{
    if (!pipe) return false;
    ssize_t diff = pipe->write_ptr - pipe->read_ptr;
    if (count) *count = diff;
    return (diff > 0) ? true : false;
}

uint8_t pipe_read_byte(struct pipe *pipe, bool *empty)
{
    if (!pipe_has_unread(pipe, NULL)) {
        if (empty) *empty = true;
        return '\0';
    }
    if (empty) *empty = false;
    return pipe->data[pipe->read_ptr++ % pipe->size];
}

uint8_t pipe_peek_byte(struct pipe *pipe, int32_t offset)
{
    return pipe->data[(pipe->read_ptr + offset) % pipe->size];
}

bool pipe_can_accept_write(struct pipe *pipe)
{
    ssize_t diff = 0;
    (void)pipe_has_unread(pipe, &diff);
    return (diff < pipe->size) ? true : false;
}

void pipe_write_byte(struct pipe *pipe, uint8_t byte)
{
    if (!pipe_can_accept_write(pipe)) {
        fprintf(dbgout, "Writing beyond pipe <%p> buffer!\n", pipe);
        pipe->read_ptr++;
    }
    pipe->data[pipe->write_ptr++ % pipe->size] = byte;
}

void pipe_write(struct pipe *pipe, uint8_t *bytes, size_t len)
{
    for (uint32_t i = 0; i < len; ++i) {
        while (!pipe_can_accept_write(pipe)) {
            __asm__ __volatile__("hlt");
        }
        pipe_write_byte(pipe, bytes[i]);
    }
}