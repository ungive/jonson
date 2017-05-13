/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>

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

void json_object_free(json_t object)
{
	struct json_object *obj = JSON_OBJVAL(object);

	struct json_bucket_list *current = obj->list;
	while (current) {
		struct json_bucket_list *next = current->next;
		json_free(current->data->value);
		free(current->data->key);
		free(current);
		current = next;
	}

	free(obj->data);
	free(obj);
}

void json_object_reserve(json_t object, size_t size)
{
	struct json_object *obj = JSON_OBJVAL(object);

	if (size <= obj->capacity)
		return;

	struct json_bucket *data = obj->data;
	struct json_bucket_list *current = obj->list;

	obj->data = ecalloc(size, sizeof(struct json_bucket));
	obj->list = NULL;
	obj->capacity = size;
	obj->size = 0;

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

void json_object_setn(json_t object, const char *key,
		      size_t key_size, json_t value)
{
	struct json_object *obj = JSON_OBJVAL(object);

	if (obj->size >= obj->capacity) {
		size_t size = obj->capacity ? (obj->capacity << 1) : 16;
		json_object_reserve(object, size);
	}

	struct json_bucket *bucket = NULL;
	int same_keys = 0;

	size_t i, index = json_hashn(key, key_size) % obj->capacity;
	for (i = 0; i < obj->capacity; ++i, index = (index + 1) % obj->capacity) {
		bucket = obj->data + index;
		if (bucket->key && !(same_keys = strncmp(bucket->key, key, key_size) == 0)) {
			bucket = NULL;
			same_keys = 0;
			continue;
		}

		if (!bucket->key)
			bucket->key = json_strdupn(key, key_size);
		json_free(bucket->value);
		bucket->value = value;
		++obj->size;

		break;
	}

	if (!bucket)
		return;

	if (!obj->list) {
		obj->list = ecalloc(1, sizeof(struct json_bucket_list));
		obj->end = obj->list;
	}

	if (same_keys) { /* the value was overwritten */
		struct json_bucket_list *previous = obj->list;
		struct json_bucket_list *current = obj->list;

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
				obj->end->data = current->data;
			}
			return;
		}
	}

	/* append the value to the end of the list */
	if (!obj->end->data)
		obj->end->data = bucket;
	else {
		obj->end->next = ecalloc(1, sizeof(struct json_bucket_list));
		obj->end->next->data = bucket;
		obj->end = obj->end->next;
	}
}

json_t json_object_getn(json_t object, const char *key, size_t key_size)
{
	struct json_object *obj = JSON_OBJVAL(object);

	size_t i, index = json_hashn(key, key_size) % obj->capacity;
	for (i = 0; i < obj->capacity; ++i, index = (index + 1) % obj->capacity) {
		struct json_bucket *bucket = obj->data + index;
		if (bucket->key && strncmp(bucket->key, key, key_size) == 0)
			return bucket->value;
	}

	return JSON_NONE;
}

JSON_TYPE json_object_try_getn(json_t object, const char *key,
			       size_t key_size, json_t *out_value)
{
	json_t value = json_object_getn(object, key, key_size);
	if (out_value)
		*out_value = value;
	return value.type;
}
