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

#define TEST_OK() \
do { \
	putchar('.'); \
	fflush(stdout); \
} while(0)

#define TEST_FAIL_LOC(file, line, col, msg) \
do { \
	if (failureslen < nitems(failures)) { \
		char *buf = str_printf("%s:%d:%d: FAIL: %s", file, line, col, msg); \
		failures[failureslen++] = buf; \
	} \
	putchar('X'); \
	fflush(stdout); \
} while(0)

#define TEST_FAIL(msg) TEST_FAIL_LOC(__FILE__, __LINE__, 1, msg)

#define TEST(x) \
if ((x)) { \
	TEST_OK(); \
} else { \
	TEST_FAIL(#x); \
}

#define TEST_IF(x) \
if (!(x)) { \
	TEST_FAIL(#x); \
} else if ((putchar('.') || 1) && (fflush(stdout) || 1))

#define TEST_STR(cmp, a, b) \
do { \
	const char *s1 = (a); \
	const char *s2 = (b); \
	if (s1 && s2 && strcmp(s1, s2) cmp 0) { \
		TEST_OK(); \
	} else { \
		TEST_FAIL(""#a" "#cmp" "#b""); \
	} \
} while(0)

#define TEST_STREQ(a, b) TEST_STR(==, a, b)
#define TEST_STRNEQ(a, b) TEST_STR(!=, a, b)

#define TESTS() \
static char *failures[512]; \
static size_t failureslen; \
static void run_tests(void); \
int main(int argc, char *argv[]) { \
	run_tests(); \
	printf("\n"); \
	for (size_t i = 0; i < failureslen; i++) { \
		puts(failures[i]); \
	} \
	return failureslen != 0; \
} \
void run_tests()
