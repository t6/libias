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
#if HAVE_ERR
# include <err.h>
#endif
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "mempool.h"
#include "str.h"
#include "util.h"

static size_t xstrlcat(char *, const char *, size_t);
static char *xstrdup(const char *);
static char *xstrndup(const char *, size_t);

int
str_casecompare(const void *ap, const void *bp, void *userdata)
{
	const char *a = *(const char **)ap;
	const char *b = *(const char **)bp;
	return strcasecmp(a, b);
}

int
str_compare(const void *ap, const void *bp, void *userdata)
{
	const char *a = *(const char **)ap;
	const char *b = *(const char **)bp;
	return strcmp(a, b);
}

char *
str_common_prefix(struct Mempool *pool, const char *a, const char *b)
{
	const char *ap = a;
	const char *bp = b;
	size_t i;
	for (i = 0; *ap != 0 && *bp != 0 && *ap++ == *bp++; i++);
	if (i > 0) {
		return mempool_take(pool, xstrndup(a, i));
	} else {
		return NULL;
	}
}

char *
str_dup(struct Mempool *pool, const char *s)
{
	return mempool_take(pool, xstrdup(s));
}

char *
str_ndup(struct Mempool *pool, const char *s, size_t n)
{
	return mempool_take(pool, xstrndup(s, n));
}

int
str_endswith(const char *s, const char *end)
{
	size_t len = strlen(end);
	if (strlen(s) < len) {
		return 0;
	}
	return strncmp(s + strlen(s) - len, end, len) == 0;
}

char *
str_join(struct Mempool *pool, struct Array *array, const char *sep)
{
	size_t seplen = strlen(sep);
	size_t lastindex = array_len(array) - 1;

	size_t sz = 1;
	size_t i = 0;
	ARRAY_FOREACH(array, const char *, s) {
		sz += strlen(s);
		if (i++ != lastindex) {
			sz += seplen;
		}
	}

	char *buf = mempool_alloc(pool, sz);
	i = 0;
	ARRAY_FOREACH(array, const char *, s) {
		xstrlcat(buf, s, sz);
		if (i++ != lastindex) {
			xstrlcat(buf, sep, sz);
		}
	}
	return buf;
}

char *
str_map(struct Mempool *pool, const char *s, size_t len, int (*f)(int))
{
	char *buf = mempool_alloc(pool, len + 1);
	for (size_t i = 0; i < len && s[i] != 0; i++) {
		buf[i] = f(s[i]);
	}
	return buf;
}

char *
str_printf(struct Mempool *pool, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	char *buf = NULL;
	int retval = vasprintf(&buf, format, ap);
	mempool_take(pool, buf);
	va_end(ap);

	if (retval < 0) {
		warn("vasprintf");
		abort();
	}

	return buf;
}

char *
str_repeat(struct Mempool *pool, const char *s, const size_t n)
{
	const size_t sz = strlen(s) * n + 1;
	char *buf = mempool_alloc(pool, sz);
	if (n > 0) {
		for (size_t i = 0; i < n; i++) {
			xstrlcat(buf, s, sz);
		}
	}
	return buf;
}

char *
str_slice(struct Mempool *pool, const char *s, const ssize_t a, const ssize_t b)
{
	size_t len = strlen(s);
	size_t start = 0;
	size_t end = 0;
	slice_to_range(len, a, b, &start, &end);
	return str_ndup(pool, s + start, end - start);
}

int
str_startswith(const char *s, const char *start)
{
	size_t len = strlen(start);
	if (strlen(s) < len) {
		return 0;
	}
	return strncmp(s, start, len) == 0;
}

char *
str_trim(struct Mempool *pool, const char *s)
{
	const char *sp = s;
	for (; *sp && isspace(*sp); ++sp);
	return str_trimr(pool, sp);
}

char *
str_triml(struct Mempool *pool, const char *s)
{
	const char *sp = s;
	for (; *sp && isspace(*sp); ++sp);
	return mempool_take(pool, xstrdup(sp));
}

char *
str_trimr(struct Mempool *pool, const char *s)
{
	size_t len = strlen(s);
	while (len > 0 && isspace(s[len - 1])) {
		len--;
	}
	return mempool_take(pool, xstrndup(s, len));
}

char *
xstrdup(const char *s)
{
	char *retval = strdup(s);
	if (retval == NULL) {
		warn("strdup");
		abort();
	}
	return retval;
}

char *
xstrndup(const char *s, size_t len)
{
	char *retval = strndup(s, len);
	if (retval == NULL) {
		warn("strndup");
		abort();
	}
	return retval;
}

size_t
xstrlcat(char *dst, const char *src, size_t dstsize)
{
	size_t len = strlcat(dst, src, dstsize);
	if (len >= dstsize) {
		warnx("strlcat: truncated string: %zu/%zu", len, dstsize);
		abort();
	}
	return len;
}
