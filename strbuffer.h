/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef STRBUFFER_H
#define STRBUFFER_H

#include <stdlib.h>
#include <string.h>

#include "ealloc.h"

struct strbuffer {
	size_t capacity;
	size_t size;
	char *buffer;
};

static inline
struct strbuffer* strbuffer_new(void)
{
	return ecalloc(1, sizeof(struct strbuffer));
}

static inline
void strbuffer_free(struct strbuffer *sb)
{
	free(sb->buffer);
	free(sb);
}

static inline
void strbuffer_clean(struct strbuffer *sb)
{
	memset(sb->buffer, 0, sb->size * sizeof(char));
	sb->size = 0;
}

static inline
char* strbuffer_to_string(struct strbuffer *sb)
{
	char *string = emalloc(sb->size + 1, sizeof(char));
	string[sb->size] = 0;
	return memcpy(string, sb->buffer, sb->size * sizeof(char));
}

void strbuffer_reserve(struct strbuffer *sb, size_t size);

size_t strbuffer_insertn(struct strbuffer *sb, size_t index,
			 const char *str, size_t size);
#define strbuffer_insert(sb, index, str) \
	strbuffer_insertn(sb, index, str, (str) ? strlen(str) : 0)

#define strbuffer_appendn(sb, str, size_) \
	strbuffer_insertn(sb, (sb) ? (sb)->size : 0, str, size_)
#define strbuffer_append(sb, str) \
	strbuffer_appendn(sb, str, (str) ? strlen(str) : 0)

static inline
size_t strbuffer_insert_char(struct strbuffer *sb, size_t index, char c)
{
	return strbuffer_insertn(sb, index, &c, 1);
}

size_t strbuffer_append_char(struct strbuffer *sb, char c);

size_t strbuffer_insert_int(struct strbuffer *sb, size_t index,
			    unsigned long long num);
#define strbuffer_append_int(sb, num) \
	strbuffer_insert_int(sb, (sb) ? (sb)->size : 0, num)

size_t strbuffer_appendf(struct strbuffer *sb, const char *format, ...);

#endif /* STRBUFFER_H */
