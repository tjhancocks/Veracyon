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
    p_in,
    p_out,
    p_err,
    p_kbd,
    p_dbg
};

struct pipe 
{
    enum pipe_purpose purpose;
    struct process *producer;
    struct process *consumer;
    uintptr_t read_ptr;
    uintptr_t write_ptr;
    size_t size;
    uint8_t *data;
    const char *name;
};

struct pipe *construct_pipe_for_process(
    struct process *process, 
    enum pipe_purpose purpose
);

void pipe_write(struct pipe *pipe, uint8_t *data, size_t count);
uint8_t *pipe_read(struct pipe *pipe, size_t *count, size_t limit);
size_t pipe_bytes_available(struct pipe *pipe);
uint8_t pipe_read_byte(struct pipe *pipe);

#endif