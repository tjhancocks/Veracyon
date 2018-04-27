# Veracyon
The Veracyon project is a hobby operating system project, intended as a learning and research platform for myself. The project will ultimately include both a custom boot loader & kernel, and be developed in a mixture of Assembly and C. It will be specific
to the Intel 32-bit architecure and BIOS.

![Veracyon Basic Shell](https://raw.githubusercontent.com/tjhancocks/Veracyon/reorganise/docs/images/veracyon-shell-basic.png)


### Building
Currently the build system for Veracyon is hard coded and not configurable. This will be changed at some point, though I don't really consider it a priority currently. In order to build everything you'll need the following:

- A freestanding GCC 6.4.0 or later toolchain.
- NASM
- imgtool (See my [imgtool repo](https://github.com/tjhancocks/imgtool))
- BOCHS


## License

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