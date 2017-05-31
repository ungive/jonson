/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "object.h"

#define INIT_LOAD_FACTOR 0.5f
#define INIT_CAPACITY    16
#define FNV_OFFSET_BASIS 2166136261
#define FNV_PRIME        16777619

uint32_t json_hash(const char *str)
{
	uint32_t hash = FNV_OFFSET_BASIS;
	char c;
	while ((c = *str++)) {
		hash ^= c;
		hash *= FNV_PRIME;
	}
	return hash;
}

uint32_t json_hashn(const char *str, size_t size)
{
	uint32_t hash = FNV_OFFSET_BASIS;
	size_t i;
	for (i = 0; i < size; ++i) {
		hash ^= str[i];
		hash *= FNV_PRIME;
	}
	return hash;
}

struct json_object *json_object_new(void)
{
	struct json_object *object = malloc(sizeof(struct json_object));
	if (!object)
		goto error_object;

	object->load_factor = INIT_LOAD_FACTOR;
	object->capacity = INIT_CAPACITY;
	object->size = 0;

	object->buckets = calloc(object->capacity, sizeof(struct json_bucket));
	if (!object->buckets)
		goto error_buckets;

	object->order = malloc(object->capacity * sizeof(size_t));
	if (!object->order)
		goto error_order;

	return object;

error_order:
	free(object->buckets);
error_buckets:
	free(object);
error_object:
	return NULL;
}

void json_object_free(struct json_object *object)
{
	for (size_t i = 0; i < object->size; ++i)
		free(object->buckets[object->order[i]].key);
	free(object->buckets);
	free(object->order);
	free(object);
}

int json_object_reserve(struct json_object *object, size_t size)
{
	if (size <= object->capacity)
		return 1;

	struct json_bucket *buckets = object->buckets;

	object->buckets = calloc(size, sizeof(struct json_bucket));
	if (!object->buckets)
		goto error_buckets;

	object->order = realloc(object->order, size * sizeof(size_t));
	if (!object->order)
		goto error_order;

	for (size_t i = 0; i < object->size; ++i) {
		struct json_bucket *bucket = buckets + object->order[i];
		size_t index = json_hash(bucket->key) % object->capacity;

		for (;; index = (index + 1) % object->capacity) {
			struct json_bucket *current = object->buckets + index;
			if (!current->key) {
				current->key = bucket->key;
				current->value = bucket->value;
				object->order[i] = index;
				break;
			}
		}
	}

	object->capacity = size;
	free(buckets);
	return 1;

error_order:
	free(object->buckets);
error_buckets:
	object->buckets = buckets;
	return 0;
}

int json_object_set_n(struct json_object *object, const char *key,
                       size_t key_size, struct json value)
{
	if (object->size >= object->capacity * object->load_factor)
		if (!json_object_reserve(object, object->capacity << 1))
			return 0;

	size_t index = json_hashn(key, key_size) % object->capacity;

	for (;; index = (index + 1) % object->capacity) {
		struct json_bucket *bucket = object->buckets + index;

		if (bucket->key) {
			if (strncmp(bucket->key, key, key_size) == 0) {
				json_free(bucket->value);
				bucket->value = value;
				return 1;
			}
			continue;
		}

		bucket->value = value;
		bucket->key = json_strndup(key, key_size);
		if (!bucket->key)
			return 0;
		break;
	}

	object->order[object->size++] = index;
	return 1;
}

struct json json_object_get_n(struct json_object *object,
                              const char *key, size_t key_size)
{
	size_t index = json_hashn(key, key_size) % object->capacity;

	for (;; index = (index + 1) % object->capacity) {
		struct json_bucket *bucket = object->buckets +index;
		if (!bucket->key)
			return JSON_NONE;
		if (strncmp(bucket->key, key, key_size) == 0)
			return bucket->value;
	}
}

enum json_type json_object_try_get_n(struct json_object *object, const char *key,
                                     size_t key_size, struct json *out_value)
{
	struct json value = json_object_get_n(object, key, key_size);
	if (out_value)
		*out_value = value;
	return value.type;
}
