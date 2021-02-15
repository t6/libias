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
#include "peg.h"
#include "peg-macros.h"
#include "test.h"
#include "util.h"

static inline char *
check_captures(RuleFn rule, const char *s, int tag, const char *sep)
{
	struct PEG *peg = peg_new(s, strlen(s), NULL);
	int result = peg_match(peg, rule);
	if (result && peg_captures(peg, tag)) {
		struct Array *caps = array_new();
		ARRAY_FOREACH(peg_captures(peg, tag), struct PEGCapture *, cap) {
			array_append(caps, cap->buf);
		}
		return str_join(caps, sep);
	}
	peg_free(peg);
	return NULL;
}

static inline int
check_match(RuleFn rule, const char *s, int expected)
{
	struct PEG *peg = peg_new(s, strlen(s), NULL);
	int result = peg_match(peg, rule);
	peg_free(peg);
	return result == expected;
}
