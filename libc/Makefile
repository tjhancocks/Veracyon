# Copyright (c) 2017-2018 Tom Hancocks
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

LIBC-C-SRCS := $(shell find $(CURDIR) -type f \( -name "*.c" \))
LIBC-C-OBJS-TMP := $(addsuffix .libc.o, $(basename $(LIBC-C-SRCS)))
LIBC-C-OBJS := $(addprefix ../build, $(LIBC-C-OBJS-TMP:$(CURDIR)%=%))
LIBC-S-SRCS := $(shell find $(CURDIR) -type f \( -name "*.s" \))
LIBC-S-OBJS-TMP := $(addsuffix .libc.s.o, $(basename $(LIBC-S-SRCS)))
LIBC-S-OBJS := $(addprefix ../build, $(LIBC-S-OBJS-TMP:$(CURDIR)%=%))

LIBK-C-SRCS := $(shell find $(CURDIR) -type f \( -name "*.c" \))
LIBK-C-OBJS-TMP := $(addsuffix .libk.o, $(basename $(LIBK-C-SRCS)))
LIBK-C-OBJS := $(addprefix ../build/libk, $(LIBK-C-OBJS-TMP:$(CURDIR)%=%))
LIBK-S-SRCS := $(shell find $(CURDIR) -type f \( -name "*.s" \))
LIBK-S-OBJS-TMP := $(addsuffix .libk.s.o, $(basename $(LIBK-S-SRCS)))
LIBK-S-OBJS := $(addprefix ../build/libk, $(LIBK-S-OBJS-TMP:$(CURDIR)%=%))

# Bad form... declaring environment tools here...
CC := i686-elf-gcc
AR := i686-elf-ar
AS := nasm

# Flags
CCFLAGS :=  -I./include -I../kernel/include\
			-ffreestanding -Wall -Wextra\
            -nostdlib -nostdinc -fno-builtin -fno-stack-protector\
            -nostartfiles -nodefaultlibs -m32 -finline-functions -std=c99\
            -O0 -fstrength-reduce -fomit-frame-pointer

CCFLAGS-LIBC := $(CCFLAGS) -D__libc__
CCFLAGS-LIBK := $(CCFLAGS) -D__libk__

# libk
.PHONY: libk
libk: $(LIBK-C-OBJS) $(LIBK-S-OBJS)
	$(AR) rcs ../build/libk.a $(LIBK-C-OBJS) $(LIBK-S-OBJS)

# Helpers
../build/libk/%.libk.s.o: %.s
	-mkdir -p $(dir $@)
	$(AS) -felf -o $@ $<

../build/libk/%.libk.o: %.c
	-mkdir -p $(dir $@)
	$(CC) $(CCFLAGS-LIBK) -o $@ -c $<