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

# FAT12
.PHONY: fat12
fat12: clean build/fat12.img
	@echo

.PHONY: fat12-bochs
fat12-bochs: clean fat12
	-mkdir -p debug
	bochs -q "boot:a" "floppya: 1_44=build/fat12.img, status=inserted" \
			 "debug: action=ignore, floppy=report" \
			 "magic_break: enabled=1" \
			 "com1: enabled=1, mode=file, dev=debug/bochs.log" \
			 "port_e9_hack: enabled=1" \
			 "clock: sync=realtime"


# FAT12 Boot Sector
build/fat12.img:
	-mkdir -p build
	make -C boot fat12-bootsector
	make -C boot core-loader
	imgtool -s support/imgtool/fat12.imgscript
