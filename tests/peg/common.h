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
#pragma once

#include "config.h"
#include <assert.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "memorypool.h"
#include "peg.h"
#include "peg/macros.h"
#include "test.h"
#include "util.h"

static enum PEGCaptureFlag
captures_to_array(struct PEGCapture *capture, void *userdata)
{
	struct Array *captures = userdata;
	array_append(captures, capture);
	return PEG_CAPTURE_KEEP;
}

static inline char *
check_captures(RuleFn rule, const char *s, unsigned int tag, const char *sep)
{
	struct MemoryPool *pool = memory_pool_new();
	struct PEG *peg = memory_pool_acquire(pool, peg_new(s, strlen(s)), free);
	struct Array *captures = memory_pool_acquire(pool, array_new(), array_free);
	int result = peg_match(peg, rule, captures_to_array, captures);
	if (result) {
		struct Array *caps = memory_pool_acquire(pool, array_new(), array_free);
		ARRAY_FOREACH(captures, struct PEGCapture *, cap) {
			if (cap->tag == tag) {
				array_append(caps, memory_pool_acquire(pool, xstrndup(cap->buf, cap->len), free));
			}
		}
		memory_pool_free(pool);
		return str_join(caps, sep);
	}
	memory_pool_free(pool);
	return NULL;
}

static inline int
check_match(RuleFn rule, const char *s, int expected)
{
	struct PEG *peg = peg_new(s, strlen(s));
	int result = peg_match(peg, rule, NULL, NULL);
	peg_free(peg);
	return result == expected;
}
