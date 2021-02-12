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
#include <string.h>

#include "peg.h"
#include "util.h"

struct PEG {
	const char *const buf;
	const size_t len;
	size_t pos;

	CaptureFn on_capture;
	MismatchFn on_mismatch;
	void *userdata;
};

#define MATCHER_INIT() do { \
		if (peg->pos > peg->len) { \
			MATCHER_RETURN(0); \
		} \
	} while (0)
#define MATCHER_RETURN(x) do { \
		return (x); \
	} while (0)

static void
peg_capture(struct PEG *peg, const char *rule, size_t len)
{
	assert((peg->pos + len) <= peg->len);
	if (peg->on_capture) {
		peg->on_capture(peg, rule, peg->buf + peg->pos, len, peg->userdata);
	}
}

int
peg_match_atleast(struct PEG *peg, const char *rule, RuleFn rulefn, int n)
{
	MATCHER_INIT();
	size_t pos = peg->pos;
	int i = 0;
	while (1) {
		if (!rulefn(peg, rule)) {
			break;
		}
		i++;
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
		if (!rulefn(peg, rule)) {
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
peg_match_char(struct PEG *peg, const char *rule, char c)
{
	MATCHER_INIT();
	if (peg->pos >= peg->len) {
		MATCHER_RETURN(0);
	}
	if (peg->buf[peg->pos] == c) {
		peg->pos++;
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
peg_match_eos(struct PEG *peg, const char *rule)
{
	MATCHER_INIT();
	MATCHER_RETURN(peg->pos == peg->len);
}

int
peg_match_range(struct PEG *peg, const char *rule, char a, char b)
{
	MATCHER_INIT();
	if (peg->pos >= peg->len) {
		MATCHER_RETURN(0);
	}
	char c = peg->buf[peg->pos];
	if (a > c || c > b) {
		MATCHER_RETURN(0);
	}
	peg->pos++;
	MATCHER_RETURN(1);
}

int
peg_match_repeat(struct PEG *peg, const char *rule, RuleFn rulefn, int n)
{
	MATCHER_INIT();
	size_t pos = peg->pos;
	for (int i = 0; i < n; i++) {
		if (!rulefn(peg, rule)) {
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
	if (rulefn(peg, rule)) {
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
	if (peg->on_capture) {
		peg_capture(peg, rule, len);
	}
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
peg_new(const char *const buf, size_t len, CaptureFn on_capture, MismatchFn on_mismatch)
{
	struct PEG proto = {
		.buf = buf,
		.len = len,
		.on_capture = on_capture,
		.on_mismatch = on_mismatch
	};
	struct PEG *peg = xmalloc(sizeof(struct PEG));
	memcpy(peg, &proto, sizeof(*peg));
	return peg;
}

void
peg_free(struct PEG *peg)
{
	free(peg);
}

