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

# Cleaning and House Keeping
.PHONY: clean
clean:
	-rm -rf build

################################################################################
# FAT12

.PHONY: fat12 fat12-text fat12-vesa
fat12: fat12-text
	@echo

fat12-text: build/fat12-text.img
	mv $^ build/fat12.img

fat12-vesa: build/fat12-vesa.img
	mv $^ build/fat12.img

################################################################################
# BOCHS

.PHONY: fat12-bochs fat12-text-bochs fat12-vesa-bochs bochs

fat12-text-bochs: clean fat12-text fat12-bochs
	@echo

fat12-vesa-bochs: clean fat12-vesa fat12-bochs
	@echo

fat12-bochs: build/fat12.img
	-mkdir -p debug
	bochs -q "boot:a" "floppya: 1_44=build/fat12.img, status=inserted" \
			 "debug: action=ignore, floppy=report" \
			 "magic_break: enabled=1" \
			 "com1: enabled=1, mode=file, dev=debug/bochs.log" \
			 "port_e9_hack: enabled=1" \
			 "memory: guest=512, host=256" \
			 "cpuid: model=pentium_mmx, level=6, mmx=1" \
			 "clock: sync=realtime"

################################################################################
# IMAGES

build/fat12-vesa.img:
	-mkdir -p build
	make -C boot fat12-bootsector
	make -C boot core-loader
	make -C kernel vkernel-elf
	imgtool -s support/imgtool/fat12.vesa.imgscript

build/fat12-text.img:
	-mkdir -p build
	make -C boot fat12-bootsector
	make -C boot core-loader
	make -C kernel vkernel-elf
	imgtool -s support/imgtool/fat12.text.imgscript
