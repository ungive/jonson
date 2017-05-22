/* Copyright (c) 2017 Jonas van den Berg <jonas.vanen@gmail.com>
 * 
 * Jonson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "token.h"

void json_token_init(struct json_token *token)
{
	token->type = JSON_TOKEN_BEGIN;
	token->position = 0;
	token->size = 0;
}
