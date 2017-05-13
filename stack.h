/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JSON_STACK_H
#define JSON_STACK_H

struct json_stack {
	int ready;
	struct json data;
	struct json_stack *next;
};

static inline struct json_stack *json_stack_new()
{
	return NULL;
}

void json_stack_free(struct json_stack *stack, int free_plates);

void json_stack_push(struct json_stack **stack_ptr, struct json value);

struct json json_stack_pop(struct json_stack **stack_ptr);

#endif /* STACK_H */
