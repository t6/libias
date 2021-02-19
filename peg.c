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
#if HAVE_ERR
# include <err.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "array.h"
#include "map.h"
#include "memorypool.h"
#include "peg.h"
#include "stack.h"
#include "queue.h"
#include "utf8.h"
#include "util.h"

struct PEG {
	const char *const buf;
	const size_t len;
	size_t pos;

	struct {
		struct Queue *queue;
		struct Stack *pos;
	} captures;

	int debug;
	struct Queue *rule_trace;

	struct MemoryPool *pool;
};


#define MATCHER_INIT() \
	size_t MATCHER_INIT_captures_queue_len = queue_len(peg->captures.queue); \
	size_t MATCHER_INIT_rule_trace_len = queue_len(peg->rule_trace); \
	do { \
		if (peg->pos > peg->len) { \
			MATCHER_RETURN(0); \
		} \
		if (peg->debug) { \
			queue_push(peg->rule_trace, (char *)rule); \
		} \
	} while (0)
#define MATCHER_RETURN(x) do { \
		if (!(x)) { \
			for (size_t i = MATCHER_INIT_captures_queue_len; i < queue_len(peg->captures.queue); i++) { \
				free(queue_dequeue(peg->captures.queue)); \
			} \
			if (peg->debug) { \
				for (size_t i = MATCHER_INIT_rule_trace_len; i < queue_len(peg->rule_trace); i++) { \
					queue_dequeue(peg->rule_trace); \
				} \
			} \
		} \
		return (x); \
	} while (0)

struct Array *
peg_captures(struct PEG *peg, unsigned int tag)
{
	struct Array *a = array_new();
	struct PEGCapture *capture;
	struct Queue *captures = queue_new();;
	while ((capture = queue_pop(peg->captures.queue))) {
		if (capture->tag == tag) {
			array_append(a, capture);
		}
		queue_push(captures, capture);
	}
	queue_free(peg->captures.queue);
	peg->captures.queue = captures;
	return a;
}

int
peg_match(struct PEG *peg, RuleFn rulefn, CaptureFn capture_machine, void *userdata)
{
	int result = peg_match_rule(peg, ":main", rulefn);

	if (capture_machine) {
		struct PEGCapture *capture;
		struct Queue *captures = queue_new();
		while ((capture = queue_pop(peg->captures.queue))) {
			memory_pool_acquire(peg->pool, capture, free);
			capture_machine(peg->pool, capture, userdata);
			queue_push(captures, capture);
		}
		queue_free(peg->captures.queue);
		peg->captures.queue = captures;
		capture = memory_pool_acquire(peg->pool, xmalloc(sizeof(struct PEGCapture)), free);
		capture->peg = peg;
		capture->buf = peg->buf;
		capture->pos = 0;
		capture->len = peg->pos;
		capture->tag = -1;
		capture->state = 0; // Accept state
		capture_machine(peg->pool, capture, userdata);
	}

	if (peg->debug && queue_len(peg->rule_trace) > 0) {
		char *rule = queue_pop(peg->rule_trace);
		printf("%s", rule);
		while ((rule = queue_pop(peg->rule_trace))) {
			printf(" -> %s", rule);
		}
		printf("\n");
	}

	return result;
}

int
peg_match_atleast(struct PEG *peg, const char *rule, RuleFn rulefn, int n)
{
	MATCHER_INIT();
	size_t pos = peg->pos;
	int i;
	for (i = 0; ; i++) {
		if (!rulefn(peg)) {
			break;
		}
	}
	if (i < n) {
		peg->pos = pos;
		MATCHER_RETURN(0);
	}
	MATCHER_RETURN(1);
}

int
peg_match_between(struct PEG *peg, const char *rule, RuleFn rulefn, int a, int b)
{
	MATCHER_INIT();
	size_t pos = peg->pos;
	int i;
	for (i = 0; i <= b; i++) {
		if (!rulefn(peg)) {
			break;
		}
	}
	if (i >= a && i <= b) {
		MATCHER_RETURN(1);
	} else {
		peg->pos = pos;
		MATCHER_RETURN(0);
	}
}

int
peg_match_capture_start(struct PEG *peg)
{
	stack_push(peg->captures.pos, (void *)(uintptr_t)peg->pos);
	return 1;
}

int
peg_match_capture_end(struct PEG *peg, unsigned int tag, unsigned int state, int retval)
{
	if (stack_len(peg->captures.pos) > 0) {
		size_t start = (size_t)stack_pop(peg->captures.pos);
		if (retval) {
			struct PEGCapture *capture = xmalloc(sizeof(struct PEGCapture));
			size_t len = peg->pos - start;
			capture->tag = tag;
			capture->state = state;
			capture->buf = peg->buf + start;
			capture->pos = start;
			capture->len = len;
			capture->peg = peg;
			queue_push(peg->captures.queue, capture);
		}
	}
	return retval;
}

