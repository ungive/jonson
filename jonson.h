/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_H
#define JONSON_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct json_object;
struct json_array;

union json_value {
	char *string;
	double number;
	int boolean;
	struct json_object *object;
	struct json_array *array;
};

enum json_type {
	JSON_TYPE_NONE,
	JSON_TYPE_NULL,
	JSON_TYPE_STRING,
	JSON_TYPE_NUMBER,
	JSON_TYPE_BOOLEAN,
	JSON_TYPE_OBJECT,
	JSON_TYPE_ARRAY
};

/*
 * Use the type to interpret the value correctly.
 */
struct json {
	union json_value value;
	enum json_type type;
};

/*
 * Creates a [struct json], with its root element being of the specified type.
 * Only types JSON_TYPE_OBJECT and JSON_TYPE_ARRAY are returned because
 * no other can be nested or would be simpler to create with this function
 * (use the macros defined at the bottom of this header instead).
 */
struct json json_build(enum json_type type, ...);

void json_free(struct json value);

/*
 * Serialises a [struct json] (converts it to string representation).
 * NULL is returned if a value of type JSON_TYPE_NONE is passed.
 */
char *json_serialise(struct json value);
#define json_serialize(value) json_serialise(value)

static inline char *json_strndup(const char *str, size_t size)
{
	char *copy = malloc((size + 1) * sizeof(char));
	copy[size] = 0;
	return memcpy(copy, str, size * sizeof(char));
}

/*
 * Encapsulates a value in a [struct json].
 */
#define JSON_STRN(data, size) \
	((struct json){ .type = JSON_TYPE_STRING, \
			.value.string = json_strndup(data, size) })
#define JSON_STR(data) JSON_STRN(data, (data) ? strlen(data) : 0)
#define JSON_NUM(data) ((struct json){ .type = JSON_TYPE_NUMBER, .value.number = data })
#define JSON_BOOL(data) ((struct json){ .type = JSON_TYPE_BOOLEAN, .value.boolean = data })
#define JSON_OBJ(data) ((struct json){ .type = JSON_TYPE_OBJECT, .value.object = data })
#define JSON_ARR(data) ((struct json){ .type = JSON_TYPE_ARRAY, .value.array = data })
#define JSON_NULL ((struct json){ .type = JSON_TYPE_NULL, .value = { 0 } })
#define JSON_NONE ((struct json){ .type = JSON_TYPE_NONE, .value = { 0 } })

/*
 * Macros for use with the function json_build().
 * JSON_END and JSON_KVP_END are for delimiting the variable argument list.
 * JSON_KVP() creates a bucket (key-value-pair) for an object.
 */
#define JSON_END JSON_NONE
#define JSON_KVP(k, v) ((struct json_bucket){ .key = k, .value = v })
#define JSON_KVP_END JSON_KVP(NULL, JSON_END)

#define JSON_STRVAL(v) (v).value.string
#define JSON_NUMVAL(v) (v).value.number
#define JSON_BOOLVAL(v) (v).value.boolean
#define JSON_OBJVAL(v) (v).value.object
#define JSON_ARRVAL(v) (v).value.array

#include "object.h"
#include "array.h"

#endif /* JONSON_H */
