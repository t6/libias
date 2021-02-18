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

#include "queue.h"
#include "util.h"

struct QueueNode {
	struct QueueNode *next;
	struct QueueNode *prev;
	void *value;
};

struct Queue {
	size_t len;
	struct QueueNode *head;
	struct QueueNode *tail;
};

struct Queue *
queue_new()
{
	struct Queue *queue = xmalloc(sizeof(struct Queue));
	return queue;
}

void
queue_free(struct Queue *queue)
{
	free(queue);
}

size_t
queue_len(struct Queue *queue)
{
	return queue->len;
}

void *
queue_peek(struct Queue *queue)
{
	if (queue->head) {
		return queue->head->value;
	} else {
		return NULL;
	}
}

void *
queue_pop(struct Queue *queue)
{
	if (queue->head) {
		void *value = queue->head->value;
		queue->head = queue->head->next;
		if (queue->head) {
			queue->head->prev = NULL;
			if (queue->head->next == NULL) {
				queue->tail = queue->head;
			}
		} else {
			queue->tail = NULL;
		}
		queue->len--;
		return value;
	} else {
		return NULL;
	}
}

void
queue_push(struct Queue *queue, void *value)
{
	if (value == NULL) {
		abort();
	}

	struct QueueNode *node = xmalloc(sizeof(struct QueueNode));
	node->value = value;
	if (queue->tail) {
		queue->tail->next = node;
		node->prev = queue->tail;
		queue->tail = node;
	} else {
		queue->head = queue->tail = node;
	}
	queue->len++;
}

void
queue_unqueue(struct Queue *queue, size_t n)
{
	for (size_t i = 0; i < n && queue->tail != NULL; queue->tail = queue->tail->prev, i++) {
		if (queue->tail->prev) {
			queue->tail->prev->next = NULL;
		}
		free(queue->tail);
		queue->len--;
	}
}