int
peg_match_char(struct PEG *peg, const char *rule, uint32_t c)
{
	MATCHER_INIT();
	char needle[UTF_SIZE + 1];
	if (!utf8_encode(c, needle)) {
		MATCHER_RETURN(0);
	}
	size_t len = strlen(needle);
	if ((peg->len - peg->pos) >= len &&
	    strncmp(peg->buf + peg->pos, needle, len) == 0) {
		peg->pos += len;
		MATCHER_RETURN(1);
	} else {
		MATCHER_RETURN(0);
	}
}

int
peg_match_char_f(struct PEG *peg, const char *rule, int (*f)(int))
{
	MATCHER_INIT();
	if (peg->pos >= peg->len) {
		MATCHER_RETURN(0);
	}
	if (f(peg->buf[peg->pos])) {
		peg->pos++;
		MATCHER_RETURN(1);
	} else {
		MATCHER_RETURN(0);
	}
}

int
peg_match_chars(struct PEG *peg, const char *rule, uint32_t chars[], size_t len)
{
	MATCHER_INIT();
	for (size_t i = 0; i < len; i++) {
		char needle[UTF_SIZE + 1];
		if (!utf8_encode(chars[i], needle)) {
			MATCHER_RETURN(0);
		}
		size_t len = strlen(needle);
		if ((peg->len - peg->pos) >= len &&
		    strncmp(peg->buf + peg->pos, needle, len) == 0) {
			peg->pos += len;
			MATCHER_RETURN(1);
		}
	}
	MATCHER_RETURN(0);
}

int
peg_match_eos(struct PEG *peg, const char *rule)
{
	MATCHER_INIT();
	MATCHER_RETURN(peg->pos == peg->len);
}

int
peg_match_range(struct PEG *peg, const char *rule, uint32_t a, uint32_t b)
{
	MATCHER_INIT();
	if (peg->pos >= peg->len) {
		MATCHER_RETURN(0);
	}

	uint32_t c;
	size_t clen = utf8_decode(peg->buf + peg->pos, peg->len - peg->pos, &c);
	if (UTF_INVALID == clen || a > c || c > b) {
		MATCHER_RETURN(0);
	}
	peg->pos += clen;
	MATCHER_RETURN(1);
}

int
peg_match_rule(struct PEG *peg, const char *rule, RuleFn rulefn)
{
	MATCHER_INIT();
	size_t pos = peg->pos;
	if (rulefn(peg)) {
		MATCHER_RETURN(1);
	} else {
		peg->pos = pos;
		MATCHER_RETURN(0);
	}
}

int
peg_match_string(struct PEG *peg, const char *rule, const char *needle)
{
	MATCHER_INIT();
	size_t len = strlen(needle);
	if ((peg->len - peg->pos) >= len &&
	    strncmp(peg->buf + peg->pos, needle, len) == 0) {
		peg->pos += len;
		MATCHER_RETURN(1);
	} else {
		MATCHER_RETURN(0);
	}
}

static int
peg_match_thruto(struct PEG *peg, const char *rule, const char *needle, int thru)
{
	MATCHER_INIT();
	if (strcmp(needle, "") == 0) {
		MATCHER_RETURN(0);
	}

	char *ptr = strnstr(peg->buf + peg->pos, needle, peg->len);
	if (ptr == NULL) {
		MATCHER_RETURN(0);
	}

	size_t offset = 0;
	if (thru) {
		offset = strlen(needle);
	}
	size_t len = ptr - (peg->buf + peg->pos) + offset;
	peg->pos += len;

	MATCHER_RETURN(1);
}

int
peg_match_thru(struct PEG *peg, const char *rule, const char *needle)
{
	return peg_match_thruto(peg, rule, needle, 1);
}

int
peg_match_to(struct PEG *peg, const char *rule, const char *needle)
{
	return peg_match_thruto(peg, rule, needle, 0);
}

struct PEG *
peg_new(const char *const buf, size_t len)
{
	struct PEG proto = {
		.buf = buf,
		.len = len,
	};
	struct PEG *peg = xmalloc(sizeof(struct PEG));
	memcpy(peg, &proto, sizeof(*peg));

	peg->pool= memory_pool_new();

	peg->debug = getenv("LIBIAS_PEG_DEBUG") != NULL;
	peg->rule_trace = queue_new();

	peg->captures.queue = queue_new();
	peg->captures.pos = stack_new();

	return peg;
}

void
peg_free(struct PEG *peg)
{
	if (peg == NULL) {
		return;
	}

	queue_free(peg->rule_trace);
	queue_free(peg->captures.queue);
	stack_free(peg->captures.pos);
	memory_pool_free(peg->pool);

	free(peg);
}
