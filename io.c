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

#include <errno.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "mempool.h"
#include "util.h"

struct LineIterator {
	FILE *f;
	char *line;
	size_t linecap;
	ssize_t linelen;
	size_t i;
};

struct LineIterator *
line_iterator(FILE *f)
{
	struct LineIterator *iter = xmalloc(sizeof(struct LineIterator));
	iter->f = f;
	return iter;
}

void
line_iterator_free(struct LineIterator **iter_)
{
	struct LineIterator *iter = *iter_;
	if (iter != NULL) {
		free(iter->line);
		free(iter);
		*iter_ = NULL;
	}
}

char *
line_iterator_next(struct LineIterator **iter_, size_t *index, size_t *linelen)
{
	struct LineIterator *iter = *iter_;
	if ((iter->linelen = getline(&iter->line, &iter->linecap, iter->f)) > 0) {
		if (iter->linelen > 0 && iter->line[iter->linelen - 1] == '\n') {
			iter->line[iter->linelen - 1] = 0;
			iter->linelen--;
		}
		*index = iter->i;
		*linelen = iter->linelen;
		iter->i++;
		return iter->line;
	} else {
		line_iterator_free(iter_);
		*iter_ = NULL;
		return NULL;
	}
}

char *
slurp(FILE *f, struct Mempool *pool)
{
#define SLURP_BUF_SIZE	(8*1024*1024)
	size_t bufsize = SLURP_BUF_SIZE + 1;
	char *buf = xrecallocarray(NULL, 0, bufsize, 1);
	size_t left = SLURP_BUF_SIZE;
	ssize_t bytes;
	size_t pos = 0;
	while ((bytes = fread(buf + pos, left, 1, f)) != 0) {
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

	return mempool_take(pool, buf);
}
