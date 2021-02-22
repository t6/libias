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
#include "json.h"
#include "peg.h"
#include "peg/json.h"
#include "test.h"
#include "util.h"

static int
check_match(RuleFn rule, const char *s, int expected)
{
	struct PEG *peg = peg_new(s, strlen(s));
	int result = peg_match(peg, rule, NULL, NULL);
	peg_free(peg);
	return result == expected;
}

TESTS() {
	// int fd = open("pkg-status.json", O_RDONLY);
	// char *buf = slurp(fd);
	// TEST(check_match(peg_json_decode, buf, 1));

	TEST(check_match(peg_json_decode, "[null]", 1));
	TEST(check_match(peg_json_decode, "\"\\\"foo\"", 1));
	TEST(check_match(peg_json_decode, "\"foo\"", 1));
	TEST(check_match(peg_json_decode, "[null,null]", 1));
	TEST(check_match(peg_json_decode, "[]", 1));
	TEST(check_match(peg_json_decode, "[  ]", 1));
	TEST(check_match(peg_json_decode, "{}", 1));
	TEST(check_match(peg_json_decode, "{ }", 1));
	TEST(check_match(peg_json_decode, "1", 1));
	TEST(check_match(peg_json_decode, "-1", 1));
	TEST(check_match(peg_json_decode, "[1, 1.0, 2.3333, 1e+10, 1e-10, 1e10]", 1));
	TEST(check_match(peg_json_decode, "{\"\":null,\"\":null}", 1));
	TEST(check_match(peg_json_decode, "{\"foo\":null}", 1));
	TEST(check_match(peg_json_decode, "[\"Да Му Еба Майката\"]", 1));
}
