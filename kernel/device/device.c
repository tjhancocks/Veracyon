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

#include <device/device.h>
#include <stdlib.h>
#include <atomic.h>

////////////////////////////////////////////////////////////////////////////////

device_t COM1 = 0;
device_t KBD = 0;
device_t VT100 = 0;

static uint32_t next_device_id = 0;

////////////////////////////////////////////////////////////////////////////////

void device_bind(device_t *handle, void *device)
{
	if (!handle)
		return;
	*(handle) = device;
}

uint32_t device_next_id(void)
{
	return ++next_device_id;
}

////////////////////////////////////////////////////////////////////////////////

int dv_write(struct device *dev, const char *restrict str)
{
	// Make sure we have a device...
	if (!dev && dev->dev_id > 0) {
		return DEV_INVALID;
	}

	//...and that we can actually write to it.
	if (!((dev->opts & DP_WRITE) && dev->write_byte)) {
		return DEV_NOWRITE;
	}

	// Does the device require atomic writes? If so, then ensure we're in an
	// atomic context.
	atom_t atomic_write;
	if (dev->opts & DP_ATOMIC_WRITE)
		atomic_start(atomic_write);

	// Begin batching certain operations on the device.
	if (dev->batch_commit)
		dev->start_batch(dev);

	// We can now start to write to the device. We should write character by
	// character and make sure the device is ready for more characters before
	// writing.
	while (str && *str) {
		while (dev->can_write && !dev->can_write(dev));
		dev->write_byte(dev, *str++);
	}

	// Some devices may require a terminating NULL character to be written to
	// device. Check and write if necessary.
	if (dev->opts & DP_WRITE_TERMINATING_NULL) {
		while (dev->can_write && !dev->can_write(dev));
		dev->write_byte(dev, '\0');
	}

	// Commit the batched operations on the device.
	if (dev->batch_commit)
		dev->batch_commit(dev);

	if (dev->opts & DP_ATOMIC_WRITE || atomic_write)
		atomic_end(atomic_write);

	return DEV_OK;
}
