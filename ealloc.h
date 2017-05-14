/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef EALLOC_H
#define EALLOC_H

#include <stddef.h>

void *emalloc(size_t num, size_t size);
void *ecalloc(size_t num, size_t size);
void *erealloc(void *ptr, size_t num, size_t size);

#endif /* EALLOC_H */
