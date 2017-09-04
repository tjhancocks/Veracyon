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

	- 0x07C00 - 0x07DDD		Bootsector
	- 0x08000 - 0x0FDFF		CoreLoader Binary (31KiB)
	- 0x0FE00 - 0x0FFFF		Boot Configuration (512B)		
	- 0x10000 - 0x107FF		Global Descriptor Table (2KiB)
	- 0x10800 - 0x10805		Global Descriptor Table Pointer (6B)
	- 0x11000 - 0x117FF		Interrupt Descriptor Table (2KiB)
	- 0x11800 - 0x11805		Interrupt Descriptor Table Pointer
	- 0x11806 - 0x1180A     Panic function pointer `void panic(registers_t reg)`
	- 0x11900 - 0x11D00		Custom Interrupt Handlers (1KiB)
	- 0x12000 - 0x12FFF		Root Page Directory (4KiB)
	- 0x13000 - 0x13FFF		Lower 4MiB Identity Mapping Page Table (4KiB/4MiB)
	- 0x14000 - 0x14FFF		Kernel Address Mapping Page Table (4KiB/4MiB)
	- 0x15000 - 0x15FFF		Reserved
	- 0x16000 - 0x16FFF		Reserved
	- 0x17000 - 0x17FFF		VESA Linear Frame Buffer Page Table (4KiB/4MiB)
	- 0x18000 - 0x18FFF		Reserved
	- 0x19000 - 0x19FFF		Reserved
	- 0x1A000 - 0x1AFFF		Reserved

### Boot Configuration

The Boot Configuration contains information about how the system should be configured,
or has been configured. This intended as a private, internally store to the Boot Loader,
but maybe accessed by the Kernel. This is not recommended however! The following describes what this structure contains.

	- 0xFE00 [Byte]		0 - VGA Text Mode (80x25), 1 - VESA Linear Frame Buffer
	- 0xFE01 [STR:31]	Kernel Name
	- 0xFE20 [DWord]	Kernel Address
	- 0xFE24 [Word]		Resolution Width
	- 0xFE26 [Word]		Resolution Height
	- 0xFE28 [Byte]		Resolution Depth
	- 0xFE2A [DWord]	Linear Frame Buffer	

