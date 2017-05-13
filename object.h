/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_OBJECT_H
#define JONSON_OBJECT_H

#include "jonson.h"
#include "ecalloc.h"

struct json_bucket {
	char *key;
	struct json value;
};

/*
 * A simple linked list to keep track of the order in which
 * buckets are added to a [struct json_object].
 */
struct json_bucket_list {
	struct json_bucket *data;
	struct json_bucket_list *next;
};

struct json_object {
	size_t capacity;
	size_t size;
	float load_factor;
	struct json_bucket *data;
	struct json_bucket_list *list;
	struct json_bucket_list *end;
};

/*
 * Hash function created by Daniel J. Bernstein.
 */
uint32_t json_hashn(const char *str, size_t size);
#define json_hash(str) json_hashn(str, (str) ? strlen(str) : 0)

struct json json_object_new(void);

void json_object_free(struct json object);

static inline void json_object_set_load_factor(struct json object,
					       float load_factor)
{
	JSON_OBJVAL(object)->load_factor = load_factor;
}

/*
 * Reserves space to fit at least 'size' elements into a [struct json_object].
 * Resizing involves reallocation of almost the entire object. Use with care.
 * Call this function before adding elements, if the amount is known
 * in advance (even if only approximately).
 */
void json_object_reserve(struct json object, size_t size);

/*
 * To prevent a value from being freed when overwritten, use
 * json_object_remove() to remove it first.
 */
void json_object_setn(struct json object, const char *key,
		      size_t key_size, struct json value);
#define json_object_set(object, key, value) \
	json_object_setn(object, key, (key) ? strlen(key) : 0, value)

/*
 * The type of the returned value will be JSON_TYPE_NONE
 * if no value was found at the specified index.
 */
struct json json_object_getn(struct json object,
			     const char *key, size_t key_size);
#define json_object_get(object, key) \
	json_object_getn(object, key, (key) ? strlen(key) : 0)

JSON_TYPE json_object_try_getn(struct json object, const char *key,
			       size_t key_size, struct json *out_value);
#define json_object_try_get(object, key, out_value) \
	json_object_try_getn(object, key, (key) ? strlen(key) : 0, out_value)

#endif /* JONSON_OBJECT_H */
