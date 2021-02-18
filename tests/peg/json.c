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

#include "tests/peg/common.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "peg/json.h"

static int
test_json_decode(const char *s, void *data)
{
	struct PEG *peg = peg_new(s, strlen(s));
	return peg_match(peg, peg_json_decode, data);
}

TESTS() {
	struct JSONCaptureMachineData data;

	int fd = open("pkg-status.json", O_RDONLY);
	char *buf = slurp(fd);
	memset(&data, 0, sizeof(data));
	TEST_IF(test_json_decode(buf, &data)) {
	}

	memset(&data, 0, sizeof(data));
	TEST(test_json_decode("[\"foo\"]", &data));
	TEST(data.json);
	TEST(data.json->type == JSON_ARRAY);
	TEST(data.json->array && array_len(data.json->array) == 1);
	TEST(((struct JSON *)array_get(data.json->array, 0))->type == JSON_STRING);
	struct PEGCapture *s = ((struct JSON *)array_get(data.json->array, 0))->string;
	TEST(strncmp(s->buf, "foo", s->len) == 0);

	TEST(check_match(peg_json_decode, "[null]", 1));
	TEST(check_match(peg_json_decode, "\"\\\"foo\"", 1));
	TEST(check_match(peg_json_decode, "\"foo\"", 1));
	TEST_STREQ(check_captures(peg_json_decode, "{\"f\":[null,1]}", 1, "@"), "");
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

	memset(&data, 0, sizeof(data));
	TEST(test_json_decode("[[1],[2,[3,[4]]]]", &data));
	TEST(data.json);
	TEST(data.json->type == JSON_ARRAY);
	TEST(array_len(data.json->array) == 2);
	TEST(((struct JSON *)array_get(data.json->array, 0))->type == JSON_ARRAY);
	TEST(array_len(((struct JSON *)array_get(data.json->array, 0))->array) == 1);
	TEST(((struct JSON *)array_get(data.json->array, 1))->type == JSON_ARRAY);
	TEST(array_len(((struct JSON *)array_get(data.json->array, 1))->array) == 2);

	memset(&data, 0, sizeof(data));
	TEST(test_json_decode("[{\"1\":[true,null,false]}]", &data));
	TEST(data.json);
	TEST(data.json->type == JSON_ARRAY);
	TEST(array_len(data.json->array) == 1);
	TEST(((struct JSON *)array_get(data.json->array, 0))->type == JSON_OBJECT);
	struct Map *obj = ((struct JSON *)array_get(data.json->array, 0))->object;
	TEST(map_len(obj) == 1);
	//TEST(map_contains(obj, (char *)"1"));
	//struct Array *array = map_get(obj, (char *)"1");
	//TEST(array_len(array) == 3);
	// TEST(((struct JSON *)array_get(array, 0))->type == JSON_TRUE);
	// TEST(((struct JSON *)array_get(array, 1))->type == JSON_NULL);
	// TEST(((struct JSON *)array_get(array, 2))->type == JSON_FALSE);

}
