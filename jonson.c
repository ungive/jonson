/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "jonson.h"
#include "object.h"
#include "array.h"
// #include "token.h"
#include "strbuffer.h"
// #include "stack.h"

struct json json_build(enum json_type type, ...)
{
	struct json result;

	va_list args;
	va_start(args, type);

	if (type == JSON_TYPE_OBJECT) {
		struct json_object *object = json_object_new();
		while (1) {
			struct json_bucket bucket = va_arg(args, struct json_bucket);
			if (bucket.value.type == JSON_TYPE_NONE)
				break;
			json_object_set(object, bucket.key, bucket.value);
		}
		result = JSON_OBJ(object);
	}
	else {
		struct json_array *array = json_array_new();
		while (1) {
			struct json value = va_arg(args, struct json);
			if (value.type == JSON_TYPE_NONE)
				break;
			json_array_add(array, value);
		}
		result = JSON_ARR(array);
	}

	va_end(args);
	return result;
}

void json_free(struct json value)
{
	switch (value.type) {
	case JSON_TYPE_STRING: free(JSON_STRVAL(value)); return;
	case JSON_TYPE_OBJECT: json_object_free(JSON_OBJVAL(value)); return;
	case JSON_TYPE_ARRAY:  json_array_free(JSON_ARRVAL(value)); return;
	default: return;
	}
}

char *json_serialise(struct json value)
{
	struct strbuffer *sb = strbuffer_new();

	switch (value.type) {
	case JSON_TYPE_NONE:
	case JSON_TYPE_NULL:
		strbuffer_append(sb, "null");
		break;
	case JSON_TYPE_STRING: {
		const char *string = JSON_STRVAL(value);
		size_t size = strlen(string);

		strbuffer_reserve(sb, size + 2 + 1);
		strbuffer_append_char(sb, '"');

		size_t i;
		for (i = 0; i < size; ++i) {
			char unescaped = 0;
			switch (string[i]) {
			case '\\': unescaped = '\\';
			case '"': unescaped = '"';
			case '/': unescaped = '/';
			case '\b': unescaped = 'b';
			case '\f': unescaped = 'f';
			case '\n': unescaped = 'n';
			case '\r': unescaped = 'r';
			case '\t': unescaped = 't';
			default:
				strbuffer_append_char(sb, string[i]);
				continue;
			}
			strbuffer_append_char(sb, '\\');
			strbuffer_append_char(sb, unescaped);
		}

		strbuffer_append_char(sb, '"');
		break;
	}
	case JSON_TYPE_NUMBER: {
		strbuffer_reserve(sb, 21);
		char buf[21];
		snprintf(buf, 21, "%.16g", JSON_NUMVAL(value));
		strbuffer_append(sb, buf);
		break;
	}
	case JSON_TYPE_BOOLEAN: {
		int boolean = JSON_BOOLVAL(value);
		strbuffer_reserve(sb, 4 + !boolean + 1);
		strbuffer_append(sb, boolean ? "true" : "false");
		break;
	}
	case JSON_TYPE_OBJECT: {
		struct json_object *object = JSON_OBJVAL(value);
		strbuffer_append_char(sb, '{');

		for (size_t i = 0; i < object->size; ++i) {
			struct json_bucket *bucket = object->buckets + object->order[i];
			char *value = json_serialise(bucket->value);
			char *key = bucket->key;

			strbuffer_append_char(sb, '"');
			strbuffer_append(sb, key);
			strbuffer_append_char(sb, '"');
			strbuffer_append_char(sb, ':');
			strbuffer_append(sb, value);
			if (i < object->size - 1)
				strbuffer_append_char(sb, ',');

			free(value);
		}

		strbuffer_append_char(sb, '}');
		break;
	}
	case JSON_TYPE_ARRAY: {
		struct json_array *array = JSON_ARRVAL(value);
		strbuffer_append_char(sb, '[');

		size_t i = 0;
		while (i < array->size) {
			char *val = json_serialise(json_array_get(array, i));
			strbuffer_append(sb, val);
			free(val);

			if (++i < array->size)
				strbuffer_append_char(sb, ',');
		}

		strbuffer_append_char(sb, ']');
		break;
	}
	}

	char *result = strbuffer_to_string(sb);
	strbuffer_free(sb);
	return result;
}
