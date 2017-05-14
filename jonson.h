/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_H
#define JONSON_H

#include <stdint.h>
#include <string.h>

#include "ealloc.h"

struct json_object;
struct json_array;

union json_value {
	char *string;
	double number;
	int boolean;
	struct json_object *object;
	struct json_array *array;
};

typedef enum JSON_TYPE {
	JSON_TYPE_NONE,
	JSON_TYPE_NULL,
	JSON_TYPE_STRING,
	JSON_TYPE_NUMBER,
	JSON_TYPE_BOOLEAN,
	JSON_TYPE_OBJECT,
	JSON_TYPE_ARRAY
} JSON_TYPE;

/*
 * Use the type to interpret the value correctly.
 */
struct json {
	union json_value value;
	enum JSON_TYPE type;
};

/*
 * Creates a [struct json], with its root element being of the specified type.
 * Only types JSON_TYPE_OBJECT and JSON_TYPE_ARRAY are returned because
 * no other can be nested or would be simpler to create with this function
 * (use the macros defined at the bottom of this header instead).
 */
struct json json_build(JSON_TYPE type, ...);

void json_free(struct json value);

/*
 * Serialises a [struct json] (converts it to string representation).
 * NULL is returned if a value of type JSON_TYPE_NONE is passed.
 */
char *json_serialise(struct json value);
#define json_serialize(value) json_serialise(value)

/*
 * Parses a JSON string and converts it into a [struct json].
 * If 'out_error_position' is set to a positive value, an unexpected token
 * was encountered at that position, or, if it equals the size of the string,
 * the JSON string ended unexpectedly.
 */
signed long long json_parsen(const char *json, size_t size, struct json *out);
#define json_parse(json, out) json_parsen(json, (json) ? strlen(json) : 0, out)

signed long long json_parsen2(const char *json, size_t size, struct json *out);

static inline char *json_strndup(const char *str, size_t size)
{
	char *copy = ecalloc(size + 1, sizeof(char));
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
