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
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "diff.h"
#include "diffutil.h"
#include "mempool.h"
#include "util.h"

char *
diff_to_patch(struct diff *p, const char *origin_name, const char *target_name, int color)
{
	struct Mempool *pool = mempool_new();
	struct Array *result = mempool_add(pool, array_new(), array_free);

	const char *color_add = ANSI_COLOR_GREEN;
	const char *color_context = ANSI_COLOR_CYAN;
	const char *color_delete = ANSI_COLOR_RED;
	const char *color_reset = ANSI_COLOR_RESET;
	if (!color) {
		color_add = color_context = color_delete = color_reset = "";
	}

	size_t origin_lines = 0;
	size_t origin_start = 1;
	size_t target_lines = 0;
	size_t target_start = 1;
	for (size_t i = 0; i < p->sessz; i++) {
		origin_lines = MAX(origin_lines, p->ses[i].originIdx);
		target_lines = MAX(target_lines, p->ses[i].targetIdx);
	}
	char *buf;
	xasprintf(&buf, "%s--- %s\n%s+++ %s\n%s@@ -%zu,%zu +%zu,%zu @@%s\n",
		color_delete, origin_name, color_add, target_name, color_context,
		origin_start, origin_lines, target_start, target_lines, color_reset);
	array_append(result, mempool_add(pool, buf, free));

	for (size_t i = 0; i < p->sessz; i++) {
		const char *line = *(const char **)p->ses[i].e;
		switch (p->ses[i].type) {
		case DIFF_ADD:
			xasprintf(&buf, "%s+%s%s\n", color_add, line, color_reset);
			array_append(result, mempool_add(pool, buf, free));
			break;
		case DIFF_COMMON:
			xasprintf(&buf, " %s\n", line);
			array_append(result, mempool_add(pool, buf, free));
			break;
		case DIFF_DELETE:
			xasprintf(&buf, "%s-%s%s\n", color_delete, line, color_reset);
			array_append(result, mempool_add(pool, buf, free));
			break;
		}
	}

	buf = str_join(result, "");
	mempool_free(pool);
	return buf;
}
