/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <stddef.h>

#include "jonson.h"
#include "array.h"

size_t json_array_chunk_size = 16;

void json_array_free(json_t array)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (arr->data) {
		size_t i;
		for (i = 0; i < arr->size; ++i)
			json_free(arr->data[i]);
	}

	free(arr->data);
	free(arr);
}

void json_array_reserve(json_t array, size_t size)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (size <= arr->capacity)
		return;

	json_t *data = ecalloc(size, sizeof(json_t));

	if (arr->data) {
		size_t i;
		for (i = 0; i < arr->capacity; ++i)
			data[i] = arr->data[i];
	}
	free(arr->data);

	arr->data = data;
	arr->capacity = size;
}

void json_array_resize(json_t array, size_t size)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (size >= arr->size)
		json_array_reserve(array, size);
	else {
		size_t i;
		json_t *end = arr->data + size;
		for (i = 0; i < size; ++i)
			json_free(end[i]);
		memset(end, 0, arr->size - size);
		arr->size = size;
	}
}

void json_array_add(json_t array, json_t value)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (arr->size >= arr->capacity)
		json_array_reserve(array, arr->capacity + json_array_chunk_size);

	arr->data[arr->size++] = value;
}

json_t json_array_get(json_t array, size_t index)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (index >= arr->size)
		return JSON_NONE;

	return arr->data[index];
}
