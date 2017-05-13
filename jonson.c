/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <stdio.h>

#include "jonson.h"
#include "object.h"
#include "array.h"
#include "token.h"
#include "strbuffer.h"
#include "stack.h"

struct json json_build(JSON_TYPE type, ...)
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
		result = json_array_new();
		while (1) {
			struct json value = va_arg(args, struct json);
			if (value.type == JSON_TYPE_NONE)
				break;
			json_array_add(result, value);
		}
	}

	va_end(args);
	return result;
}

void json_free(struct json value)
{
	switch (value.type) {
	case JSON_TYPE_STRING: free(JSON_STRVAL(value)); return;
	case JSON_TYPE_OBJECT: json_object_free(JSON_OBJVAL(value)); return;
	case JSON_TYPE_ARRAY:  json_array_free(value); return;
	default: return;
	}
}

char *json_serialise(struct json value)
{
	strbuffer_t *sb = strbuffer_new();

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

		struct json_bucket_list *current = object ? object->list : NULL;
		while (current) {
			struct json_bucket *bucket = current->data;

			char *value_ = json_serialise(bucket->value);
			char *key = json_serialise((struct json){
				.type = JSON_TYPE_STRING,
				.value.string = bucket->key
			});

			strbuffer_append(sb, key);
			strbuffer_append_char(sb, ':');
			strbuffer_append(sb, value_);

			free(key);
			free(value_);

			if ((current = current->next))
				strbuffer_append_char(sb, ',');
		}

		strbuffer_append_char(sb, '}');
		break;
	}
	case JSON_TYPE_ARRAY: {
		struct json_array *array = JSON_ARRVAL(value);
		strbuffer_append_char(sb, '[');

		size_t i = 0;
		while (i < array->size) {
			char *value_ = json_serialise(json_array_get(value, i));
			strbuffer_append(sb, value_);
			free(value_);

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

signed long long json_parsen(const char *json, size_t size, struct json *out)
{
	struct json_token token;
	json_token_initn(&token, json, size);
	struct json_stack *stack = json_stack_new();
	struct json result = JSON_NONE;

	while (json_next_token(&token)) {
		switch (token.type) {
		case JSON_TOKEN_STRING: {
			size_t size = token.size - 2;
			const char *str = token.json + token.position + 1;
			const char *end = str + size;

			char *value = ecalloc(size + 1, sizeof(char));
			char *current = value;

			while (str != end) {
				if (*str == '\\' && str + 1 != end) {
					switch (*(str + 1)) {
					case '\\': *current++ = '\\'; break;
					case '"':  *current++ = '"';  break;
					case '/':  *current++ = '/';  break;
					case 'b':  *current++ = '\b'; break;
					case 'f':  *current++ = '\f'; break;
					case 'n':  *current++ = '\n'; break;
					case 'r':  *current++ = '\r'; break;
					case 't':  *current++ = '\t'; break;
					default:
						token.position = (str - token.json) + 1;
						goto unexpected_token;
					}
					++str;
					continue;
				}
				*current++ = *str++;
			}

			json_stack_push(&stack, (struct json){
				.type = JSON_TYPE_STRING,
				.value.string = value
			});

			stack->ready = 1;
			continue;
		}
		case JSON_TOKEN_NUMBER: {
			if (stack && stack->data.type == JSON_TYPE_OBJECT)
				goto unexpected_token;
			double num = strtod(token.json + token.position, NULL);

			json_stack_push(&stack, JSON_NUM(num));
			stack->ready = 1;
			continue;
		}
		case JSON_TOKEN_NULL: {
			if (stack && stack->data.type == JSON_TYPE_OBJECT)
				goto unexpected_token;

			json_stack_push(&stack, JSON_NULL);
			stack->ready = 1;
			continue;
		}
		case JSON_TOKEN_FALSE:
		case JSON_TOKEN_TRUE: {
			if (stack && stack->data.type == JSON_TYPE_OBJECT)
				goto unexpected_token;

			json_stack_push(&stack,
				JSON_BOOL(token.type == JSON_TOKEN_TRUE));
			stack->ready = 1;
			continue;
		}
		case JSON_TOKEN_NAME_SEPARATOR:
			if (!stack || stack->data.type != JSON_TYPE_STRING ||
					!stack->next || stack->next->data.type != JSON_TYPE_OBJECT)
				goto unexpected_token;
			continue;
		case JSON_TOKEN_BEGIN_OBJECT:
			json_stack_push(&stack, JSON_OBJ(json_object_new()));
			continue;
		case JSON_TOKEN_BEGIN_ARRAY:
			json_stack_push(&stack, json_array_new());
			continue;
		case JSON_TOKEN_VALUE_SEPARATOR:
		case JSON_TOKEN_END_OBJECT:
		case JSON_TOKEN_END_ARRAY:
			if (token.type != JSON_TOKEN_END_OBJECT &&
					stack && stack->next &&
					stack->next->data.type == JSON_TYPE_ARRAY) {
				struct json value = json_stack_pop(&stack);
				json_array_add(stack->data, value);
			}
			else if (token.type != JSON_TOKEN_END_ARRAY &&
					stack && stack->next && stack->next->next &&
					stack->next->data.type == JSON_TYPE_STRING &&
					stack->next->next->data.type == JSON_TYPE_OBJECT) {
				struct json value = json_stack_pop(&stack);
				struct json key = json_stack_pop(&stack);
				json_object_set(JSON_OBJVAL(stack->data),
					JSON_STRVAL(key), value);
				free(JSON_STRVAL(key));
			}
			if (!stack || (stack->data.type != JSON_TYPE_OBJECT &&
				       stack->data.type != JSON_TYPE_ARRAY) ||
					stack->ready)
				goto unexpected_token;
			if (token.type != JSON_TOKEN_VALUE_SEPARATOR)
				stack->ready = 1;
			continue;
		case JSON_TOKEN_END:
			if (!stack || stack->next || !stack->ready)
				goto unexpected_end_of_input;
			result = json_stack_pop(&stack);
			goto success;
		default: goto unexpected_token;
		}
	}

unexpected_token:
unexpected_end_of_input:
	json_free(result);
	json_stack_free(stack, 1);

	if (out)
		*out = JSON_NONE;
	return token.position;

success:
	json_stack_free(stack, 0);

	if (out)
		*out = result;
	return -1;
}
