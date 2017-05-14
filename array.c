/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>

#include "config.h"
#include "jonson.h"

struct json_array *json_array_new(void)
{
	struct json_array *array = ecalloc(1, sizeof(struct json_array));
	return array;
}

void json_array_free(struct json_array *array)
{
	if (array->data) {
		size_t i;
		for (i = 0; i < array->size; ++i)
			json_free(array->data[i]);
	}

	free(array->data);
	free(array);
}

void json_array_reserve(struct json_array *array, size_t size)
{
	if (size <= array->capacity)
		return;

	struct json *data = ecalloc(size, sizeof(struct json));

	if (array->data) {
		size_t i;
		for (i = 0; i < array->capacity; ++i)
			data[i] = array->data[i];
	}
	free(array->data);

	array->data = data;
	array->capacity = size;
}

void json_array_resize(struct json_array *array, size_t size)
{
	if (size >= array->size)
		json_array_reserve(array, size);
	else {
		size_t i;
		struct json *end = array->data + size;
		for (i = 0; i < size; ++i)
			json_free(end[i]);
		memset(end, 0, array->size - size);
		array->size = size;
	}
}

void json_array_add(struct json_array *array, struct json value)
{
	if (array->size >= array->capacity) {
		size_t size = array->capacity ?
			(array->capacity << 1) : JSON_ARRAY_INITIAL_CAPACITY;
		json_array_reserve(array, size);
	}

	array->data[array->size++] = value;
}

struct json json_array_get(struct json_array *array, size_t index)
{
	if (index >= array->size)
		return JSON_NONE;
	return array->data[index];
}
