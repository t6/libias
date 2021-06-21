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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "array.h"
#include "mempool.h"
#include "str.h"
#include "test.h"
#include "util.h"

TESTS() {
	TEST(str_endswith("foo", "o"));
	TEST(str_endswith("foobarverylong", "verylong"));
	TEST(!str_endswith("foobarverylong", "foobar"));
	TEST(str_endswith("foo", ""));
	TEST(!str_endswith("", "o"));
	TEST(!str_endswith("", "verylong"));
	TEST(str_endswith("", ""));

	struct Array *array = mempool_array(pool);
	TEST_STREQ(str_join(pool, array, ","), "");
	array_append(array, "1");
	TEST_STREQ(str_join(pool, array, ","), "1");
	array_append(array, "2");
	array_append(array, "3");
	TEST_STREQ(str_join(pool, array, ","), "1,2,3");
	TEST_STREQ(str_join(pool, array, "foobar"), "1foobar2foobar3");

	TEST_STREQ(str_repeat(pool, "foo", 0), "");
	TEST_STREQ(str_repeat(pool, "foo", 1), "foo");
	TEST_STREQ(str_repeat(pool, "foo", 3), "foofoofoo");
	TEST_STREQ(str_repeat(pool, "a", 10), "aaaaaaaaaa");

	TEST_STREQ(str_slice(pool, "a", 0, 1), "a");
	TEST_STREQ(str_slice(pool, "a", 0, 2), "a");
	TEST_STREQ(str_slice(pool, "foo", 0, 2), "fo");
	TEST_STREQ(str_slice(pool, "foo", 1, 2), "o");
	TEST_STREQ(str_slice(pool, "foo", 2, 1), "");
	TEST_STREQ(str_slice(pool, "foo", 1, 3), "oo");
	TEST_STREQ(str_slice(pool, "foo", 1, 1000), "oo");
	TEST_STREQ(str_slice(pool, "foo", 1000, 1000), "");
	TEST_STREQ(str_slice(pool, "foo", 0, -1), "foo");
	TEST_STREQ(str_slice(pool, "foo", -1, -1), "foo");
	TEST_STREQ(str_slice(pool, "foo", 0, -2), "fo");
	TEST_STREQ(str_slice(pool, "foo", 0, -3), "f");
	TEST_STREQ(str_slice(pool, "foo", 0, -4), "");
	TEST_STREQ(str_slice(pool, "foo", 0, -4), "");

	TEST_STREQ(str_trim(pool, "foo"), "foo");
	TEST_STREQ(str_trim(pool, "   foo  "), "foo");
	TEST_STREQ(str_trim(pool, " \tfoo  "), "foo");

	TEST_STREQ(str_triml(pool, "foo"), "foo");
	TEST_STREQ(str_triml(pool, "   foo  "), "foo  ");
	TEST_STREQ(str_triml(pool, " \tfoo  "), "foo  ");

	TEST_STREQ(str_trimr(pool, "foo"), "foo");
	TEST_STREQ(str_trimr(pool, "   foo  "), "   foo");
	TEST_STREQ(str_trimr(pool, " \tfoo  "), " \tfoo");
}
