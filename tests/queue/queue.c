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
#include <stdio.h>
#include <string.h>

#include "mempool.h"
#include "queue.h"
#include "str.h"
#include "test.h"
#include "util.h"

TESTS() {
	struct Queue *queue = mempool_queue(pool);
	queue_push(queue, "1");
	queue_push(queue, "2");
	queue_push(queue, "3");
	TEST(queue_len(queue) == 3);
	TEST_STREQ(queue_peek(queue), "1");
	TEST_STREQ(queue_pop(queue), "1");
	TEST(queue_len(queue) == 2);
	TEST_STREQ(queue_pop(queue), "2");
	TEST(queue_len(queue) == 1);
	TEST_STREQ(queue_pop(queue), "3");
	TEST(queue_len(queue) == 0);
	TEST(queue_pop(queue) == NULL);

	queue_push(queue, "1");
	queue_push(queue, "2");
	queue_push(queue, "3");
	queue_push(queue, "4");
	TEST(queue_len(queue) == 4);
	queue_dequeue(queue);
	queue_dequeue(queue);
	queue_dequeue(queue);
	TEST_STREQ(queue_peek(queue), "1");
	queue_dequeue(queue);
	TEST(queue_len(queue) == 0);

	queue = mempool_queue(pool);
	queue_push(queue, "1");
	TEST_STREQ(queue_dequeue(queue), "1");
	TEST(queue_pop(queue) == NULL);
}
