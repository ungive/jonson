/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stddef.h>
#include <string.h>

#include "jonson.h"
#include "token.h"

#define TOKEN_NULL            "null"
#define TOKEN_TRUE            "true"
#define TOKEN_FALSE           "false"
#define TOKEN_BEGIN_ARRAY     '['
#define TOKEN_BEGIN_OBJECT    '{'
#define TOKEN_END_ARRAY       ']'
#define TOKEN_END_OBJECT      '}'
#define TOKEN_NAME_SEPARATOR  ':'
#define TOKEN_VALUE_SEPARATOR ','
#define TOKEN_QUOTATION_MARK  '"'
#define TOKEN_SPACE           ' '
#define TOKEN_HORIZONTAL_TAB  '\t'
#define TOKEN_LINE_FEED       '\n'
#define TOKEN_CARRIAGE_RETURN '\r'
#define TOKEN_END             '\0'

#define TOKEN_PLUS            '+'
#define TOKEN_MINUS           '-'
#define TOKEN_DECIMAL_POINT   '.'
#define TOKEN_EXP_LOWER       'e'
#define TOKEN_EXP_UPPER       'E'

void json_token_initn(struct json_token *token,
		      const char *json, size_t json_size)
{
	memset(token, 0, sizeof(struct json_token));
	token->type = JSON_TOKEN_NONE;
	token->json = json;
	token->json_size = json_size;
	token->position = 0;
	token->size = 0;
}

static size_t json_number_size(const char *str, size_t size)
{
	const char *current = str;
	const char *exponent = NULL;
	const char *decimal_point = NULL;

	for (; (unsigned)(current - str) < size; ++current) {
		if (*current >= '0' && *current <= '9')
			continue;
		if (!exponent && (*current == TOKEN_EXP_LOWER ||
				  *current == TOKEN_EXP_UPPER)) {
			exponent = current;
			continue;
		}
		if (*current == TOKEN_MINUS &&
				(current == str || current - 1 == exponent))
			continue;
		if (*current == TOKEN_PLUS &&
				current != str && current - 1 == exponent)
			continue;
		if (!decimal_point && *current == TOKEN_DECIMAL_POINT) {
			decimal_point = current;
			continue;
		}
		break;
	}

	return current - str;
}

int json_next_token(struct json_token *token)
{
	if (token->position >= token->json_size)
		return 0;

	token->position += token->size;
	const char *start = token->json + token->position;
	const char *current = start;

	do {
		if ((unsigned)(current - token->json) >= token->json_size) {
			token->type = JSON_TOKEN_END;
			token->size = 1;
			goto success;
		}

		switch (*current) {
		case TOKEN_SPACE:
		case TOKEN_HORIZONTAL_TAB:
		case TOKEN_LINE_FEED:
		case TOKEN_CARRIAGE_RETURN:
			++token->position;
			++start;
			break;
		case TOKEN_BEGIN_ARRAY:
			if (token->type & ~(JSON_TOKEN_NONE |
					    JSON_TOKEN_BEGIN_ARRAY |
					    JSON_TOKEN_NAME_SEPARATOR |
					    JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			token->type = JSON_TOKEN_BEGIN_ARRAY;
			token->size = 1;
			goto success;
		case TOKEN_END_ARRAY:
			if (token->type & (JSON_TOKEN_NONE |
					   JSON_TOKEN_BEGIN_OBJECT |
					   JSON_TOKEN_NAME_SEPARATOR |
					   JSON_TOKEN_VALUE_SEPARATOR |
					   JSON_TOKEN_END))
				goto unexpected_token;
			token->type = JSON_TOKEN_END_ARRAY;
			token->size = 1;
			goto success;
		case TOKEN_BEGIN_OBJECT:
			if (token->type & ~(JSON_TOKEN_NONE |
					    JSON_TOKEN_BEGIN_ARRAY |
					    JSON_TOKEN_NAME_SEPARATOR |
					    JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			token->type = JSON_TOKEN_BEGIN_OBJECT;
			token->size = 1;
			goto success;
		case TOKEN_END_OBJECT:
			if (token->type & (JSON_TOKEN_NONE |
					   JSON_TOKEN_BEGIN_ARRAY |
					   JSON_TOKEN_NAME_SEPARATOR |
					   JSON_TOKEN_VALUE_SEPARATOR |
					   JSON_TOKEN_END))
				goto unexpected_token;
			token->type = JSON_TOKEN_END_OBJECT;
			token->size = 1;
			goto success;
		case TOKEN_NAME_SEPARATOR:
			if (token->type & ~JSON_TOKEN_STRING)
				goto unexpected_token;
			token->type = JSON_TOKEN_NAME_SEPARATOR;
			token->size = 1;
			goto success;
		case TOKEN_VALUE_SEPARATOR:
			if (token->type & (JSON_TOKEN_BEGIN_ARRAY |
					   JSON_TOKEN_BEGIN_OBJECT |
					   JSON_TOKEN_NAME_SEPARATOR |
					   JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;
			token->type = JSON_TOKEN_VALUE_SEPARATOR;
			token->size = 1;
			goto success;
		case TOKEN_QUOTATION_MARK: {
			if (token->type & ~(JSON_TOKEN_NONE |
					    JSON_TOKEN_BEGIN_ARRAY |
					    JSON_TOKEN_BEGIN_OBJECT |
					    JSON_TOKEN_NAME_SEPARATOR |
					    JSON_TOKEN_VALUE_SEPARATOR))
				goto unexpected_token;

			token->type = JSON_TOKEN_STRING;

			const char *str_start = ++current;
			while ((unsigned)(current - token->json) < token->json_size &&
					(*current != '"' || *(current - 1) == '\\'))
				++current;

			if ((unsigned)(current - token->json) >= token->json_size) {
				++token->position;
				if (str_start == current)
					goto unexpected_end_of_input;
				goto unexpected_token;
			}

			token->size = (current - start) + 1;
			goto success;
		}
		case 'n':
			if (strncmp(current, TOKEN_NULL, 4) != 0)
				goto unexpected_token;
			token->type = JSON_TOKEN_NULL;
			token->size = 4;
			goto success;
		case 't':
			if (strncmp(current, TOKEN_TRUE, 4) != 0)
				goto unexpected_token;
			token->type = JSON_TOKEN_TRUE;
			token->size = 4;
			goto success;
		case 'f':
			if (strncmp(current, TOKEN_FALSE, 5) != 0)
				goto unexpected_token;
			token->type = JSON_TOKEN_FALSE;
			token->size = 5;
			goto success;
		case TOKEN_END:
			token->type = JSON_TOKEN_END;
			token->size = 1;
			goto success;
		default:
			if (*current == TOKEN_MINUS ||
					(*current >= '0' && *current <= '9')) {
				if (token->type & ~(JSON_TOKEN_NONE |
						    JSON_TOKEN_BEGIN_ARRAY |
						    JSON_TOKEN_NAME_SEPARATOR |
						    JSON_TOKEN_VALUE_SEPARATOR))
					goto unexpected_token;

				token->type = JSON_TOKEN_NUMBER;
				token->size = json_number_size(current,
					token->json_size - (current - token->json));
				current += token->size;

				if (*(current - 1) < '0' ||
						*(current - 1) > '9') {
					token->position += token->size;
					if (*current)
						goto unexpected_token;
					else
						goto unexpected_end_of_input;
				}
				goto success;
			}
			goto unexpected_token;
		}
	}
	while (*current++);

success:
	return 1;

unexpected_token:
unexpected_end_of_input:
	return 0;
}
