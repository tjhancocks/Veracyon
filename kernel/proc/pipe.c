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

#ifndef KERNEL_PIPE_SIZE
#   define KERNEL_PIPE_SIZE     1024
#endif

struct pipe *construct_pipe_for_process(
    struct process *process, 
    enum pipe_purpose purpose
) {
    if (!process) {
        fprintf(COM1, "Attempted to construct a pipe with no parent process\n");
        return NULL;
    }

    struct pipe *pipe = calloc(1, sizeof(*pipe));
    pipe->purpose = purpose;

    switch (pipe->purpose) {
        case p_in:
            pipe->name = "stdin";
            pipe->consumer = process;
            break;
        case p_kbd:
            pipe->name = "kbdin";
            pipe->consumer = process;
            break;
        case p_out:
            pipe->name = "stdout";
            pipe->producer = process;
            break;
        case p_err:
            pipe->name = "stderr";
            pipe->producer = process;
            break;
        case p_dbg:
        default:
            pipe->name = "dbgcom1";
            pipe->producer = process;
            break;
    }

    pipe->size = KERNEL_PIPE_SIZE;
    pipe->data = calloc(KERNEL_PIPE_SIZE, sizeof(*pipe->data));
    pipe->read_ptr = pipe->write_ptr = 0;

    return pipe;
}

void pipe_write(struct pipe *pipe, uint8_t *data, size_t count)
{
    for (int i = 0; i < count; ++i) {
        pipe->data[pipe->write_ptr++ % pipe->size] = data[i];
    }
}

uint8_t *pipe_read(struct pipe *pipe, size_t *count, size_t limit)
{
    *count = pipe_bytes_available(pipe);
    size_t bytes_to_read = (limit < *count) ? limit : *count;

    if (*count == 0) {
        return NULL;
    }
    
    uint8_t *data = calloc(bytes_to_read, sizeof(*data));
    memcpy(data, pipe->data + (pipe->read_ptr % pipe->size), bytes_to_read);
    pipe->read_ptr += bytes_to_read;
    return data;
}

size_t pipe_bytes_available(struct pipe *pipe)
{
    return pipe->write_ptr - pipe->read_ptr;
}

uint8_t pipe_read_byte(struct pipe *pipe)
{
    if (pipe_bytes_available(pipe) <= 0) {
        return '\0';
    }
    size_t count = 0;
    uint8_t *bytes = pipe_read(pipe, &count, 1);
    uint8_t byte = bytes[0];
    free(bytes);
    return byte;
}