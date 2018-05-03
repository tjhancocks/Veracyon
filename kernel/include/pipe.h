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

#ifndef __VKERNEL_PIPE__
#define __VKERNEL_PIPE__

#include <stdint.h>

struct process;

enum pipe_purpose
{
    // Pipe where owner receives a byte stream of data
    p_recv = (1 << 0),

    // Pipe where owner sends a byte stream of data
    p_send = (1 << 1),

    // Pipe is used to transmit keyboard scancodes.
    // Should only be used by the keyboard process to send scancodes to the
    // frontmost process.
    p_keyboard = (1 << 2),

    // Pipe is used to transmit error messages.
    p_err = (1 << 3),

    // Debugging messages should be sent via debug output.
    p_dbg = (1 << 4)
};

struct pipe 
{
    enum pipe_purpose purpose;
    struct process *owner;
    struct process *target;
    uintptr_t read_ptr;
    uintptr_t write_ptr;
    size_t size;
    uint8_t *data;
    const char *name;
};

void pipe_write(struct pipe *pipe, uint8_t *data, size_t count);
uint8_t *pipe_read(struct pipe *pipe, size_t *count, size_t limit);
size_t pipe_bytes_available(struct pipe *pipe);
uint8_t pipe_read_byte(struct pipe *pipe);

enum pipe_binding
{
    pipe_target_process = 1,
    pipe_source_process = 2,
    pipe_owner_process = 3,
};

/**
 Construct a new unbound pipe with the specified purpose.
 */
struct pipe *pipe(enum pipe_purpose mask);

/**
 Bind the specified data/object to the provided pipe. This binding might be
 the target process or source process.
 */
void pipe_bind(struct pipe *pipe, enum pipe_binding binding, const void *data);

/**
 Get the total number of pipes currently attached to the specified process.
 */
size_t pipe_count_for_process(struct process *process);

/**
 Retrieve an array of pipes matching the specified purpose for a given process.
 A count of those pipes is also provided.
 */
struct pipe **pipe_get_for_process(
    struct process *process, 
    enum pipe_purpose mask,
    size_t *count
);

/**
 Retrieve the current _best_ pipe for the specified purpose for a given process.
 */
struct pipe *pipe_get_best(struct process *process, enum pipe_purpose mask);

#endif