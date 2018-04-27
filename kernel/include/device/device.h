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

#ifndef __VKERNEL_DEVICE__
#define __VKERNEL_DEVICE__

#include <stdint.h>

typedef void *device_t;

////////////////////////////////////////////////////////////////////////////////
// The following a collection of commonly used devices throughout the kernel.
// If they are available, then they will be configured to represent the 
// appropriate device.

extern device_t _COM1;	// Output only
extern device_t _KBD; 	// Input only
extern device_t _VT100;	// Output only

enum {
	__COM1_ID = 1,
	__KBD_ID = 2,
	__VT100_ID = 3,
};

/**
 Bind the provided device driver information to the specified device handle.
 */
void device_bind(device_t *handle, void *device);

/**
 Look up device based on id.
 */
device_t get_device(uint32_t dev_id);

////////////////////////////////////////////////////////////////////////////////

enum device_descriptor
{
	// The following are device types. These are typically 
	// components/peripherals
	device_type_serial = (1 << 0),
	device_type_keyboard = (1 << 1),
	device_type_terminal = (1 << 2),

	// The following are device protocols. This denotes the type of device
	// that has been connected.
	device_protocol_RS232 = (1 << 16),
	device_protocol_PS2 = (1 << 17),

	// The next group is a ready constructed list of devices recognised by the
	// kernel.
	device_COM1 = device_type_serial | device_protocol_RS232,
	device_PS2_keyboard = device_type_keyboard | device_protocol_PS2,
	device_VT100 = device_type_terminal
};

enum device_options
{
	DP_READ = 1 << 0,
	DP_WRITE = 1 << 1,
	DP_ATOMIC_READ = 1 << 2,
	DP_ATOMIC_WRITE = 1 << 3,
	DP_WRITE_TERMINATING_NULL = 1 << 4,
	DP_ALLOWS_ANSI = 1 << 5,
};

enum device_error
{
	DEV_OK = 1,
	DEV_ERROR = 0,
	DEV_INVALID = -1,
	DEV_NOREAD = -2,
	DEV_NOWRITE = -3,
};

struct device_pipe
{
	uint8_t *buffer;
	uint32_t length;
	uint32_t r_idx;
	uint32_t w_idx;
};

struct device
{
	// This field exists purely because certain device descriptors maybe created
	// prior to memory management existing. This will help the kernel to tell
	// which descriptors should be released and which shouldn't.
	char in_heap;
	uint32_t dev_id;
	const char *name;
	enum device_descriptor kind;
	enum device_options opts;

	// The device may wish to keep a reference to device specific information.
	void *info;

	// The buffers may contain a reference to data being written or read from
	// the device.
	// NOTE: The device is under no obligation to use these pipes. If the pipes
	// are unused, then the buffer should be left as NULL.
	struct device_pipe in;
	struct device_pipe out;

	// The next group of fields are a collection of optional function pointers,
	// that allow the kernel to interface with device.
	void(*write_byte)(struct device *, uint8_t);
	int(*can_write)(struct device *);
	uint8_t(*read_byte)(struct device *);
	int(*bytes_available)(struct device *);
	void(*start_batch)(struct device *);
	void(*batch_commit)(struct device *);
};

/**
 Returns the next available device id. This will also increment the next 
 available device id, so this should only be called when an actual device id
 is required.
 */
uint32_t device_next_id(void);

/**
 Write a string to the specified device. If the device can not be written to
 then a negative value will be returned.
 */
int dv_write(struct device *dev, const char *restrict str);

#endif