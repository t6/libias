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

#include <sys/param.h>
#if HAVE_ERR
# include <err.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "array.h"
#include "mempool.h"
#include "str.h"
#include "util.h"

char *
read_symlink(int dir, const char *path)
{
	char buf[PATH_MAX];
	ssize_t len = readlinkat(dir, path, buf, sizeof(buf));
	if (len != -1) {
		return xstrndup(buf, len);
	}
	return NULL;
}

char *
slurp(int fd)
{
#define SLURP_BUF_SIZE	(8*1024*1024)
	size_t bufsize = SLURP_BUF_SIZE + 1;
	char *buf = xmalloc(bufsize);
	size_t left = SLURP_BUF_SIZE;
	ssize_t bytes;
	size_t pos = 0;
	while ((bytes = read(fd, buf + pos, left)) != 0) {
		if (bytes < 0) {
			if (errno == EAGAIN) {
				continue;
			}
			free(buf);
			return NULL;
		}
		left -= bytes;
		pos += bytes;
		if (left == 0) {
			size_t oldsize = bufsize;
			bufsize += SLURP_BUF_SIZE;
			left = SLURP_BUF_SIZE;
			buf = xrecallocarray(buf, oldsize, bufsize, 1);
		}
	}

	return buf;
}

int
update_symlink(int dir, const char *path1, const char *path2, struct Mempool *pool, char **prev)
{
	if (prev != NULL) {
		*prev = NULL;
	}
	while (symlinkat(path1, dir, path2) == -1) {
		if (errno == EEXIST) {
			if (prev != NULL) {
				*prev = mempool_take(pool, read_symlink(dir, path2));
			}
			if (unlinkat(dir, path2, 0) == -1) {
				if (prev != NULL) {
					*prev = NULL;
				}
				return 0;
			}
		} else {
			if (prev != NULL) {
				*prev = NULL;
			}
			return 0;
		}
	}

	return 1;
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
