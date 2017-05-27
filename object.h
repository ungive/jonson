/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_OBJECT_H
#define JONSON_OBJECT_H

#include "jonson.h"

struct json_bucket {
	char *key;
	struct json value;
	struct json_bucket *next;
	struct json_bucket *order_next;
	struct json_bucket **order_pnext;
};

struct json_object {
	size_t capacity;
	size_t size;
	float load_factor;
	struct json_bucket **buckets;
	struct json_bucket *order_first;
	struct json_bucket **order_last;
};

/*
 * Hash function created by Daniel J. Bernstein.
 */
uint32_t json_hashn(const char *str, size_t size);
uint32_t json_hash(const char *str);

struct json_object *json_object_new(void);

void json_object_free(struct json_object *object);

/*
 * Reserves space to fit at least 'size' elements into a [struct json_object].
 * Resizing involves reallocation of almost the entire object. Use with care.
 * Call this function before adding elements, if the amount is known
 * in advance (even if only approximately).
 */
void json_object_reserve(struct json_object *object, size_t size);

/*
 * To prevent a value from being freed when overwritten, use
 * json_object_remove() to remove it first.
 */
void json_object_setn(struct json_object *object, const char *key,
		      size_t key_size, struct json value);
#define json_object_set(object, key, value) \
	json_object_setn(object, key, (key) ? strlen(key) : 0, value)

/*
 * The type of the returned value will be JSON_TYPE_NONE
 * if no value was found at the specified index.
 */
struct json json_object_getn(struct json_object *object,
			     const char *key, size_t key_size);
#define json_object_get(object, key) \
	json_object_getn(object, key, (key) ? strlen(key) : 0)

JSON_TYPE json_object_try_getn(struct json_object *object, const char *key,
			       size_t key_size, struct json *out_value);
#define json_object_try_get(object, key, out_value) \
	json_object_try_getn(object, key, (key) ? strlen(key) : 0, out_value)

#endif /* JONSON_OBJECT_H */
