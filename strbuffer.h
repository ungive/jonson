/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef STRBUFFER_H
#define STRBUFFER_H

#include <stdlib.h>
#include <string.h>

#include "ecalloc.h"

typedef struct strbuffer {
	size_t capacity;
	size_t size;
	char *buffer;
} strbuffer_t;

static inline
strbuffer_t* strbuffer_new(void)
{
	return ecalloc(1, sizeof(strbuffer_t));
}

static inline
void strbuffer_free(strbuffer_t *sb)
{
	free(sb->buffer);
	free(sb);
}

static inline
void strbuffer_clean(strbuffer_t *sb)
{
	memset(sb->buffer, 0, sb->size * sizeof(char));
	sb->size = 0;
}

static inline
char* strbuffer_to_string(strbuffer_t *sb)
{
	char *string = ecalloc(sb->size + 1, sizeof(char));
	return memcpy(string, sb->buffer, sb->size * sizeof(char));
}

void strbuffer_reserve(strbuffer_t *sb, size_t size);

size_t strbuffer_insertn(strbuffer_t *sb, size_t index,
			 const char *str, size_t size);
#define strbuffer_insert(sb, index, str) \
	strbuffer_insertn(sb, index, str, (str) ? strlen(str) : 0)

#define strbuffer_appendn(sb, str, size_) \
	strbuffer_insertn(sb, (sb) ? (sb)->size : 0, str, size_)
#define strbuffer_append(sb, str) \
	strbuffer_appendn(sb, str, (str) ? strlen(str) : 0)

static inline
size_t strbuffer_insert_char(strbuffer_t *sb, size_t index, char c)
{
	return strbuffer_insertn(sb, index, &c, 1);
}

static inline
size_t strbuffer_append_char(strbuffer_t *sb, char c)
{
	return strbuffer_appendn(sb, &c, 1);
}

size_t strbuffer_insert_int(strbuffer_t *sb, size_t index,
			    unsigned long long num);
#define strbuffer_append_int(sb, num) \
	strbuffer_insert_int(sb, (sb) ? (sb)->size : 0, num)

size_t strbuffer_appendf(strbuffer_t *sb, const char *format, ...);

#endif /* STRBUFFER_H */
