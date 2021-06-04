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

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mempool.h"
#include "peg.h"
#include "peg/grammar.h"
#include "str.h"
#include "test.h"
#include "util.h"

static RULE(comment);
static RULE(digit);
static RULE(date);
static RULE(entry);
static RULE(entry_comment);
static RULE(MOVED);

RULE(comment) {
	if (CHAR('#'))
	if (THRU("\n"))
	return 1;
	return 0;
}

RULE(digit) {
	return CHARF(isdigit);
}

RULE(date) {
	if (REPEAT(digit, 4))
	if (CHAR('-'))
	if (REPEAT(digit, 2))
	if (CHAR('-'))
	if (REPEAT(digit, 2))
	return 1;
	return 0;
}

RULE(entry) {
	if (TO("|"))
	if (CHAR('|'))
	if (TO("|"))
	if (CHAR('|'))
	if (MATCH(date))
	if (CHAR('|'))
	if (TO("\n"))
	if (CHAR('\n'))
	return 1;
	return 0;
}

RULE(entry_comment) { // (+ :comment :entry)
	if (!MATCH(comment))
	if (!MATCH(entry))
	return 0;
	return 1;
}

RULE(MOVED) {
	if (ANY(entry_comment))
	if (EOS())
	return 1;
	return 0;
}

static int
check_match(const char *s, int expected)
{
	struct PEG *peg = peg_new(s, strlen(s));
	int result = peg_match(peg, MOVED, NULL, NULL);
	peg_free(peg);
	return result == expected;
}

TESTS() {
	const char * buf = "#\naudio/polypaudio|audio/pulseaudio|2008-01-01|Project renamed\n# asdjflasdk\n"
	"audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n"
	"audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n";

	TEST(check_match(buf, 1));
	TEST(check_match("", 1));
	TEST(check_match("foo", 0));
}
