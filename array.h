/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
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

struct json_array *json_array_new(void);

void json_array_free(struct json_array *array);

int json_array_reserve(struct json_array *array, size_t size);

int json_array_resize(struct json_array *array, size_t size);

int json_array_add(struct json_array *array, struct json value);

static inline struct json json_array_get(struct json_array *array, size_t index)
{
	return index < array->size ? array->data[index] : JSON_NONE;
}

#endif /* JONSON_ARRAY_H */
