# Veracyon

**IMPORTANT** This version of Veracyon is now archived and will be removed at somepoint. Please go to https://github.com/tjhancocks/VeracyonOS.git and https://github.com/tjhancocks/vkernel.git for the current project.

The Veracyon project is a hobby operating system project, intended as a learning and research platform for myself. The project will ultimately include both a custom boot loader & kernel, and be developed in a mixture of Assembly and C. It will be specific
to the Intel 32-bit architecure and BIOS.

![Veracyon Basic Shell](https://raw.githubusercontent.com/tjhancocks/Veracyon/reorganise/docs/images/veracyon-shell-basic.png)

### Boot Loader (CoreLoader)
The boot loader of Veracyon is called _CoreLoader_. It is/will be a well defined piece of software that gets the computer into a known and defined state that the Kernel can then take advantage of.

Preliminary specs for CoreLoader are available in the repo, and they will be fully fleshed out over time and made available in the wiki should you want to develop your own implementation of it.

CoreLoader is responsible for setting up the following things:

- Global Descriptor Table
- Interrupt Descriptor Table (all ISRs handled at a basic level)
- Programmable Interrupt Controller
- Programmable Interrupt Timer (appropriate IRQ handled as well)
- Paging (Kernel can be loaded directly into higher half)
- VESA display mode can be configured.
- Basic disk driver and file system (appropriate to the boot media)

Ultimately all of this functionality will be available to the kernel via the boot configuration structure passed to it. It will include function pointers, data structure pointers and information about the system. It is expected however that kernel will replace most of these with its own implementations however (i.e. Paging, Disk Drivers and File Systems).

### The Kernel (vkernel)
The kernel of Veracyon (vkernel) will be a hybrid architecture (monolithic/micro). It will split out some of its functionality into external drivers and daemons, whilst keeping other parts inside the kernel itself. The exact seperation of concerns has not yet been fully determined.

The kernel relies on the state left by CoreLoader for operation. It adopts the Global Descriptor Table and Interrupt Descriptor Table that were setup by CoreLoader and makes them first class entities. In fact it will even install various IRQs and interrupt handlers itself.

Other aspects such as Paging however, will be migrated by the Kernel. It will reconstruct paging structures and entities and bring its own Physical Memory Manager and Virtual Memory Manager. These concepts are too critical and central to the kernel to leave for the boot loader to manage.

### Building
Currently the build system for Veracyon is hard coded and not configurable. This will be changed at some point, though I don't really consider it a priority currently. In order to build everything you'll need the following:

- A freestanding GCC 4.9.0 or later toolchain.
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
