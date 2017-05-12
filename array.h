/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_ARRAY_H
#define JONSON_ARRAY_H

#include "jonson.h"

struct json_array {
	size_t capacity;
	size_t size;
	struct json *data;
};

/*
 * The amount by which a [struct json_array]'s data array is resized.
 * Used by json_object_set() if there is no space to add an element.
 */
extern size_t json_array_chunk_size;

static inline json_t json_array_new(void)
{
	return JSON_ARR(ecalloc(1, sizeof(struct json_array)));
}

void json_array_free(json_t array);

/*
 * Reserves space to fit at least 'size' elements into a [struct json_array].
 */
void json_array_reserve(json_t array, size_t size);

/*
 * Resizes a [struct json_array] to 'size'.
 * Elements that excess are freed and deleted.
 */
void json_array_resize(json_t array, size_t size);

void json_array_add(json_t array, json_t value);

/*
 * The type of the returned value will be JSON_TYPE_NONE
 * if the index exceeds the size of the array.
 */
json_t json_array_get(json_t array, size_t index);

#endif /* JONSON_ARRAY_H */
