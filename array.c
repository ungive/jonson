/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>

#include "array.h"

#define INIT_CAPACITY 16

struct json_array *json_array_new(void)
{
	struct json_array *array = emalloc(1, sizeof(struct json_array));
	array->capacity = INIT_CAPACITY;
	array->size = 0;
	array->data = ecalloc(array->capacity, sizeof(struct json));
	return array;
}

void json_array_free(struct json_array *array)
{
	size_t i;
	for (i = 0; i < array->size; ++i)
		json_free(array->data[i]);

	free(array->data);
	free(array);
}

void json_array_reserve(struct json_array *array, size_t size)
{
	if (size <= array->capacity)
		return;

	array->data = erealloc(array->data, size, sizeof(struct json));
	array->capacity = size;
}

void json_array_resize(struct json_array *array, size_t size)
{
	if (size < array->size) {
		size_t i;
		struct json *end = array->data + size;
		for (i = 0; i < size; ++i)
			json_free(end[i]);
		memset(end, 0, array->size - size);
		array->size = size;
	}
	else {
		json_array_reserve(array, size);
	}
}

void json_array_add(struct json_array *array, struct json value)
{
	if (array->size >= array->capacity)
		json_array_reserve(array, array->capacity << 1);
	
	array->data[array->size++] = value;
}

struct json json_array_get(struct json_array *array, size_t index)
{
	if (index >= array->size)
		return JSON_NONE;

	return array->data[index];
}
