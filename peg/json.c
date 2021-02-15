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
#include "peg-macros.h"
#include "util.h"

// See https://tools.ietf.org/html/rfc7159

static int ishexdigit(int c)
{
	c = tolower(c);
	return isdigit(c) || (c >= 'a' && c <= 'f');
}

RULE(array);
RULE(member);
RULE(number);
RULE(object);
RULE(string);
RULE(value);
RULE(ws);

RULE(JSON) {
	if (ANY(ws))
	if (MATCH(value))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(ws) { return CHARF(isspace); }

RULE(begin_array) { return CHAR('['); }
RULE(begin_object) { return CHAR('{'); }
RULE(end_array) { return CHAR(']'); }
RULE(end_object) { return CHAR('}'); }
RULE(name_separator) { return CHAR(':'); }
RULE(value_separator) { return CHAR(','); }
RULE(escape) { return CHAR('\\'); }
RULE(quotation_mark) { return CHAR('"'); }
RULE(decimal_point) { return CHAR('.'); }
RULE(minus) { return CHAR('-'); }
RULE(plus) { return CHAR('+'); }
RULE(zero) { return CHAR('0'); }
RULE(digit) { return CHARF(isdigit); }
RULE(hexdigit) { return CHARF(ishexdigit);}

RULE(value) {
	if (!STRING("false"))
	if (!STRING("null"))
	if (!STRING("true"))
	if (!MATCH(object))
	if (!MATCH(array))
	if (!MATCH(number))
	if (!MATCH(string))
	return 0;
	return 1;
}

RULE(object_members) {
	if (ANY(ws))
	if (MATCH(value_separator))
	if (ANY(ws))
	if (MATCH(member))
	return 1;
	return 0;
}

RULE(object_0) {
	if (MATCH(begin_object))
	if (ANY(ws))
	if (MATCH(member))
	if (ANY(object_members))
	if (ANY(ws))
	if (MATCH(end_object))
	return 1;
	return 0;
}

RULE(object_1) {
	if (MATCH(begin_object))
	if (ANY(ws))
	if (MATCH(end_object))
	return 1;
	return 0;
}

RULE(object) {
	if (!MATCH(object_0))
	if (!MATCH(object_1))
	return 0;
	return 1;
}

RULE(member) {
	if (MATCH(string))
	if (MATCH(name_separator))
	if (MATCH(value))
	return 1;
	return 0;
}

RULE(array_values) {
	if (ANY(ws))
	if (MATCH(value_separator))
	if (ANY(ws))
	if (MATCH(value))
	return 1;
	return 0;
}

RULE(array_0) {
	if (MATCH(begin_array))
	if (ANY(ws))
	if (MATCH(value))
	if (ANY(array_values))
	if (ANY(ws))
	if (MATCH(end_array))
	return 1;
	return 0;
}

RULE(array_1) {
	if (MATCH(begin_array))
	if (ANY(ws))
	if (MATCH(end_array))
	return 1;
	return 0;
}

RULE(array) {
	if (!MATCH(array_0))
	if (!MATCH(array_1))
	return 0;
	return 1;
}

RULE(integer) {
	if (MATCH(zero)) return 1;
	if (RANGE('1', '9'))
	if (ANY(digit))
	return 1;
	return 0;
}

RULE(e) {
	if (!CHAR('e'))
	if (!CHAR('E'))
	return 0;
	return 1;
}

RULE(minus_plus) {
	if (!MATCH(minus))
	if (!MATCH(plus))
	return 0;
	return 1;
}

RULE(exp) {
	if (MATCH(e))
	if (OPT(MATCH(minus_plus)))
	if (SOME(digit))
	return 1;
	return 0;
}

RULE(frac) {
	if (MATCH(decimal_point))
	if (SOME(digit))
	return 1;
	return 0;
}

RULE(number) {
	if (OPT(MATCH(minus)))
	if (MATCH(integer))
	if (OPT(MATCH(frac)))
	if (OPT(MATCH(exp)))
	return 1;
	return 0;
}

RULE(unescaped) {
	if (!CHAR(0x20))
	if (!CHAR(0x21))
	if (!RANGE(0x23, 0x5b))
	if (!RANGE(0x5d, 0x10ffff))
	return 0;
	return 1;
}

RULE(escaped_1) {
	if (CHAR(0x75))
	if (REPEAT(hexdigit, 4))
	return 1;
	return 0;
}

RULE(escaped_0) {
	if (!SET(0x22, 0x5c, 0x2f, 0x62, 0x66, 0x6e, 0x72, 0x74))
	if (!MATCH(escaped_1))
	return 0;
	return 1;
}

RULE(escaped) {
	if (MATCH(escape))
	if (MATCH(escaped_0))
	return 1;
	return 0;
}

RULE(character) {
	if (!MATCH(unescaped))
	if (!MATCH(escaped))
	return 0;
	return 1;
}

RULE(string) {
	if (MATCH(quotation_mark))
	if (ANY(character))
	if (MATCH(quotation_mark))
	return 1;
	return 0;
}

MAIN_RULE(json_decode) {
	if (MATCH(JSON))
	if (EOS())
	return 1;
	return 0;
}
