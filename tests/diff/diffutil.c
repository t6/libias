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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "diff.h"
#include "diffutil.h"
#include "io.h"
#include "mempool.h"
#include "mempool/file.h"
#include "str.h"
#include "test.h"
#include "util.h"

TESTS() {
	struct Array *a = mempool_array(pool);
	for (size_t i = 0; i < 16; i++) {
		array_append(a, "1");
	}
	struct Array *b = mempool_array(pool);
	array_append(b, "2");
	array_append(b, "2");
	for (size_t i = 0; i < 8; i++) {
		array_append(b, "1");
	}
	array_append(b, "3");
	for (size_t i = 0; i < 7; i++) {
		array_append(b, "1");
	}

	struct diff *d;
	TEST_IF((d = array_diff(a, b, pool, str_compare, NULL))) {
		char *actual = diff_to_patch(d, pool, NULL, NULL, 3, 0);
		FILE *f = mempool_fopenat(pool, AT_FDCWD, "tests/diff/0001.diff", "r", 0);
		char *expected = slurp(f, pool);
		TEST_STREQ(actual, expected);
	}
}
