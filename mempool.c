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

#include "mempool.h"
#include "queue.h"
#include "util.h"

struct Mempool {
	struct Queue *queue;
};

struct Mempool *
mempool_new()
{
	struct Mempool *pool = xmalloc(sizeof(struct Mempool));
	pool->queue = queue_new();
	return pool;
}

void
mempool_free(struct Mempool *pool)
{
	if (pool == NULL) {
		return;
	}
	mempool_release(pool);
	queue_free(pool->queue);
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
	if (ptr) {
		assert(freefn != NULL);
		queue_push(pool->queue, ptr);
		queue_push(pool->queue, freefn);
	}
	return ptr;
}

void
mempool_release(struct Mempool *pool)
{
	void *ptr;
	while ((ptr = queue_pop(pool->queue))) {
		void (*freefn)(void *) = queue_pop(pool->queue);
		freefn(ptr);
	}
}
