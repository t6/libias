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

#include <assert.h>
#include <stdlib.h>

#include "stack.h"
#include "util.h"

struct Stack {
	void **buf;
	size_t cap;
	size_t len;
};

struct StackIterator {
	struct Stack *stack;
	size_t i;
};

static const size_t INITIAL_STACK_CAP = 16;

struct Stack *
stack_new()
{
	struct Stack *stack = xmalloc(sizeof(struct Stack));
	stack->cap = INITIAL_STACK_CAP;
	stack->len = 0;
	stack->buf = xrecallocarray(NULL, 0, stack->cap, sizeof(void *));
	return stack;
}

void
stack_free(struct Stack *stack)
{
	if (stack == NULL) {
		return;
	}

	free(stack->buf);
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
	if (stack->len > 0) {
		for (size_t i = stack->len - 1; ; i--) {
			if (value == stack->buf[i]) {
				return 1;
			}
			if (i == 0) {
				break;
			}
		}
	}
	return 0;
}

void *
stack_peek(struct Stack *stack)
{
	if (stack->len > 0) {
		return stack->buf[stack->len - 1];
	} else {
		return NULL;
	}
}

void *
stack_pop(struct Stack *stack)
{
	if (stack->len > 0) {
		void *v = stack->buf[stack->len - 1];
		stack->len--;
		return v;
	} else {
		return NULL;
	}
}

void
stack_push(struct Stack *stack, const void *value)
{
	assert(stack->cap > 0);
	assert(stack->cap > stack->len);

	stack->buf[stack->len++] = (void *)value;
	if (stack->len >= stack->cap) {
		size_t new_cap = stack->cap + INITIAL_STACK_CAP;
		assert(new_cap > stack->cap);
		void **new_buf = xrecallocarray(stack->buf, stack->cap, new_cap, sizeof(void *));
		stack->buf = new_buf;
		stack->cap = new_cap;
	}
}

void
stack_truncate(struct Stack *stack)
{
	stack->len = 0;
}

struct StackIterator *
stack_iterator(struct Stack *stack)
{
	struct StackIterator *iter = xmalloc(sizeof(struct StackIterator));
	iter->i = 0;
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
	if (iter->i < iter->stack->len) {
		*index = iter->i;
		return iter->stack->buf[iter->stack->len - iter->i++ - 1];
	} else {
		stack_iterator_free(iter_);
		*iter_ = NULL;
		return NULL;
	}
}
