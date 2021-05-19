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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "diff.h"
#include "diffutil.h"
#include "mempool.h"
#include "str.h"
#include "util.h"

struct Hunk {
	size_t start;
	size_t end;
};

static int
by_start(const void *ap, const void *bp, void *userdata)
{
	struct Hunk *a = *(struct Hunk **)ap;
	struct Hunk *b = *(struct Hunk **)bp;
	if (a->start < b->start) {
		return -1;
	} else if (a->start > b->start) {
		return 1;
	} else {
		return 0;
	}
}

static struct Array *
get_hunks(struct Mempool *pool, struct diff *p, size_t context)
{
	assert(p->sessz > 0);
	if (context == 0) {
		struct Array *hunks = mempool_array(pool);
		struct Hunk *h = mempool_add(pool, xmalloc(sizeof(struct Hunk)), free);
		h->start = 0;
		h->end = p->sessz - 1;
		array_append(hunks, h);
		return hunks;
	}

	struct Array *edit_ranges = mempool_array(pool);
	struct Hunk *h = mempool_add(pool, xmalloc(sizeof(struct Hunk)), free);
	int first = 1;
	int last_common = 1;
	for (size_t i = 0; i < p->sessz; i++) {
		switch (p->ses[i].type) {
		case DIFF_COMMON:
			if (!last_common) {
				array_append(edit_ranges, h);
				h = mempool_add(pool, xmalloc(sizeof(struct Hunk)), free);
			}
			last_common = 1;
			first = 1;
			break;
		case DIFF_ADD:
		case DIFF_DELETE:
			last_common = 0;
			if (first) {
				h->start = i;
				first = 0;
			}
			h->end = i;
			break;
		}
	}
	if (!first) {
		array_append(edit_ranges, h);
	}

	ARRAY_FOREACH(edit_ranges, struct Hunk *, h) {
		if (h->start > context) {
			h->start -= context;
		}
		h->end = MIN(h->end + context, p->sessz - 1);
	}
	array_sort(edit_ranges, by_start, NULL);

	struct Array *hunks = mempool_array(pool);
	struct Hunk *last = NULL;
	ARRAY_FOREACH(edit_ranges, struct Hunk *, h) {
		if (last == NULL) {
			last = h;
			continue;
		} else {
			if (h->start <= last->end) {
				if (h->end > last->end) {
					last->end = h->end;
				}
			} else if (h->start > last->end) {
				array_append(hunks, last);
				last = h;
			}
		}
	}
	if (last) {
		array_append(hunks, last);
	}

	assert(array_len(hunks) > 0);
	return hunks;
}


char *
diff_to_patch(struct diff *p, TostringFn tostring, void *tostring_userdata, size_t context, int color)
{
	if (p->sessz == 0) {
		return NULL;
	}

	SCOPE_MEMPOOL(pool);
	const char *color_add = ANSI_COLOR_GREEN;
	const char *color_context = ANSI_COLOR_CYAN;
	const char *color_delete = ANSI_COLOR_RED;
	const char *color_reset = ANSI_COLOR_RESET;
	if (!color) {
		color_add = color_context = color_delete = color_reset = "";
	}

	struct Array *hunks = get_hunks(pool, p, context);
	struct Array *result = mempool_array(pool);
	ARRAY_FOREACH(hunks, struct Hunk *, h) {
		size_t origin_len = 0;
		size_t target_len = 0;
		for (size_t i = h->start; i <= h->end; i++) {
			switch (p->ses[i].type) {
			case DIFF_ADD:
				target_len++;
				break;
			case DIFF_COMMON:
				origin_len++;
				target_len++;
				break;
			case DIFF_DELETE:
				origin_len++;
				break;
			}
		}
		size_t origin_start = p->ses[h->start].originIdx;
		if (origin_start == 0) {
			origin_start = 1;
		}
		size_t target_start = p->ses[h->start].targetIdx;
		char *buf;
		if (origin_len > 1) {
			buf = str_printf("%s@@ -%zu,%zu", color_context, origin_start, origin_len);
		} else {
			buf = str_printf("%s@@ -%zu", color_context, origin_start);
		}
		array_append(result, mempool_add(pool, buf, free));
		if (target_len > 1) {
			buf = str_printf(" +%zu,%zu @@%s\n", target_start, target_len, color_reset);
		} else {
			buf = str_printf(" +%zu @@%s\n", target_start, color_reset);
		}
		array_append(result, mempool_add(pool, buf, free));
		for (size_t i = h->start; i <= h->end; i++) {
			char *line;
			if (tostring) {
				line = mempool_add(pool, tostring(*(void **)p->ses[i].e, tostring_userdata), free);
			} else {
				line = *(void **)p->ses[i].e;
			}
			switch (p->ses[i].type) {
			case DIFF_ADD:
				buf = str_printf("%s+%s%s\n", color_add, line, color_reset);
				break;
			case DIFF_COMMON:
				buf = str_printf(" %s\n", line);
				break;
			case DIFF_DELETE:
				buf = str_printf("%s-%s%s\n", color_delete, line, color_reset);
				break;
			}
			array_append(result, mempool_add(pool, buf, free));
		}
	}

	return str_join(result, "");
}
