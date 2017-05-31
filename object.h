/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_OBJECT_H
#define JONSON_OBJECT_H

#include "jonson.h"

#include <stdint.h>

struct json_bucket {
	char *key;
	struct json value;
};

struct json_object {
	float load_factor;
	size_t capacity;
	size_t size;
	size_t *order;
	struct json_bucket *buckets;
};

uint32_t json_hashn(const char *str, size_t size);
uint32_t json_hash(const char *str);

struct json_object *json_object_new(void);

void json_object_free(struct json_object *object);

int json_object_reserve(struct json_object *object, size_t size);

int json_object_set_n(struct json_object *object, const char *key,
                      size_t key_size, struct json value);

#define json_object_set(object, key, value) \
        json_object_set_n(object, key, strlen(key), value)

struct json json_object_get_n(struct json_object *object,
                             const char *key, size_t key_size);

#define json_object_get(object, key) \
        json_object_get_n(object, key, (key) ? strlen(key) : 0)

enum json_type json_object_try_get_n(struct json_object *object, const char *key,
                                     size_t key_size, struct json *out_value);

#define json_object_try_get(object, key, out_value) \
        json_object_try_get_n(object, key, (key) ? strlen(key) : 0, out_value)

#endif /* JONSON_OBJECT_H */
