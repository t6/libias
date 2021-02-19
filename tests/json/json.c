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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "map.h"
#include "json.h"
#include "test.h"
#include "util.h"

// static int
// test_json_decode(const char *s, void *data)
// {
// 	struct PEG *peg = peg_new(s, strlen(s));
// 	return peg_match(peg, peg_json_decode, json_capture_machine, data);
// }

TESTS() {
	const char *buf;
	const char *s;
	struct JSON *json;
	struct Array *array;
	struct Map *object;

	// int fd = open("pkg-status.json", O_RDONLY);
	// buf = slurp(fd);
	// json = json_new(buf, strlen(buf));
	// TEST(json);

	buf = "[\"foo\",\"bar\"]";
	json = json_new(buf, strlen(buf));
	TEST_IF(json) {
	TEST_IF(json_type(json) == JSON_ARRAY) {
	array = json_unwrap_array(json);
	TEST_IF(array_len(array) == 2) {
	s = json_unwrap_string(array_get(array, 0));
	TEST_IF(s) {
	TEST_STREQ(s, "foo");
	s = json_unwrap_string(array_get(array, 1));
	TEST_IF(s) {
	TEST_STREQ(s, "bar");
	}}}}}

	buf = "[[1],[2,[3,[4]]]]";
	json = json_new(buf, strlen(buf));
	TEST_IF(json && json_type(json) == JSON_ARRAY) {
	array = json_unwrap_array(json);
	TEST_IF(array_len(array) == 2) {
	array = json_unwrap_array(array_get(array, 0));
	TEST_IF(array && json_type(json) == JSON_ARRAY && array_len(array) == 1) {
	TEST(json_type(array_get(array, 0)) == JSON_NUMBER_INT);
	buf = "{\"1\":[true,null,false]}";
	json = json_new(buf, strlen(buf));
	TEST_IF(json && json_type(json) == JSON_OBJECT) {
	object = json_unwrap_object(json);
	TEST(map_contains(object, (char *)"1"));
	json = map_get(object, (char *)"1");
	TEST_IF(json && json_type(json) == JSON_ARRAY) {
	array = json_unwrap_array(json);
	TEST(array_len(array) == 3);
	TEST(json_type(array_get(array, 0)) == JSON_TRUE);
	TEST(json_type(array_get(array, 1)) == JSON_NULL);
	TEST(json_type(array_get(array, 2)) == JSON_FALSE);
	}}}}}
}
