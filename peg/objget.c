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

#include "peg.h"
#include "peg/macros.h"
#include "peg/objget.h"

//
static RULE(character);
static RULE(digit);
static RULE(escaped);
static RULE(index_key);
static RULE(key);
static RULE(keys);
static RULE(string_key);

RULE(keys) {
	if (CHAR('.'))
	if (MATCH(key))
	return 1;
	return 0;
}

RULE(digit) {
	if (CHARF(isdigit))
	return 1;
	return 0;
}

RULE(index_key) {
	if (SOME(digit))
	return 1;
	return 0;
}

static int
isany(int c)
{
	return c != '.';
}

RULE(escaped) {
	if (CHAR('\\'))
	if (SET('.', '\\'))
	return 1;
	return 0;
}

RULE(character) {
	if (!MATCH(escaped))
	if (!CHARF(isany))
	return 0;
	return 1;
}

RULE(string_key) {
	if (SOME(character))
	return 1;
	return 0;
}

RULE(key) {
	if (!CAPTURE(MATCH(index_key), 0, PEG_OBJGET_INDEX))
	if (!CAPTURE(MATCH(string_key), 0, PEG_OBJGET_KEY))
	return 0;
	return 1;
}

RULE(peg_objget_decode) {
	if (MATCH(key))
	if (ANY(keys))
	if (EOS())
	return 1;
	return 0;
}
