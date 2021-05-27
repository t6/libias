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

#include "stack.h"
#include "str.h"
#include "test.h"
#include "util.h"

TESTS() {
	struct Stack *stack = stack_new();
	stack_push(stack, "1");
	stack_push(stack, "2");
	stack_push(stack, "3");
	TEST(stack_len(stack) == 3);
	TEST_STREQ(stack_peek(stack), "3");
	TEST_STREQ(stack_pop(stack), "3");
	TEST(stack_len(stack) == 2);
	TEST_STREQ(stack_pop(stack), "2");
	TEST(stack_len(stack) == 1);
	TEST_STREQ(stack_pop(stack), "1");
	TEST(stack_len(stack) == 0);
	TEST(stack_pop(stack) == NULL);

	stack_push(stack, "1");
	stack_push(stack, "2");
	stack_push(stack, "3");
	size_t i = 0;
	STACK_FOREACH(stack, const char *, s) {
		switch(s_index) {
		case 0: TEST_STREQ(s, "3"); break;
		case 1: TEST_STREQ(s, "2"); break;
		case 2: TEST_STREQ(s, "1"); break;
		default: TEST(0); break;
		}
		i++;
	}
	TEST(i == 3);
}
