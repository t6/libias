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
#include <string.h>

#include "array.h"
#include "map.h"
#include "peg.h"
#include "utf8.h"
#include "util.h"

#define MAX_CAPTURES 			(64 * 128)

struct PEG {
	const char *const buf;
	const size_t len;
	size_t pos;

	struct {
		struct PEGCapture captures[MAX_CAPTURES];
		size_t len;
		size_t stack[MAX_CAPTURES];
		size_t stack_len;
	} captures;

	CaptureFn capture_machine;
	struct Array *gc[2];
};


#define MATCHER_INIT() size_t MATCHER_INIT_captures_len = peg->captures.len; do { \
		if (peg->pos > peg->len) { \
			MATCHER_RETURN(0); \
		} \
	} while (0)
#define MATCHER_RETURN(x) do { \
		if (!(x)) { \
			peg->captures.len = MATCHER_INIT_captures_len; \
		} \
		return (x); \
	} while (0)

struct Array *
peg_captures(struct PEG *peg, unsigned int tag)
{
	struct Array *a = array_new();
	for (size_t i = 0; i < peg->captures.len; i++) {
		struct PEGCapture *capture = &peg->captures.captures[i];
		if (capture->tag == tag) {
			if (!capture->buf) {
				capture->buf = peg_gc(peg, xstrndup(peg->buf + capture->pos, capture->len), free);
			}
			array_append(a, capture);
		}
	}

	return a;
}

int
peg_match(struct PEG *peg, RuleFn rulefn, void *userdata)
{
	int result = peg_match_rule(peg, ":main", rulefn);

	if (peg->capture_machine && userdata) {
		for (size_t i = 0; i < peg->captures.len; i++) {
			struct PEGCapture *capture = &peg->captures.captures[i];
			if (capture->buf == NULL) {
				capture->buf = peg_gc(peg, xstrndup(peg->buf + capture->pos, capture->len), free);
			}
			peg->capture_machine(capture, userdata);
		}
		struct PEGCapture capture;
		capture.peg = peg;
		capture.buf = peg_gc(peg, xstrndup(peg->buf, peg->pos), free);
		capture.pos = 0;
		capture.len = peg->pos;
		capture.tag = -1;
		capture.state = 0; // Accept state
		peg->capture_machine(&capture, userdata);
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
	if (peg->captures.stack_len >= MAX_CAPTURES) {
		return 0;
	}
	peg->captures.stack[peg->captures.stack_len++] = peg->pos;
	return 1;
}

int
peg_match_capture_end(struct PEG *peg, unsigned int tag, unsigned int state, CaptureFn f, size_t size, int retval)
{
	if (peg->captures.stack_len > 0) {
		peg->captures.stack_len--;
		if (retval && peg->captures.len < MAX_CAPTURES) {
			struct PEGCapture *capture = &peg->captures.captures[peg->captures.len++];
			size_t start = peg->captures.stack[peg->captures.stack_len];
			size_t len = peg->pos - start;
			capture->tag = tag;
			capture->state = state;
			capture->pos = start;
			capture->len = len;
			capture->peg = peg;
			peg->capture_machine = f;
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

	peg->gc[0] = array_new();
	peg->gc[1] = array_new();

	return peg;
}

void
peg_free(struct PEG *peg)
{
	if (peg == NULL) {
		return;
	}

	for (size_t i = 0; i < array_len(peg->gc[0]); i++) {
		void *ptr = array_get(peg->gc[0], i);
		void (*freefn)(void *) = array_get(peg->gc[1], i);
		freefn(ptr);
	}
	array_free(peg->gc[0]);
	array_free(peg->gc[1]);

	free(peg);
}

void *
peg_gc(struct PEG *peg, void *ptr, void *freefn)
{
	if (ptr) {
		assert(freefn != NULL);
		array_append(peg->gc[0], ptr);
		array_append(peg->gc[1], freefn);
	}
	return ptr;
}
