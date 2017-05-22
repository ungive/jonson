/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JONSON_TOKEN_H
#define JONSON_TOKEN_H

#include <stddef.h>

/*
 * Rename these macros. To JSON_TOKEN_* e.g.
 */

#define TOKEN_SPACE           ' '
#define TOKEN_CARRIAGE_RETURN '\r'
#define TOKEN_LINE_FEED       '\n'
#define TOKEN_HORIZONTAL_TAB  '\t'

#define TOKEN_END             '\0'
#define TOKEN_BEGIN_ARRAY     '['
#define TOKEN_END_ARRAY       ']'
#define TOKEN_BEGIN_OBJECT    '{'
#define TOKEN_END_OBJECT      '}'
#define TOKEN_NAME_SEPARATOR  ':'
#define TOKEN_VALUE_SEPARATOR ','
#define TOKEN_TRUE            "true"
#define TOKEN_FALSE           "false"
#define TOKEN_NULL            "null"

#define TOKEN_TRUE_SIZE       (sizeof(TOKEN_TRUE) - 1)
#define TOKEN_FALSE_SIZE      (sizeof(TOKEN_FALSE) - 1)
#define TOKEN_NULL_SIZE       (sizeof(TOKEN_NULL) - 1)

#define TOKEN_QUOTATION_MARK  '"'
#define TOKEN_PLUS            '+'
#define TOKEN_MINUS           '-'
#define TOKEN_DECIMAL_POINT   '.'
#define TOKEN_ELOWER          'e'
#define TOKEN_EUPPER          'E'

enum JSON_TOKEN {
	JSON_TOKEN_NONE            = 0x0000,
	JSON_TOKEN_BEGIN           = 0x0001,
	JSON_TOKEN_END             = 0x0002,
	JSON_TOKEN_BEGIN_ARRAY     = 0x0004,
	JSON_TOKEN_END_ARRAY       = 0x0008,
	JSON_TOKEN_BEGIN_OBJECT    = 0x0010,
	JSON_TOKEN_END_OBJECT      = 0x0020,
	JSON_TOKEN_NAME_SEPARATOR  = 0x0040,
	JSON_TOKEN_VALUE_SEPARATOR = 0x0080,
	JSON_TOKEN_STRING          = 0x0100,
	JSON_TOKEN_NUMBER          = 0x0200,
	JSON_TOKEN_TRUE            = 0x0400,
	JSON_TOKEN_FALSE           = 0x0800,
	JSON_TOKEN_NULL            = 0x1000,
	JSON_TOKEN_WHITESPACE      = 0x2000
};

struct json_token {
	enum JSON_TOKEN type;
	size_t position;
	size_t size;
};

void json_token_init(struct json_token *token);

#endif /* JONSON_TOKEN_H */
