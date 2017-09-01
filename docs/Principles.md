# Veracon
## Design & Development Principles
### Introduction

Veracyon is an Operating System project that is intended primarily to be a learning platform and exercise. First and foremost, it will be for the x86 architecture. It will begin life as a 32-bit platform (due primarily to the test hardware available to myself) but will eventually support 64-bit machines.

The project will encompass the following fundamental components:

- Bootloader (CoreLoader)
- Kernel (vkernel)
- File System (LFS/Lunar File System)
- Assembler (vasm)
- Compiler (vcc)

### CoreLoader

The Bootloader (aka CoreLoader), is designed specifically to work with the Veracyon Kernel, but the specification will be produced in such a way that differing implementations of either should be compatible.

The fundamental tenant of CoreLoader is that it should get the system into a known state, constructing a _System Configuration_ object that can be passed to the Kernel detailing information about the hardware, and CPU setup. The reason for this is so that the Kernel does not need to concern itself with the gritty details of the CPU setup.

This will include elements such as the GDT, IDT, Memory Detection, VESA initialisation, Paging setup, etc. However, should the Kernel want/need to, in can override these with its own versions.

For no reason other than a desire to really practice and get a much better and deeper understanding of assemblers and machine language, this projects CoreLoader implementation will be done in assembly.

### vkernel
