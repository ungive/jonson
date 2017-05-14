/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include "ealloc.h"

void *emalloc(size_t num, size_t size)
{
	void *mem = malloc(num * size);
	if (!mem) {
		fprintf(stderr, "Failed to allocate %zu bytes\n", num * size);
		exit(EXIT_FAILURE);
	}
	return mem;
}

void *ecalloc(size_t num, size_t size)
{
	void *mem = calloc(num, size);
	if (!mem) {
		fprintf(stderr, "Failed to allocate %zu bytes\n", num * size);
		exit(EXIT_FAILURE);
	}
	return mem;
}

void *erealloc(void *ptr, size_t num, size_t size)
{
	void *mem = realloc(ptr, num * size);
	if (!mem && size) {
		fprintf(stderr, "Failed to reallocate %zu bytes\n", num * size);
		exit(EXIT_FAILURE);
	}
	return mem;
}
