/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_STREAM_H
#define JONSON_STREAM_H

#include "jonson.h"
#include "strbuffer.h"
#include "stack.h"
#include "chain/chain.h"
#include "token.h"

enum JSON_STREAM_STATE {
	JSONS_STR_SEQ     = 0x001, /* String sequence */
	JSONS_STR_ESC_SEQ = 0x002, /* String escape sequence */
	JSONS_NUM_SEQ     = 0x004, /* Number sequence */
	JSONS_NUM_WAS_EXP = 0x008, /* Last character was an E or e */
	JSONS_NUM_HAS_EXP = 0x010, /* Number contains an E or e */
	JSONS_NUM_HAS_DOT = 0x020, /* Number contains a decimal point */
	JSONS_NUM_NEG     = 0x040, /* Number is negative */
	JSONS_NUM_EXP_NEG = 0x080, /* Number's exponent is negative */
	JSONS_TRUE_SEQ    = 0x100, /* True sequence */
	JSONS_FALSE_SEQ   = 0x200, /* False sequence */
	JSONS_NULL_SEQ    = 0x400  /* Null sequence */
};

struct json_stream {
	unsigned int state;
	struct chain *chain;
	struct json_stack *stack;
	struct json_token token;
	struct {
		double value;
		double fraction; /* < REMOVE THIS! */
		unsigned int exponent;
		unsigned int precision;
	} number;
};

struct json_stream *json_stream_new(void);
void json_stream_free(struct json_stream *stream);
int json_stream_write_n(struct json_stream *stream,
			 const char *chunk, size_t size);
#define json_stream_write(stream, chunk) \
	json_stream_write_n(stream, chunk, (chunk) ? strlen(chunk) : 0)

#endif /* JONSON_STREAM_H */
