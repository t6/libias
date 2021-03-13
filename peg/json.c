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

#include "peg.h"
#include "peg/json.h"
#include "peg/grammar.h"

// See https://tools.ietf.org/html/rfc7159

//
static RULE(JSON);
static RULE(array);
static RULE(array_0);
static RULE(array_1);
static RULE(array_values);
static RULE(array_values_0);
static RULE(begin_array);
static RULE(begin_object);
static RULE(character);
static RULE(decimal_point);
static RULE(digit);
static RULE(e);
static RULE(end_array);
static RULE(end_object);
static RULE(escape);
static RULE(escaped);
static RULE(escaped_0);
static RULE(escaped_1);
static RULE(exponent);
static RULE(fraction);
static RULE(hexdigit);
static RULE(integer);
static RULE(member);
static RULE(minus);
static RULE(minus_plus);
static RULE(name_separator);
static RULE(number);
static RULE(object);
static RULE(object_0);
static RULE(object_1);
static RULE(object_members);
static RULE(object_members_0);
static RULE(plus);
static RULE(quotation_mark);
static RULE(string);
static RULE(unescaped);
static RULE(value);
static RULE(value_separator);
static RULE(ws);
static RULE(zero);

static int
ishexdigit(int c)
{
	c = tolower(c);
	return isdigit(c) || (c >= 'a' && c <= 'f');
}

RULE(JSON) {
	if (ANY(ws))
	if (MATCH(value))
	if (ANY(ws))
	return ACCEPT;
	return REJECT;
}

RULE(ws) { return SET(0x20, 0x09, 0x0A, 0x0D); }

RULE(begin_array) {
	if (CHAR('['))
	return ACCEPT;
	return ERROR("expected beginning of array '['");
}

RULE(begin_object) {
	if (CHAR('{'))
	return ACCEPT;
	return ERROR("expected beginning of object '{'");
}

RULE(end_array) {
	if (CHAR(']'))
	return ACCEPT;
	return ERROR("expected end of array ']'");
}

RULE(end_object) {
	if (CHAR('}'))
	return ACCEPT;
	return ERROR("expected end of object '}'");
}

RULE(name_separator) {
	if (CHAR(':'))
	return ACCEPT;
	return ERROR("expected name separator ':'");
}

RULE(value_separator) {
	if (CHAR(','))
	return ACCEPT;
	return ERROR("expected value separator ','");
}

RULE(escape) {
	if (CHAR('\\'))
	return ACCEPT;
	return ERROR("expected escape '\\'");
}

RULE(quotation_mark) {
	if (CHAR('"'))
	return ACCEPT;
	return ERROR("expected '\"'");
}

RULE(decimal_point) {
	if (CHAR('.'))
	return ACCEPT;
	return ERROR("expected '.'");
}

RULE(minus) {
	if (CHAR('-'))
	return ACCEPT;
	return ERROR("expected '-'");
}

RULE(plus) {
	if (CHAR('+'))
	return ACCEPT;
	return ERROR("expected '+'");
}

RULE(zero) {
	if (CHAR('0'))
	return ACCEPT;
	return ERROR("expected '0'");
}

RULE(digit) {
	if (CHARF(isdigit))
	return ACCEPT;
	return ERROR("expected decimal digit");
}

RULE(hexdigit) {
	if (CHARF(ishexdigit))
	return ACCEPT;
	return ERROR("expected hexadecimal digit");
}

RULE(value) {
	if (!CAPTURE(STRING("false"), 0, PEG_JSON_FALSE))
	if (!CAPTURE(STRING("null"), 0, PEG_JSON_NULL))
	if (!CAPTURE(STRING("true"), 0, PEG_JSON_TRUE))
	if (!MATCH(object))
	if (!MATCH(array))
	if (!CAPTURE(MATCH(number), 0, PEG_JSON_NUMBER))
	if (!MATCH(string))
	return ERROR("");
	return ACCEPT;
}

RULE(object_members_0) {
	if (MATCH(value_separator))
	if (ANY(ws))
	if (MATCH(end_object))
	return ACCEPT;
	return REJECT;
}

RULE(object_members) {
	if (ANY(ws))
	if (!LOOKAHEAD(object_members_0))
	if (MATCH(value_separator))
	if (ANY(ws))
	if (MATCH(member))
	return ACCEPT;
	return REJECT;
}

RULE(object_0) {
	if (CAPTURE(MATCH(begin_object), 0, PEG_JSON_OBJECT_BEGIN))
	if (ANY(ws))
	if (MATCH(member))
	if (ANY(object_members))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_object), 0, PEG_JSON_OBJECT_END))
	return ACCEPT;
	return REJECT;
}

