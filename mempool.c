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

#include "array.h"
#include "map.h"
#include "mempool.h"
#include "queue.h"
#include "set.h"
#include "stack.h"
#include "util.h"

struct Mempool {
	struct Stack *stack;
	struct Map *map;
	struct Mempool *owner;
};

struct Mempool *
mempool_new()
{
	struct Mempool *pool = xmalloc(sizeof(struct Mempool));
	pool->stack = stack_new();
	pool->owner = pool;
	return pool;
}

struct Mempool *
mempool_new_unique()
{
	struct Mempool *pool = xmalloc(sizeof(struct Mempool));
	pool->map = map_new(NULL, NULL, NULL, NULL);
	pool->owner = pool;
	return pool;
}

void
mempool_free(struct Mempool *pool)
{
	if (pool == NULL) {
		return;
	}
	if (pool->owner != pool) {
		abort();
	}
	mempool_release(pool);
	map_free(pool->map);
	stack_free(pool->stack);
	free(pool);
}

static void
mempool_free_owned(struct Mempool *pool)
{
	pool->owner = pool;
	mempool_free(pool);
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
	if (!ptr || !freefn) {
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
	if (other && pool != other && pool->owner != other && other->owner != pool) {
		mempool_add(pool, other, mempool_free_owned);
		other->owner = pool;
	}
}

void
mempool_release(struct Mempool *pool)
{
	if (pool->owner != pool) {
		abort();
	} else if (pool->map) {
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

void *
mempool_alloc(struct Mempool *pool, size_t sz)
{
	return mempool_add(pool, xmalloc(sz), free);
}

void *
mempool_take(struct Mempool *pool, void *ptr)
{
	return mempool_add(pool, ptr, free);
}

struct Array *
mempool_array(struct Mempool *pool)
{
	return mempool_add(pool, array_new(), array_free);
}

struct Map *
mempool_map(struct Mempool *pool, MempoolCompareFn compare, void *compare_userdata, void *keyfree, void *valuefree)
{
	return mempool_add(pool, map_new(compare, compare_userdata, keyfree, valuefree), map_free);
}

struct Queue *
mempool_queue(struct Mempool *pool)
{
	return mempool_add(pool, queue_new(), queue_free);
}

struct Set *
mempool_set(struct Mempool *pool, MempoolCompareFn compare, void *compare_userdata, void *keyfree)
{
	return mempool_add(pool, set_new(compare, compare_userdata, keyfree), set_free);
}

struct Stack *
mempool_stack(struct Mempool *pool)
{
	return mempool_add(pool, stack_new(), stack_free);
}
