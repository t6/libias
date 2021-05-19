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

#include "map.h"
#include "test.h"
#include "str.h"
#include "util.h"

TESTS() {
	struct Map *map = map_new(str_compare, NULL, NULL, NULL);
	map_add(map, "1", "foo");
	map_add(map, "2", "bar");

	TEST(map_len(map) == 2);

	size_t i = 0;
	MAP_FOREACH(map, char *, key, char *, value) {
		TEST(i == key_index);
		switch (i++) {
		case 0:
			TEST_STREQ(key, "1");
			TEST_STREQ(value, "foo");
			break;
		case 1:
			TEST_STREQ(key, "2");
			TEST_STREQ(value, "bar");
			break;
		}
	}
	TEST(i == 2);

	i = 0;
	MAP_FOREACH(map, char *, key, char *, value) {
		i++;
		break;
	}
	TEST(i == 1);

	i = 0;
	map = map_new(NULL, NULL, NULL, NULL);
	MAP_FOREACH(map, char *, key, char *, value) {
		i++;
	}
	TEST(i == 0);
}
