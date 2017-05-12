/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_TOKEN_H
#define JONSON_TOKEN_H

#include <stddef.h>

typedef enum JSON_TOKEN {
	JSON_TOKEN_NONE            = 0x0001,
	JSON_TOKEN_BEGIN_ARRAY     = 0x0002,
	JSON_TOKEN_BEGIN_OBJECT    = 0x0004,
	JSON_TOKEN_END_ARRAY       = 0x0008,
	JSON_TOKEN_END_OBJECT      = 0x0010,
	JSON_TOKEN_NAME_SEPARATOR  = 0x0020,
	JSON_TOKEN_VALUE_SEPARATOR = 0x0040,
	JSON_TOKEN_STRING          = 0x0100,
	JSON_TOKEN_NUMBER          = 0x0200,
	JSON_TOKEN_NULL            = 0x0400,
	JSON_TOKEN_FALSE           = 0x0800,
	JSON_TOKEN_TRUE            = 0x1000,
	JSON_TOKEN_END             = 0x2000
} JSON_TOKEN;

/*
 * The 'type' and the 'size' always describe the token at 'positon'
 * when used with the function json_next_token().
 */
struct json_token {
	enum JSON_TOKEN type;
	const char *json;
	size_t json_size;
	size_t position;
	size_t size;
};

/*
 * Initialises/resets a [struct json_token].
 */
void json_token_initn(struct json_token *token,
		      const char *json, size_t json_size);
#define json_token_init(token, json) \
	json_token_initn(token, json, (json) ? strlen(json) : 0)

/*
 * Updates a [struct json_token] to point to the next token in its JSON string.
 * Strings are interpreted as single tokens.
 * A non-zero return value indicates success, a zero failure.
 * If an error occured, 'position' in [struct json_token] is set
 * accordingly (as described in json_parsen()).
 */
int json_next_token(struct json_token *token);

#endif /* JONSON_TOKEN_H */