RULE(object_1) {
	if (CAPTURE(MATCH(begin_object), 0, PEG_JSON_OBJECT_BEGIN))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_object), 0, PEG_JSON_OBJECT_END))
	return ACCEPT;
	return REJECT;
}

RULE(object) {
	if (!MATCH(object_0))
	if (!MATCH(object_1))
	return ERROR("");
	return ACCEPT;
}

RULE(member) {
	if (MATCH(string))
	if (ANY(ws))
	if (MATCH(name_separator))
	if (ANY(ws))
	if (CAPTURE(MATCH(value), 0, PEG_JSON_OBJECT_VALUE))
	return ACCEPT;
	return REJECT;
}

RULE(array_values_0) {
	if (MATCH(value_separator))
	if (ANY(ws))
	if (MATCH(end_array))
	return ACCEPT;
	return REJECT;
}

RULE(array_values) {
	if (ANY(ws))
	if (!LOOKAHEAD(array_values_0))
	if (MATCH(value_separator))
	if (ANY(ws))
	if (CAPTURE(MATCH(value), 0, PEG_JSON_ARRAY_VALUE))
	return ACCEPT;
	return REJECT;
}

RULE(array_0) {
	if (CAPTURE(MATCH(begin_array), 0, PEG_JSON_ARRAY_BEGIN))
	if (ANY(ws))
	if (CAPTURE(MATCH(value), 0, PEG_JSON_ARRAY_VALUE))
	if (ANY(array_values))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_array), 0, PEG_JSON_ARRAY_END))
	return ACCEPT;
	return REJECT;
}

RULE(array_1) {
	if (CAPTURE(MATCH(begin_array), 0, PEG_JSON_ARRAY_BEGIN))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_array), 0, PEG_JSON_ARRAY_END))
	return ACCEPT;
	return REJECT;
}

RULE(array) {
	if (!MATCH(array_0))
	if (!MATCH(array_1))
	return ERROR("");
	return ACCEPT;
}

RULE(integer) {
	if (MATCH(zero)) return ACCEPT;
	if (RANGE('1', '9'))
	if (ANY(digit))
	return ACCEPT;
	return REJECT;
}

RULE(e) {
	if (!CHAR('e'))
	if (!CHAR('E'))
	return REJECT;
	return ACCEPT;
}

RULE(minus_plus) {
	if (!MATCH(minus))
	if (!MATCH(plus))
	return REJECT;
	return ACCEPT;
}

RULE(exponent) {
	if (MATCH(e))
	if (OPT(MATCH(minus_plus)))
	if (SOME(digit))
	return ACCEPT;
	return REJECT;
}

RULE(fraction) {
	if (MATCH(decimal_point))
	if (SOME(digit))
	return ACCEPT;
	return REJECT;
}

RULE(number) {
	if (OPT(MATCH(minus)))
	if (MATCH(integer))
	if (OPT(MATCH(fraction)))
	if (OPT(MATCH(exponent)))
	return ACCEPT;
	return ERROR("");
}

RULE(unescaped) {
	if (!CHAR(0x20))
	if (!CHAR(0x21))
	if (!RANGE(0x23, 0x5b))
	if (!RANGE(0x5d, 0x10ffff))
	return REJECT;
	return ACCEPT;
}

RULE(escaped_1) {
	if (CHAR(0x75))
	if (REPEAT(hexdigit, 4))
	return ACCEPT;
	return ERROR("invalid unicode escape");
}

RULE(escaped_0) {
	if (!SET(0x22, 0x5c, 0x2f, 0x62, 0x66, 0x6e, 0x72, 0x74))
	if (!MATCH(escaped_1))
	return REJECT;
	return ACCEPT;
}

RULE(escaped) {
	if (MATCH(escape))
	if (MATCH(escaped_0))
	return ACCEPT;
	return REJECT;
}

RULE(character) {
	if (!MATCH(unescaped))
	if (!MATCH(escaped))
	return REJECT;
	return ACCEPT;
}

RULE(string) {
	if (MATCH(quotation_mark))
	if (CAPTURE(ANY(character), 0, PEG_JSON_STRING))
	if (MATCH(quotation_mark))
	return ACCEPT;
	return ERROR("invalid string");
}

RULE(peg_json_decode) {
	if (MATCH(JSON))
	if (EOS())
	return ACCEPT;
	return REJECT;
}
