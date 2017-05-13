/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>

#include "config.h"
#include "jonson.h"
#include "array.h"

struct json json_array_new(void)
{
	return JSON_ARR(ecalloc(1, sizeof(struct json_array)));
}

void json_array_free(struct json array)
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

void json_array_reserve(struct json array, size_t size)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (size <= arr->capacity)
		return;

	struct json *data = ecalloc(size, sizeof(struct json));

	if (arr->data) {
		size_t i;
		for (i = 0; i < arr->capacity; ++i)
			data[i] = arr->data[i];
	}
	free(arr->data);

	arr->data = data;
	arr->capacity = size;
}

void json_array_resize(struct json array, size_t size)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (size >= arr->size)
		json_array_reserve(array, size);
	else {
		size_t i;
		struct json *end = arr->data + size;
		for (i = 0; i < size; ++i)
			json_free(end[i]);
		memset(end, 0, arr->size - size);
		arr->size = size;
	}
}

void json_array_add(struct json array, struct json value)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (arr->size >= arr->capacity) {
		size_t size = arr->capacity ?
			(arr->capacity << 1) : JSON_ARRAY_INITIAL_CAPACITY;
		json_array_reserve(array, size);
	}

	arr->data[arr->size++] = value;
}

struct json json_array_get(struct json array, size_t index)
{
	struct json_array *arr = JSON_ARRVAL(array);

	if (index >= arr->size)
		return JSON_NONE;

	return arr->data[index];
}
