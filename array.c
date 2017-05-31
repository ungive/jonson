/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "array.h"

#define INIT_CAPACITY 16

struct json_array *json_array_new(void)
{
	struct json_array *array = malloc(sizeof(struct json_array));
	if (!array)
		goto error_array;

	array->size = 0;
	array->capacity = INIT_CAPACITY;
	array->data = malloc(array->capacity * sizeof(struct json));
	if (!array->data)
		goto error_data;

	return array;

error_data:
	free(array);
error_array:
	return NULL;
}

void json_array_free(struct json_array *array)
{
	for (size_t i = 0; i < array->size; ++i)
		json_free(array->data[i]);
	free(array->data);
	free(array);
}

int json_array_reserve(struct json_array *array, size_t size)
{
	if (size > array->capacity) {
		array->capacity = size;
		array->data = realloc(array->data, size * sizeof(struct json));
		if (!array->data)
			return 0;
	}
	return 1;
}

int json_array_resize(struct json_array *array, size_t size)
{
	if (size == array->size)
		return 1;
	if (size > array->size)
		return json_array_reserve(array, size);

	struct json *end = array->data + size;
	size_t end_size = array->size - size;
	for (size_t i = 0; i < end_size; ++i)
		json_free(end[i]);
	array->size = size;

	return 1;
}

int json_array_add(struct json_array *array, struct json value)
{
	if (array->size >= array->capacity)
		return json_array_reserve(array, array->capacity << 1);
	
	array->data[array->size++] = value;
	return 1;
}
