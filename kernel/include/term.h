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

#ifndef __VKERNEL_TERM__
#define __VKERNEL_TERM__

#include <kern_types.h>

struct term_interface {
	uint8_t use_ansi;
	void(*set_cursor)(uint32_t, uint32_t);
	void(*get_cursor)(uint32_t *, uint32_t *);
	void(*update_cursor)(void);
	void(*puts)(const char *restrict);
	void(*putc)(const char);
	void(*clear)(uint8_t);
	void(*set_attribute)(uint8_t);
	void(*get_attribute)(uint8_t *);
	void(*set_default_attribute)(uint8_t);
	void(*restore_default_attribute)(void);
};

/**
 The "krnout" or "kernel output" terminal handle refers to the terminal 
 responsible for handling what is shown on screen.
 */
extern const uint32_t krnout;

/**
 The "dbgout" or "debug output" terminal handle refers to the terminal
 responsible for handling what is sent via the COM1 Serial Port.
 */
extern const uint32_t dbgout;

/**
 The "allout" or "all outputs" terminal handle refers to all of the terminals
 registered in the kernel.
 */
extern const uint32_t allout;


/**
 Set whether the specified terminal should perform ANSI parsing and rendering
 before dispatching to the terminal driver.

 	- handle: A valid terminal handle.
 	- use_ansi: Any none 0 value will turn on ANSI processing for the terminal.

 WARNING:
 	Enabling ANSI processing for a terminal that does not support it may have
 	undesired effects.
 */
void term_use_ansi(uint32_t handle, uint8_t use_ansi);


/**
 Bind a "set_cursor" function to the specified terminal.
 The set_cursor function should allow the terminal to alter the position of its
 cursor.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_set_cursor(uint32_t handle, void(*fn)(uint32_t, uint32_t));

/**
 Bind a "get_cursor" function to the specified terminal.
 The get_cursor function should allow the terminal to retrieve the position of
 its cursor.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_get_cursor(uint32_t handle, void(*fn)(uint32_t *, uint32_t *));

/**
 Bind a "update_cursor" function to the specified terminal.
 The update_cursor function should allow the terminal to force the cursors 
 position to be flushed and updated immediately.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_update_cursor(uint32_t handle, void(*fn)());

/**
 Bind a "puts" function to the specified terminal.
 The puts function should allow a string to be written to the terminal at the
 current cursor position. Note that if ANSI processing is enabled then this
 function can be ignored and putc be used instead.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_puts(uint32_t handle, void(*fn)(const char *restrict));

/**
 Bind a "putc" function to the specified terminal.
 The putc function should allow a character to be written to the terminal at the
 current cursor position.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_putc(uint32_t handle, void(*fn)(const char));

/**
 Bind a "clear" function to the specified terminal.
 The clear function should allow the terminal to invoke a full clearing/erasing
 of the screen buffer.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_clear(uint32_t handle, void(*fn)(uint8_t));

/**
 Bind a "set_attribute" function to the specified terminal.
 The set_attribute function should allow the terminal to change the current
 colour attributes.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_set_attribute(uint32_t handle, void(*fn)(uint8_t));

/**
 Bind a "get_attribute" function to the specified terminal.
 The get_attribute function should allow the terminal to read the current colour
 attributes.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_get_attribute(uint32_t handle, void(*fn)(uint8_t *));

/**
 Bind a "set_default_attribute" function to the specified terminal.
 The set_default_attribute should allow the terminal to specify what the default
 colour should be.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_set_default_attribute(uint32_t handle, void(*fn)(uint8_t));

/**
 Bind a "restore_default_attribute" function to the specified terminal. 
 The restore_default_attribute should allow the terminal to restore the current
 attribute to the default one.

 	- handle: A valid terminal handle.
 	- fn: A function pointer with the correct prototype.
 */
void term_bind_restore_default_attribute(uint32_t handle, void(*fn)());


/**
 Set the position of the cursor in the specified terminal.

 	- handle: A valid terminal handle.
 	- x: The x position of the cursor
 	- y: The y position of the cursor
 */
void term_set_cursor(uint32_t handle, uint32_t x, uint32_t y);

/**
 Get the current position of the cursor from the specified terminal.

 	- handle: A valid terminal handle.
 	- x: A reference to a variable to store the x position of the cursor.
 	- y: A reference to a variable to store the y position of the cursor.
 */
void term_get_cursor(uint32_t handle, uint32_t *x, uint32_t *y);

/**
 Instruct the specified terminal to update the location of the cursor.

 	- handle: A valid terminal handle.
 */
void term_update_cursor(uint32_t handle);

/**
 Display the provided string in the specified terminal at the current cursor
 position.

 	- handle: A valid terminal handle.
 	- str: The string to write out to the terminal.
 */
void term_puts(uint32_t handle, const char *restrict str);

/**
 Display the provided character in the specified terminal at the current cursor
 position.

 	- handle: A valid terminal handle.
 	- c: The character to write out to the terminal.
 */
void term_putc(uint32_t handle, const char c);

/**
 Clear the contents of the specified terminal, changing the default attribute to
 the one specified.

 	- handle: A valid terminal handle.
 	- attribute: The new default attribute to use whilst clearing the terminal.
 */
void term_clear(uint32_t handle, uint8_t attribute);

/**
 Set the specified terminal cursor attribute to the provided value.

 	- handle: A valid terminal handle.
 	- attribute: The new cursor attribute value.
 */
void term_set_attribute(uint32_t handle, uint8_t attribute);

/**
 Get the current cursor attribute from the specified terminal.

 	- handle: A valid terminal handle.
 	- attribute: A reference to a variable for the attribute to be stored in.
 */
void term_get_attribute(uint32_t handle, uint8_t *attribute);

/**
 Set the default cursor attribute in the specified terminal.
	
 	- handle: A valid terminal handle.
 	- attribute: The new default attribute for the terminal.
 */
void term_set_default_attribute(uint32_t handle, uint8_t attribute);

/**
 Restore the current cursor attribute to the default attribute in the specified
 terminal.
 
 	- handle: A valid terminal handle.
 */
void term_restore_default_attribute(uint32_t handle);

#endif