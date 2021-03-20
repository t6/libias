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

#include <sys/param.h>
#include <assert.h>
#if HAVE_ERR
# include <err.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "array.h"
#include "mempool.h"
#include "peg.h"
#include "queue.h"
#include "set.h"
#include "stack.h"
#include "utf8.h"
#include "util.h"

static const size_t PEG_MAX_DEPTH = 10000;
static const size_t PEG_MAX_ERRORS = 4;

struct PEGError {
	size_t index;
	size_t pos;
	const char *rule;
	const char *msg;
};

struct PEG {
	const char *const buf;
	const size_t len;
	size_t pos;
	size_t depth;

	struct {
		struct Set *set;
		struct Queue *queue;
		struct Stack *pos;
		int enabled;
	} captures;

	int debug;
	struct Array *errors;
	size_t error_index;
	struct Queue *rule_trace;

	struct Mempool *pool;
};


#define MATCHER_INIT() \
	size_t MATCHER_INIT_captures_queue_len = queue_len(peg->captures.queue); \
	size_t MATCHER_INIT_rule_trace_len = queue_len(peg->rule_trace); \
	do { \
		peg->depth++; \
		if (peg->depth > PEG_MAX_DEPTH || peg->pos > peg->len) { \
			MATCHER_RETURN(0); \
		} \
		if (peg->debug) { \
			queue_push(peg->rule_trace, (char *)rule); \
		} \
	} while (0)
#define MATCHER_POP(x) \
do { \
	for (size_t i = MATCHER_INIT_captures_queue_len; i < queue_len(peg->captures.queue); i++) { \
		struct PEGCapture *capture = queue_dequeue(peg->captures.queue); \
		set_remove(peg->captures.set, capture); \
		free(capture); \
	} \
	if (peg->debug) { \
		for (size_t i = MATCHER_INIT_rule_trace_len; i < queue_len(peg->rule_trace); i++) { \
			queue_dequeue(peg->rule_trace); \
		} \
	} \
} while(0)
#define MATCHER_RETURN(x) \
do { \
	peg->depth--; \
	if (!(x)) { \
		MATCHER_POP(); \
	} \
	return (x); \
} while (0)

static int
compare_capture(const void *ap, const void *bp, void *userdata)
{
	struct PEGCapture *a = *(struct PEGCapture **)ap;
	struct PEGCapture *b = *(struct PEGCapture **)bp;
	if (a->pos < b->pos) {
		return -1;
	} else if (a->pos > b->pos) {
		return 1;
	} else if (a->len < b->len) {
		return -1;
	} else if (a->len > b->len) {
		return 1;
	} else if (a->state < b->state) {
		return -1;
	} else if (a->state > b->state) {
		return 1;
	} else if (a->tag < b->tag) {
		return -1;
	} else if (a->tag > b->tag) {
		return 1;
	} else {
		return 0;
	}
}

static int
compare_error(const void *ap, const void *bp, void *userdata)
{
	struct PEGError *a = *(struct PEGError **)ap;
	struct PEGError *b = *(struct PEGError **)bp;
	if (a->pos < b->pos) {
		return 1;
	} else if (a->pos > b->pos) {
		return -1;
	} else if (userdata != NULL && a->index < b->index) {
		return -1;
	} else if (userdata != NULL && a->index > b->index) {
		return 1;
	} else if (a->rule == NULL && b->rule == NULL) {
		return 0;
	} else if (a->rule == NULL) {
		return 1;
	} else if (b->rule == NULL) {
		return -1;
	} else {
		return strcmp(a->rule, b->rule);
	}
}

