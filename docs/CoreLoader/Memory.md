# CoreLoader Specification
## Memory Layout 
### Introduction

CoreLoader is the boot loader for the Veracyon Operating System and project. It will be responsible for bringing the computer into a known, predictable state before launching and handing control over to the *Kernel*.

In order to best achieve this, CoreLoader will construct all of its data structures (GDT, IDT, Paging, Memory Map, etc) in known locations, and pass a data structure over to the Kernel that details each of these structures.

This document is intended to make suggestions on where these data structures should be located in memory, and what the Kernel expects to be given.

**NOTE**: This document is designed with a BIOS system in mind. There may be a revised/alternate version produced in the future that also considers a UEFI system.

### Memory Regions

It is *generally* standard for BIOS to load the bootsector to the memory address `0x07c0:0x0000` (Linear: `0x00007c00`), and begin executing from there. It will also only load 512 bytes from the boot disk.

This document assumes the above to be true. If this is not the case on a given system, then the contents of this document should be considered to be *undefined behaviour*.

	- 0x07c00 - 0x07dff		Bootsector
	- 0x08000 - 0x0ffff		CoreLoader Binary (32KiB)
	- 0x10000 - 0x107ff		Global Descriptor Table (2KiB)
	- 0x10800 - 0x10805		Global Descriptor Table Pointer (6B)
	- 0x11000 - 0x117ff		Interrupt Descriptor Table (2KiB)
	- 0x11800 - 0x11805		Interrupt Descriptor Table Pointer


