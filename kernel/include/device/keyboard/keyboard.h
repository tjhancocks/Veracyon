/*
 Copyright (c) 2017 Tom Hancocks
 
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

#ifndef __VKERNEL_KEYBOARD__
#define __VKERNEL_KEYBOARD__

#include <stdint.h>
#include <device/keyboard/scancode.h>

/**
 Initialise all required concrete keyboard drivers (PS/2 and USB). These drivers 
 are then responsible for establishing communication back to the virtual 
 keyboard driver.
 */
void keyboard_driver_prepare(void);

/**
 Inform the virtual keyboard driver that a scancode has been received. The 
 scancode is assumed to be part of "Scancode Set 1". The virtual keyboard driver
 will then manage the construction of key events and buffering them.

 	- scancode: The scancode that the virtual keyboard driver should process.
 */
void keyboard_received_scancode(uint8_t scancode);

/**
 Receive the next keyevent from virtual keyboard driver. This is a blocking
 function.

 Returns:
 	A keyboard event structure with keycode information, modifier key states,
 	etc.
 */
struct keyevent *keyboard_wait_for_keyevent(void);

/**
 Query the keyboard buffer for unread queued key events.
 */
uint32_t keyboard_buffer_has_items(void);

#endif