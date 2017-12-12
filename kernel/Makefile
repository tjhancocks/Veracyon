# Copyright (c) 2017 Tom Hancocks
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

C-KERNEL-SRCS := $(shell find $(CURDIR) -type f \( -name "*.c" \))
C-KERNEL-OBJS-TMP := $(addsuffix .o, $(basename $(C-KERNEL-SRCS)))
C-KERNEL-OBJS := $(addprefix ../build, $(C-KERNEL-OBJS-TMP:$(CURDIR)%=%))
S-KERNEL-SRCS := $(shell find $(CURDIR) -type f \( -name "*.s" \))
S-KERNEL-OBJS-TMP := $(addsuffix .s.o, $(basename $(S-KERNEL-SRCS)))
S-KERNEL-OBJS := $(addprefix ../build, $(S-KERNEL-OBJS-TMP:$(CURDIR)%=%))

# Bad form... declaring environment tools here...
CC := i686-elf-gcc
LD := i686-elf-gcc
AS := nasm

# Flags
CCFLAGS := -I./include -I../boot/includes -ffreestanding -Wall -Wextra\
            -nostdlib -nostdinc -fno-builtin -fno-stack-protector\
            -nostartfiles -nodefaultlibs -m32 -finline-functions -std=c99\
            -O0 -fstrength-reduce -fomit-frame-pointer

LDFLAGS := -ffreestanding -O0 -nostdlib -lgcc

LIBGCC := $(shell i586-elf-gcc -print-file-name=libgcc.a)

# vkernel-elf
.PHONY: vkernel-elf
vkernel-elf: $(C-KERNEL-OBJS) $(S-KERNEL-OBJS) $(LIBGCC)
	echo $(C-KERNEL-OBJS-TMP)
	$(LD) -T./kernel.ld $(LDFLAGS) -o ../build/vkernel $^

# Helpers
../build/%.s.o: %.s
	-mkdir -p $(dir $@)
	$(AS) -felf -o $@ $<

../build/%.o: %.c
	-mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ -c $<