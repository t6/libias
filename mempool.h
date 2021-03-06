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
#pragma once

struct Mempool;

typedef int (*MempoolCompareFn)(const void *, const void *, void *);

struct Mempool *mempool_new(void);
struct Mempool *mempool_new_unique(void);
void mempool_free(struct Mempool *);
void mempool_cleanup(struct Mempool **);
void mempool_inherit(struct Mempool *, struct Mempool *);
void mempool_release_all(struct Mempool *);

void *mempool_add(struct Mempool *, void *, void *);
void *mempool_alloc(struct Mempool *, size_t);
void *mempool_forget(struct Mempool *, void *);
void *mempool_move(struct Mempool *, void *, struct Mempool *);
void *mempool_release(struct Mempool *, void *);
void *mempool_take(struct Mempool *, void *);

struct Array *mempool_array(struct Mempool *);
struct Map *mempool_map(struct Mempool *, MempoolCompareFn, void *, void *, void *);
struct Queue *mempool_queue(struct Mempool *);
struct Set *mempool_set(struct Mempool *, MempoolCompareFn, void *, void *);
struct Stack *mempool_stack(struct Mempool *);

#define SCOPE_MEMPOOL(x) \
	struct Mempool *x __cleanup(mempool_cleanup) = mempool_new()

