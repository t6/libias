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

#include "map.h"
#include "mempool.h"
#include "stack.h"
#include "util.h"

struct Mempool {
	struct Stack *stack;
	struct Map *map;
};

struct Mempool *
mempool_new()
{
	struct Mempool *pool = xmalloc(sizeof(struct Mempool));
	pool->stack = stack_new();
	return pool;
}

struct Mempool *
mempool_new_unique()
{
	struct Mempool *pool = xmalloc(sizeof(struct Mempool));
	pool->map = map_new(NULL, NULL, NULL, NULL);
	return pool;
}

void
mempool_free(struct Mempool *pool)
{
	if (pool == NULL) {
		return;
	}
	mempool_release(pool);
	map_free(pool->map);
	stack_free(pool->stack);
	free(pool);
}

void
mempool_cleanup(struct Mempool **pool)
{
	if (pool) {
		mempool_free(*pool);
		*pool = NULL;
	}
}

void *
mempool_add(struct Mempool *pool, void *ptr, void *freefn)
{
	assert(freefn != NULL);
	if (!ptr) {
		return NULL;
	}

	if (pool->map) {
		map_add(pool->map, ptr, freefn);
	} else {
		stack_push(pool->stack, freefn);
		stack_push(pool->stack, ptr);
	}

	return ptr;
}

void
mempool_inherit(struct Mempool *pool, struct Mempool *other)
{
	if (other->map) {
		MAP_FOREACH(other->map, void *, ptr, void *, freefn) {
			mempool_add(pool, ptr, freefn);
		}
		map_truncate(other->map);
	} else {
		void *ptr;
		while ((ptr = stack_pop(other->stack))) {
			void (*freefn)(void *) = stack_pop(other->stack);
			mempool_add(pool, ptr, freefn);
		}
		stack_truncate(other->stack);
	}
}

void
mempool_release(struct Mempool *pool)
{
	if (pool->map) {
		MAP_FOREACH(pool->map, void *, ptr, void *, f) {
			void (*freefn)(void *) = f;
			freefn(ptr);
		}
		map_truncate(pool->map);
	} else {
		void *ptr;
		while ((ptr = stack_pop(pool->stack))) {
			void (*freefn)(void *) = stack_pop(pool->stack);
			freefn(ptr);
		}
	}
}
