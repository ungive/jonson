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
	return object;
}

void json_object_free(struct json_object *object)
{
	struct json_bucket_list *current = object->list;
	while (current) {
		struct json_bucket_list *next = current->next;
		json_free(current->data->value);
		free(current->data->key);
		free(current);
		current = next;
	}

	free(object->data);
	free(object);
}

void json_object_reserve(struct json_object *object, size_t size)
{
	if (size <= object->capacity)
		return;

	struct json_bucket *data = object->data;
	struct json_bucket_list *current = object->list;

	object->data = ecalloc(size, sizeof(struct json_bucket));
	object->list = NULL;
	object->capacity = size;
	object->size = 0;

	while (current) {
		struct json_bucket_list *next = current->next;
		struct json_bucket *data = current->data;
		json_object_set(object, data->key, data->value);
		free(data->key);
		free(current);
		current = next;
	}

	free(data);
	free(current);
}

void json_object_setn(struct json_object *object, const char *key,
		      size_t key_size, struct json value)
{
	if (object->size >= object->capacity * object->load_factor) {
		size_t size = object->capacity ?
			(object->capacity << 1) : JSON_OBJECT_INITIAL_CAPACITY;
		json_object_reserve(object, size);
	}

	struct json_bucket *bucket = NULL;
	int same_keys = 0;

	size_t i, index = json_hashn(key, key_size) % object->capacity;
	for (i = 0; i < object->capacity; ++i, index = (index + 1) % object->capacity) {
		bucket = object->data + index;
		if (bucket->key && !(same_keys = strncmp(bucket->key, key, key_size) == 0)) {
			bucket = NULL;
			same_keys = 0;
			continue;
		}

		if (!bucket->key)
			bucket->key = json_strdupn(key, key_size);
		json_free(bucket->value);
		bucket->value = value;
		++object->size;

		break;
	}

	if (!bucket)
		return;

	if (!object->list) {
		object->list = ecalloc(1, sizeof(struct json_bucket_list));
		object->end = object->list;
	}

	if (same_keys) { /* the value was overwritten */
		struct json_bucket_list *previous = object->list;
		struct json_bucket_list *current = object->list;

		/* find the bucket that was edited */
		while (current && current->next && current->data != bucket) {
			previous = current;
			current = current->next;
		}

		if (current) {
			if (current->next) {
				/* put the bucket at the end of the list */
				previous->data = current->next->data;
				previous->next = current->next->next;
				object->end->data = current->data;
			}
			return;
		}
	}

	/* append the value to the end of the list */
	if (!object->end->data)
		object->end->data = bucket;
	else {
		object->end->next = ecalloc(1, sizeof(struct json_bucket_list));
		object->end->next->data = bucket;
		object->end = object->end->next;
	}
}

struct json json_object_getn(struct json_object *object,
			     const char *key, size_t key_size)
{
	size_t i, index = json_hashn(key, key_size) % object->capacity;
	for (i = 0; i < object->capacity; ++i, index = (index + 1) % object->capacity) {
		struct json_bucket *bucket = object->data + index;
		if (bucket->key && strncmp(bucket->key, key, key_size) == 0)
			return bucket->value;
	}

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