int
peg_match(struct PEG *peg, RuleFn rulefn, CaptureFn capture_machine, void *userdata)
{
	peg->captures.enabled = capture_machine != NULL;
	peg->depth = 0;
	int result = peg_match_rule(peg, ":main", rulefn);

	if (capture_machine) {
		int stop = 0;
		struct PEGCapture *capture;
		while (!stop && (capture = queue_pop(peg->captures.queue))) {
			mempool_add(peg->pool, capture, free);
			switch (capture_machine(capture, userdata)) {
			case PEG_CAPTURE_CONTINUE:
				break;
			case PEG_CAPTURE_STOP:
				stop = 1;
				break;
			}
		}
		if (stop) {
			while ((capture = queue_pop(peg->captures.queue))) {
				free(capture);
			}
		} else {
			capture = mempool_add(peg->pool, xmalloc(sizeof(struct PEGCapture)), free);
			capture->peg = peg;
			capture->buf = peg->buf;
			capture->pos = 0;
			capture->len = peg->pos;
			capture->tag = -1;
			capture->state = 0; // Accept state
			capture_machine(capture, userdata);
		}
		set_truncate(peg->captures.set);
	}

	if (peg->debug && queue_len(peg->rule_trace) > 0) {
		char *rule = queue_pop(peg->rule_trace);
		printf("%s", rule);
		while ((rule = queue_pop(peg->rule_trace))) {
			printf(" -> %s", rule);
		}
		printf(" -> end@%zu\n", peg->pos);
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
	if (peg->captures.enabled) {
		stack_push(peg->captures.pos, (void *)(uintptr_t)peg->pos);
	}
	return 1;
}

int
peg_match_capture_end(struct PEG *peg, unsigned int tag, unsigned int state, int retval)
{
	if (peg->captures.enabled && stack_len(peg->captures.pos) > 0) {
		size_t start = (size_t)stack_pop(peg->captures.pos);
		if (retval) {
			size_t len = peg->pos - start;
			struct PEGCapture c = { .tag = tag, .state = state, .pos = start, .len = len };
			if (!set_contains(peg->captures.set, &c)) {
				struct PEGCapture *capture = xmalloc(sizeof(struct PEGCapture));
				capture->tag = tag;
				capture->state = state;
				capture->buf = peg->buf + start;
				capture->pos = start;
				capture->len = len;
				capture->peg = peg;
				queue_push(peg->captures.queue, capture);
				set_add(peg->captures.set, capture);
			}
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
peg_match_error(struct PEG *peg, const char *rule, const char *msg)
{
	struct PEGError key = { .rule = rule, .msg = msg, .pos = peg->pos };
	if (array_find(peg->errors, &key, compare_error, &key) == -1) {
		struct PEGError *err = array_get(peg->errors, PEG_MAX_ERRORS - 1);
		err->index = peg->error_index++;
		err->rule = rule;
		err->msg = msg;
		err->pos = peg->pos;
		array_sort(peg->errors, compare_error, NULL);
	}
	return 0;
}

int
peg_match_lookahead(struct PEG *peg, const char *rule, RuleFn rulefn)
{
	MATCHER_INIT();
	size_t pos = peg->pos;
	if (rulefn(peg)) {
		peg->pos = pos;
		MATCHER_POP();
		return 1;
	} else {
		peg->pos = pos;
		MATCHER_RETURN(0);
	}
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
peg_match_repeat(struct PEG *peg, const char *rule, RuleFn rulefn, int n)
{
	MATCHER_INIT();
	size_t pos = peg->pos;
	for (int i = 0; i < n; i++) {
		if (!rulefn(peg)) {
			peg->pos = pos;
			MATCHER_RETURN(0);
		}
	}
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

static void
peg_line_col_at_pos(struct PEG *peg, size_t pos, size_t *line, size_t *col)
{
	*line = 1;
	*col = 1;
	size_t len = MIN(pos, peg->len);
	for (size_t i = 0; i < len;) {
		uint32_t c;
		size_t clen = utf8_decode(peg->buf + i, peg->len - i, &c);
		if (UTF_INVALID == clen) {
			*line = 1;
			*col = i;
			return;
		}
		if (c == '\n') {
			(*line)++;
			*col = 1;
		} else {
			(*col)++;
		}
		i += clen;
	}
}

char *
peg_print_errors(struct PEG *peg, const char *filename)
{
	if (filename == NULL) {
		filename = "<stdin>";
	}
	SCOPE_MEMPOOL(pool);
	struct Array *errors = mempool_add(pool, array_new(), array_free);
	ARRAY_FOREACH(peg->errors, struct PEGError *, err) {
		if (err->rule == NULL) {
			break;
		}
		size_t line;
		size_t col;
		peg_line_col_at_pos(peg, err->pos, &line, &col);
		char *buf;
		if (!err->msg || strcmp(err->msg, "") == 0) {
			buf = str_printf("%s:%zu:%zu: in %s\n", filename, line, col, err->rule);
		} else {
			buf = str_printf("%s:%zu:%zu: in %s: %s\n", filename, line, col, err->rule, err->msg);
		}
		mempool_add(pool, buf, free);
		array_append(errors, buf);
	}
	return str_join(errors, "");
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

	peg->pool = mempool_new();
	mempool_add(peg->pool, peg, free);

	peg->errors = mempool_add(peg->pool, array_new(), array_free);
	for (size_t i = 0; i < PEG_MAX_ERRORS; i++) {
		array_append(peg->errors, mempool_add(peg->pool, xmalloc(sizeof(struct PEGError)), free));
	}

	peg->debug = getenv("LIBIAS_PEG_DEBUG") != NULL;
	peg->rule_trace = mempool_add(peg->pool, queue_new(), queue_free);

	peg->captures.set = mempool_add(peg->pool, set_new(compare_capture, peg, NULL), set_free);
	peg->captures.queue = mempool_add(peg->pool, queue_new(), queue_free);
	peg->captures.pos = mempool_add(peg->pool, stack_new(), stack_free);

	return peg;
}

void
peg_free(struct PEG *peg)
{
	if (peg == NULL) {
		return;
	}
	mempool_free(peg->pool);
}
