/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>

#include "stack.h"
#include "ealloc.h"

void json_stack_free(struct json_stack *stack, int plates)
{
	struct json_stack_node *current = stack->top;
	while (current) {
		if (plates)
			json_free(current->data);
		struct json_stack_node *next = current->next;
		free(current);
		current = next;
	}
	free(stack);
}

void json_stack_push(struct json_stack *stack, struct json value)
{
	struct json_stack_node *plate =
		emalloc(1, sizeof(struct json_stack_node));
	plate->data = value;
	plate->next = stack->top;
	stack->top = plate;
}

struct json json_stack_pop(struct json_stack *stack)
{
	if (!stack->top)
		return JSON_NONE;
	struct json_stack_node *top = stack->top;
	struct json data = top->data;
	stack->top = top->next;
	free(top);
	return data;
}

int json_stack_end_array(struct json_stack *stack)
{
	if (JSON_STACK_SEQUENCE_AV(stack->top)) {
		struct json value = json_stack_pop(stack);
		struct json_array *array = JSON_ARRVAL(stack->top->data);
		json_array_add(array, value);
		stack->top->ready = 1;
		return 1;
	}
	return 0;
}

int json_stack_end_object(struct json_stack *stack)
{
	if (JSON_STACK_SEQUENCE_OKV(stack->top)) {
		struct json value = json_stack_pop(stack);
		char *key = JSON_STRVAL(json_stack_pop(stack));
		struct json_object *object = JSON_OBJVAL(stack->top->data);
		json_object_set(object, key, value);
		stack->top->ready = 1;
		free(key);
		return 1;
	}
	return 0;
}
