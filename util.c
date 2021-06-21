/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2019 Tobias Kortkamp <tobik@FreeBSD.org>
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
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "mempool.h"
#include "str.h"
#include "util.h"

void
slice_to_range(size_t len, ssize_t a_, ssize_t b_, size_t *start, size_t *end)
{
	size_t a;
	if (a_ < 0) {
		a_ = 0;
	}
	a = a_;
	if (a >= len) {
		a = len;
	}

	size_t b;
	if (b_ < 0) {
		b_ = len + b_ + 1;
		if (b_ < 0) {
			b_ = 0;
		}
	}
	b = b_;
	if (b > len) {
		b = len;
	}

	if (a > b) {
		*start = 0;
		*end = 0;
	} else {
		assert(a <= len);
		assert(b <= len);
		*start = a;
		*end = b;
	}
}

#if HAVE_GNU_QSORT_R

void
sort(void *base, size_t nmemb, size_t size, CompareFn compar, void *userdata)
{
	qsort_r(base, nmemb, size, compar, userdata);
}

#else

// c.f. https://reviews.freebsd.org/D17083

struct SortWrapper {
	CompareFn compare;
	void *userdata;
};

static int
qsort_r_compare_wrapper(void *userdata, const void *a, const void *b)
{
	const struct SortWrapper *wrapper = userdata;
	return wrapper->compare(a, b, wrapper->userdata);
}

void
sort(void *base, size_t nmemb, size_t size, CompareFn compare, void *userdata)
{
	struct SortWrapper wrapper = { compare, userdata };
	qsort_r(base, nmemb, size, &wrapper, qsort_r_compare_wrapper);
}

#endif

void *
xmalloc(size_t size)
{
	void *x = malloc(size);
	if (x == NULL) {
		warn("malloc");
		abort();
	}
	memset(x, 0, size);
	return x;
}

void *
xrecallocarray(void *ptr, size_t old, size_t nmemb, size_t size)
{
	ptr = recallocarray(ptr, old, nmemb, size);
	if (ptr == NULL) {
		warn("recallocarray");
		abort();
	}
	return ptr;
}
