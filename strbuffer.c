/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "strbuffer.h"
#include "ecalloc.h"

void strbuffer_reserve(strbuffer_t *sb, size_t size)
{
	if (!sb || size <= sb->capacity)
		return;

	char *new_buffer = ecalloc(size, sizeof(char));
	if (sb->buffer) {
		size_t i;
		for (i = 0; i < sb->size; ++i)
			new_buffer[i] = sb->buffer[i];
	}

	free(sb->buffer);
	sb->buffer = new_buffer;
	sb->capacity = size;
}

size_t strbuffer_insertn(strbuffer_t *sb, size_t index,
			 const char *str, size_t size)
{
	if (!sb || !str || size == 0 || index > sb->size)
		return 0;

	size_t capacity = sb->size + size;
	if (capacity > sb->capacity)
		strbuffer_reserve(sb, capacity << 1);

	memcpy(sb->buffer + index + size, sb->buffer + index,
		(sb->size - index) * sizeof(char));
	memcpy(sb->buffer + index, str, size * sizeof(char));
	sb->size += size;
	return size;
}

size_t strbuffer_insert_int(strbuffer_t *sb, size_t index,
			    unsigned long long num)
{
	if (!sb || index > sb->size)
		return 0;

	size_t size = 0;
	do {
		char c = '0' + (num % 10);
		strbuffer_insert_char(sb, index, c);
		num /= 10;
		++size;
	}
	while (num != 0);

	return size;
}

size_t strbuffer_appendf(strbuffer_t *sb, const char *format, ...)
{
	if (!sb || !format)
		return 0;

	va_list args;
	memset(args, 0, sizeof(va_list));
	va_start(args, format);

	size_t previous_size = sb->size;
	size_t chunk_size = 0;

	const char *current = format;
	while (*current) {
		if (*current == '%' && (current == format || *(current - 1) != '%')) {
			switch (*(current + 1)) {
			case 's': {
				strbuffer_appendn(sb, current - chunk_size, chunk_size);
				const char *str = va_arg(args, const char *);
				strbuffer_append(sb, str);
				break;
			}
			case 'c': {
				strbuffer_appendn(sb, current - chunk_size, chunk_size);
				char c = va_arg(args, int);
				strbuffer_append_char(sb, c);
				break;
			}
			case 'i':
			case 'n':
			case 'd': {
				strbuffer_appendn(sb, current - chunk_size, chunk_size);
				unsigned long long num = va_arg(args, unsigned long long);
				strbuffer_append_int(sb, num);
				break;
			}
			}
			chunk_size = 0;
			current += 2;
			continue;
		}
		++chunk_size;
		++current;
	}

	strbuffer_appendn(sb, current - chunk_size, chunk_size);
	va_end(args);

	return sb->size - previous_size;
}
