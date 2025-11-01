#pragma once

#define DELETE_MOVE(type)                   \
	type(type&& other)			  = delete; \
	type& operator=(type&& other) = delete;

#define DELETE_COPY(type)                        \
	type(const type& other)			   = delete; \
	type& operator=(const type& other) = delete;

#define LOW_BYTE(x)	 ((u8) x) & 0b00001111
#define HIGH_BYTE(x) ((u8) x) << 4

#define STRINGIFY(x) #x
#define AS_STRING(x) STRINGIFY(x)

#define CARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
