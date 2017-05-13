/* Copyright (c) 2017 Jonas Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>

#include "jonson.h"
#include "stack.h"
#include "ecalloc.h"

void json_stack_free(struct json_stack *stack, int free_plates)
{
	struct json_stack *current = stack;
	while (current) {
		if (free_plates)
			json_free(current->data);
		struct json_stack *next = current->next;
		free(current);
		current = next;
	}
}

void json_stack_push(struct json_stack **stack_ptr, struct json value)
{
	struct json_stack *plate = ecalloc(1, sizeof(struct json_stack));
	plate->data = value;
	plate->next = *stack_ptr;
	*stack_ptr = plate;
}

struct json json_stack_pop(struct json_stack **stack_ptr)
{
	struct json_stack *stack = *stack_ptr;
	struct json data = stack->data;

	*stack_ptr = stack->next ? stack->next : NULL;
	free(stack);

	return data;
}
