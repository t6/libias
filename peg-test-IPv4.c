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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "peg.h"
#include "util.h"

RULE(dig) { return RANGE('0', '9'); }
RULE(num0_4) { return RANGE('0', '4'); }
RULE(num0_5) { return RANGE('0', '5'); }
RULE(byte_1) { if (STRING("25")) if (MATCH(num0_5)) return 1; return 0; }
RULE(byte_2) { if (CHAR('2')) if (MATCH(num0_4)) if (MATCH(dig)) return 1; return 0; }
RULE(byte_3) { if (CHAR('1')) if (MATCH(dig)) if (MATCH(dig)) return 1; return 0; }
RULE(byte) {
	if (!MATCH(byte_1))
	if (!MATCH(byte_2))
	if (!MATCH(byte_3))
	if (!BETWEEN(dig, 1, 2))
	return 0;
	return 1;
}
RULE(ipv4) {
	if (MATCH(byte))
	if (CHAR('.'))
	if (MATCH(byte))
	if (CHAR('.'))
	if (MATCH(byte))
	if (CHAR('.'))
	if (MATCH(byte))
	if (EOS())
	return 1;
	return 0;
}

static void
on_capture(struct PEG *peg, const char *rule, const char *buf, size_t len, void *userdata)
{
	(void)userdata; (void)peg;
	char *s = xstrndup(buf, len);
	printf("captured in %s: %s\n", rule, s);
	free(s);
}

static void
on_mismatch(struct PEG *peg, const char *rule, void *userdata)
{
	(void)userdata; (void)peg;
	printf("mismatch %s\n", rule);
}

int
main(int argc, char *argv[])
{
	const char *buf = "256.0.0.0";
	int state;
	struct PEG *peg = peg_new(buf, strlen(buf), on_capture, on_mismatch);
	state = peg_match_rule(peg, "IPv4", ipv4);
	peg_free(peg);
	return !state;
}
