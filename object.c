/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>

#include "config.h"
#include "jonson.h"
#include "object.h"
#include "ecalloc.h"

uint32_t json_hashn(const char *str, size_t size)
{
	uint32_t hash = 5381;
	size_t i;
	for (i = 0; i < size; ++i)
		hash = ((hash << 5) + hash) + str[i];
	return hash;
}

struct json_object *json_object_new(void)
{
	struct json_object *object = ecalloc(1, sizeof(struct json_object));
	object->load_factor = JSON_OBJECT_INITIAL_LOAD_FACTOR;
	object->order_last = &object->order_first;
	return object;
}

void json_object_free(struct json_object *object)
{
	struct json_bucket *current = object->order_first;
	while (current) {
		struct json_bucket *next = current->order_next;
		json_free(current->value);
		free(current->key);
		free(current);
		current = next;
	}
	free(object->buckets);
	free(object);
}

void json_object_reserve(struct json_object *object, size_t size)
{
	if (size <= object->capacity)
		return;

	struct json_bucket **buckets = object->buckets;
	struct json_bucket *current = object->order_first;

	object->buckets = ecalloc(size, sizeof(struct json_bucket));
	object->order_first = NULL;
	object->order_last = &object->order_first;
	object->capacity = size;
	object->size = 0;

	while (current) {
		struct json_bucket *next = current->order_next;
		json_object_set(object, current->key, current->value);
		free(current->key);
		free(current);
		current = next;
	}
	free(buckets);
}

static int json_strneq(register const char *str1,
		       register const char *str2,
		       size_t num)
{
	while (*str1++ == *str2)
		if (!*str2++ || --num == 0)
			return 1;
	return 0;
}

void json_object_setn(struct json_object *object, const char *key,
		      size_t key_size, struct json value)
{
	if (object->size >= object->capacity * object->load_factor)
		json_object_reserve(object, object->capacity ?
			(object->capacity << 1) : JSON_OBJECT_INITIAL_CAPACITY);

	size_t index = json_hashn(key, key_size) % object->capacity;

	/* Remove the element with the same key. */
	struct json_bucket *current = object->buckets[index];
	for (; current; current = current->next)
		if (json_strneq(current->key, key, key_size)) {
			*current->order_pnext = current->order_next;
			object->order_last = current->order_pnext;
			json_free(current->value);
			free(current->key);
			free(current);
			--object->size;
			break;
		}

	/* Create a new bucket and fill it with the passed key and value. */
	struct json_bucket *bucket = ecalloc(1, sizeof(struct json_bucket));
	bucket->key = json_strndup(key, key_size);
	bucket->value = value;

	/* Set this bucket's next bucket to the bucket
	   that has already been at this index. */
	bucket->next = object->buckets[index];

	/* The previous' next pointer points to this bucket. */
	bucket->order_pnext = object->order_last;

	/* Set the bucket at this index the the newly created bucket. */
	object->buckets[index] = bucket;

	/* The last bucket in the order is now this bucket.
	   The next last bucket is going to be this bucket's next bucket. */
	*object->order_last = bucket;
	object->order_last = &bucket->order_next;

	++object->size;
}

struct json json_object_getn(struct json_object *object,
			     const char *key, size_t key_size)
{
	size_t index = json_hashn(key, key_size) % object->capacity;

	struct json_bucket *current = object->buckets[index];
	for (; current; current = current->order_next)
		if (json_strneq(current->key, key, key_size))
			return current->value;

	return JSON_NONE;
}

JSON_TYPE json_object_try_getn(struct json_object *object, const char *key,
			       size_t key_size, struct json *out_value)
{
	struct json value = json_object_getn(object, key, key_size);
	if (out_value)
		*out_value = value;
	return value.type;
}
