/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stream.h"

static double precision_table[17] = {
	[1] = 1e-1, [2] = 1e-2, [3] = 1e-3,
	[4] = 1e-4, [5] = 1e-5, [6] = 1e-6,
	[7] = 1e-7, [8] = 1e-8, [9] = 1e-9,
	[10] = 1e-10, [11] = 1e-11, [12] = 1e-12,
	[13] = 1e-13, [14] = 1e-14, [15] = 1e-15,
	[16] = 1e-16
};

struct json_stream *json_stream_new(void)
{
	struct json_stream *stream = ecalloc(1, sizeof(struct json_stream));
	json_token_init(&stream->token);
	stream->chain = chain_new();
	stream->stack = json_stack_new();
	return stream;
}

void json_stream_free(struct json_stream *stream)
{
	chain_free(stream->chain);
	json_stack_free(stream->stack, 1);
	free(stream);
}

int json_stream_write_n(struct json_stream *stream,
			const char *chunk, size_t size)
{
	chain_append_n(stream->chain, chunk, size);

	for (size_t i = 0; i < size; ++i)
	{
		char c = chunk[i];

		if (stream->state & JSONS_STR_SEQ) {
			if (stream->state & JSONS_STR_ESC_SEQ) {
				/* Last problem:
				 * The passed chunk is not editable.
				 * If it was, there's still the problem with
				 * replacing two characters with one ...
				 * The only reasonable solution to this would
				 * be replacing a sequence inside the chain.
				 */
				stream->state &= ~JSONS_STR_ESC_SEQ;
				switch (c) {
				case '\\':
				case '"':
				case '/': break;
				case 'b': c = '\b'; break;
				case 'f': c = '\f'; break;
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 't': c = '\t'; break;
				default:
					goto unexpected_token;
				}
				++stream->token.size;
				goto success;
			}
			if (c == '\\') {
				stream->state |= JSONS_STR_ESC_SEQ;
				goto success;
			}
			++stream->token.size;
			if (c == '"') {
				size_t position = stream->token.position + 1;
				size_t size = stream->token.size - 2;
				char *str = chain_report(stream->chain, position, size);
				json_stack_push(stream->stack, (struct json){
					.type = JSON_TYPE_STRING,
					.value.string = str
				});
				stream->stack->top->ready = 1;
				stream->state &= ~JSONS_STR_SEQ;
			}
			goto success;
		}

		if (stream->state & JSONS_NUM_SEQ) {
			if (stream->state & JSONS_NUM_WAS_EXP) {
				stream->state &= ~JSONS_NUM_WAS_EXP;
				if (c == TOKEN_MINUS)
					stream->state |= JSONS_NUM_EXP_NEG;
				if (c == TOKEN_PLUS || c == TOKEN_MINUS) {
					++stream->token.size;
					goto success;
				}
			}
			if (c >= '0' && c <= '9') {
				int digit = c - '0';
				if (stream->state & JSONS_NUM_HAS_EXP) {
					stream->number.exponent *= 10;
					stream->number.exponent += digit;
				}
				else if (stream->state & JSONS_NUM_HAS_DOT) {
					if (stream->number.precision < 16)
						stream->number.value +=
							precision_table[++stream->number.precision] * digit;
				}
				else {
					stream->number.value *= 10;
					stream->number.value += digit;
				}
				++stream->token.size;
				goto success;
			}
			if (!(stream->state & JSONS_NUM_HAS_EXP) &&
					(c == TOKEN_ELOWER || c == TOKEN_EUPPER)) {
				stream->state |= (JSONS_NUM_HAS_EXP | JSONS_NUM_WAS_EXP);
				++stream->token.size;
				goto success;
			}
			if (!(stream->state & JSONS_NUM_HAS_DOT) &&
					c == TOKEN_DECIMAL_POINT) {
				stream->state |= JSONS_NUM_HAS_DOT;
				++stream->token.size;
				goto success;
			}

			if (stream->number.exponent) {
				double factor = 1.0;
				for (unsigned int i = 0; i < stream->number.exponent; ++i)
					factor *= 10;
				if (stream->state & JSONS_NUM_EXP_NEG)
					factor = 1 / factor;
				stream->number.value *= factor;
			}
			if (stream->state & JSONS_NUM_NEG)
				stream->number.value *= -1.0;

			json_stack_push(stream->stack, JSON_NUM(stream->number.value));
			stream->stack->top->ready = 1;

			stream->number.value = 0.0;
			stream->number.exponent = 0;
			stream->number.precision = 0;

			stream->state &= ~(JSONS_NUM_SEQ |
					   JSONS_NUM_HAS_EXP |
					   JSONS_NUM_HAS_DOT |
					   JSONS_NUM_NEG |
					   JSONS_NUM_EXP_NEG);
		}

		if (stream->state & JSONS_TRUE_SEQ) {
			if (c != TOKEN_TRUE[stream->token.size])
				goto unexpected_token;
			if (stream->token.size >= TOKEN_TRUE_SIZE - 1) {
				json_stack_push(stream->stack, JSON_BOOL(1));
				stream->stack->top->ready = 1;
				stream->state &= ~JSONS_TRUE_SEQ;
			}
			++stream->token.size;
			goto success;
		}

		if (stream->state & JSONS_FALSE_SEQ) {
			if (c != TOKEN_FALSE[stream->token.size])
				goto unexpected_token;
			if (stream->token.size >= TOKEN_FALSE_SIZE - 1) {
				json_stack_push(stream->stack, JSON_BOOL(0));
				stream->stack->top->ready = 1;
				stream->state &= ~JSONS_FALSE_SEQ;
			}
			++stream->token.size;
			goto success;
		}

		if (stream->state & JSONS_NULL_SEQ) {
			if (c != TOKEN_NULL[stream->token.size])
				goto unexpected_token;
			if (stream->token.size >= TOKEN_NULL_SIZE - 1) {
				json_stack_push(stream->stack, JSON_NULL);
				stream->stack->top->ready = 1;
				stream->state &= ~JSONS_NULL_SEQ;
			}
			++stream->token.size;
			goto success;
		}

		enum JSON_TOKEN last_token = stream->token.type;
		stream->token.position += stream->token.size;

		switch (c) {
		case TOKEN_SPACE:
		case TOKEN_CARRIAGE_RETURN:
		case TOKEN_LINE_FEED:
		case TOKEN_HORIZONTAL_TAB:
			/* Whitespace is not buffered. */
			stream->token.size = 1;
			goto success;
		case TOKEN_END:
			stream->token.type = JSON_TOKEN_END;
			stream->token.size = 1;
			/* It's not really necessary to check here, since both
			   branches return 0. Should be done once the result or
			   an error message is requested. */
			struct json_stack_node *top = stream->stack->top;
			if (!top || !top->ready || top->next)
				goto unexpected_end_of_input;
			goto end_of_input;
		case TOKEN_BEGIN_ARRAY:
			if (last_token & ~(JSON_TOKEN_BEGIN |
					   JSON_TOKEN_BEGIN_ARRAY |
					   JSON_TOKEN_NAME_SEPARATOR |
					   JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			stream->token.type = JSON_TOKEN_BEGIN_ARRAY;
			stream->token.size = 1;
			json_stack_push(stream->stack, JSON_ARR(json_array_new()));
			goto success;
		case TOKEN_END_ARRAY:
			if (last_token & (JSON_TOKEN_BEGIN |
					  JSON_TOKEN_END |
					  JSON_TOKEN_BEGIN_OBJECT |
					  JSON_TOKEN_NAME_SEPARATOR |
					  JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			stream->token.type = JSON_TOKEN_END_ARRAY;
			stream->token.size = 1;
			json_stack_end_array(stream->stack);
			goto success;
		case TOKEN_BEGIN_OBJECT:
			if (last_token & ~(JSON_TOKEN_BEGIN |
					   JSON_TOKEN_BEGIN_ARRAY |
					   JSON_TOKEN_NAME_SEPARATOR |
					   JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			stream->token.type = JSON_TOKEN_BEGIN_OBJECT;
			stream->token.size = 1;
			json_stack_push(stream->stack, JSON_OBJ(json_object_new()));
			goto success;
		case TOKEN_END_OBJECT:
			if (last_token & (JSON_TOKEN_BEGIN |
					  JSON_TOKEN_END |
					  JSON_TOKEN_BEGIN_ARRAY |
					  JSON_TOKEN_NAME_SEPARATOR |
					  JSON_TOKEN_VALUE_SEPARATOR/* |
					  JSON_TOKEN_NAME)*/))
				goto unexpected_token;
			stream->token.type = JSON_TOKEN_END_OBJECT;
			stream->token.size = 1;
			json_stack_end_object(stream->stack);
			goto success;
		case TOKEN_VALUE_SEPARATOR:
			if (last_token & (JSON_TOKEN_BEGIN |
					  JSON_TOKEN_END |
					  JSON_TOKEN_BEGIN_ARRAY |
					  JSON_TOKEN_BEGIN_OBJECT |
					  JSON_TOKEN_NAME_SEPARATOR |
					  JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			stream->token.type = JSON_TOKEN_VALUE_SEPARATOR;
			stream->token.size = 1;
			if (json_stack_end_array(stream->stack) ||
					json_stack_end_object(stream->stack))
				stream->stack->top->ready = 0;
			goto success;
		case TOKEN_NAME_SEPARATOR:
			/* Use JSON_TOKEN_NAME here ... */
			if (last_token & ~(JSON_TOKEN_STRING))
				goto unexpected_token;
			if (!JSON_STACK_SEQUENCE_OK(stream->stack->top))
				goto unexpected_token;
			stream->token.type = JSON_TOKEN_NAME_SEPARATOR;
			stream->token.size = 1;
			goto success;
		case TOKEN_QUOTATION_MARK:
			stream->token.type = JSON_TOKEN_STRING;
			stream->token.size = 1;
			if (last_token & ~(JSON_TOKEN_BEGIN |
					   JSON_TOKEN_BEGIN_ARRAY |
					   JSON_TOKEN_BEGIN_OBJECT |
					   JSON_TOKEN_NAME_SEPARATOR |
					   JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			stream->state |= JSONS_STR_SEQ;
			goto success;
		case 't':
			stream->state |= JSONS_TRUE_SEQ;
			stream->token.type = JSON_TOKEN_TRUE;
			stream->token.size = 1;
			goto success;
		case 'f':
			stream->state |= JSONS_FALSE_SEQ;
			stream->token.type = JSON_TOKEN_FALSE;
			stream->token.size = 1;
			goto success;
		case 'n':
			stream->state |= JSONS_NULL_SEQ;
			stream->token.type = JSON_TOKEN_NULL;
			stream->token.size = 1;
			goto success;
		default:
			if (c == TOKEN_MINUS || (c >= '0' && c <= '9')) {
				if (last_token & ~(JSON_TOKEN_BEGIN |
						   JSON_TOKEN_BEGIN_ARRAY |
						   JSON_TOKEN_NAME_SEPARATOR |
						   JSON_TOKEN_VALUE_SEPARATOR))
					goto unexpected_token;
				stream->state |= JSONS_NUM_SEQ;
				stream->token.type = JSON_TOKEN_NUMBER;
				stream->token.size = 1;
				if (c == TOKEN_MINUS)
					stream->state |= JSONS_NUM_NEG;
				else
					stream->number.value += (c - '0');
				goto success;
			}
			goto unexpected_token;
		}

	success:
		continue;
	}

	return 1;

unexpected_token:
unexpected_end_of_input:
	printf("Error in JSON string.\n");
end_of_input:
	return 0;
}
