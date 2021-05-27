/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2021 Tobias Kortkamp <tobik@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <stdlib.h>

#include "stack.h"
#include "util.h"

struct StackNode {
	struct StackNode *next;
	void *value;
};

struct Stack {
	size_t len;
	struct StackNode *head;
};

struct StackIterator {
	size_t i;
	struct Stack *stack;
	struct StackNode *node;
};

struct Stack *
stack_new()
{
	struct Stack *stack = xmalloc(sizeof(struct Stack));
	return stack;
}

void
stack_free(struct Stack *stack)
{
	if (stack == NULL) {
		return;
	}

	struct StackNode *node = stack->head;
	while (node) {
		struct StackNode *next = node->next;
		free(node);
		node = next;
	}
	free(stack);
}

size_t
stack_len(struct Stack *stack)
{
	return stack->len;
}

int
stack_contains(struct Stack *stack, const void *value)
{
	for (struct StackNode *node = stack->head; node; node = node->next) {
		if (value == node->value) {
			return 1;
		}
	}
	return 0;
}

void *
stack_peek(struct Stack *stack)
{
	if (stack->head) {
		return stack->head->value;
	} else {
		return NULL;
	}
}

void *
stack_pop(struct Stack *stack)
{
	if (stack->head) {
		struct StackNode *next = stack->head->next;
		void *value = stack->head->value;
		free(stack->head);
		stack->head = next;
		stack->len--;
		return value;
	} else {
		return NULL;
	}
}

void
stack_push(struct Stack *stack, const void *value)
{
	struct StackNode *node = xmalloc(sizeof(struct StackNode));
	node->value = (void *)value;
	node->next = stack->head;
	stack->head = node;
	stack->len++;
}

void
stack_truncate(struct Stack *stack)
{
	struct StackNode *node = stack->head;
	while (node) {
		struct StackNode *next = node->next;
		free(node);
		node = next;
	}
	stack->head = NULL;
	stack->len = 0;
}

struct StackIterator *
stack_iterator(struct Stack *stack)
{
	struct StackIterator *iter = xmalloc(sizeof(struct StackIterator));
	iter->node = stack->head;
	iter->stack = stack;
	return iter;
}

void
stack_iterator_free(struct StackIterator **iter_)
{
	struct StackIterator *iter = *iter_;
	if (iter != NULL) {
		free(iter);
		*iter_ = NULL;
	}
}

void *
stack_iterator_next(struct StackIterator **iter_, size_t *index)
{
	struct StackIterator *iter = *iter_;
	if (iter->node) {
		*index = iter->i;
		void *value = iter->node->value;
		iter->node = iter->node->next;
		iter->i++;
		return value;
	} else {
		stack_iterator_free(iter_);
		*iter_ = NULL;
		return NULL;
	}
}
