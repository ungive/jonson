/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JSON_STACK_H
#define JSON_STACK_H

#include "jonson.h"

struct json_stack_node {
	int ready;
	struct json data;
	struct json_stack_node *next;
};

struct json_stack {
	struct json_stack_node *top;
};

static inline struct json_stack *json_stack_new(void)
{
	return ecalloc(1, sizeof(struct json_stack));
}

void json_stack_free(struct json_stack *stack, int plates);

void json_stack_push(struct json_stack *stack, struct json value);
struct json json_stack_pop(struct json_stack *stack);

int json_stack_end_array(struct json_stack *stack);
int json_stack_end_object(struct json_stack *stack);

/* Array->value sequence */
#define JSON_STACK_SEQUENCE_AV(node) \
	((node) && (node)->next && (node)->next->data.type == JSON_TYPE_ARRAY)

/* Object->key sequence */
#define JSON_STACK_SEQUENCE_OK(node) \
	((node) && (node)->next && (node)->data.type == JSON_TYPE_STRING && \
		(node)->next->data.type == JSON_TYPE_OBJECT)

/* Object->key->value sequence */
#define JSON_STACK_SEQUENCE_OKV(node) \
	((node) && JSON_STACK_SEQUENCE_OK((node)->next))

#endif /* JSON_STACK_H */
