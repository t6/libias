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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "map.h"
#include "memorypool.h"
#include "peg.h"
#include "peg/json.h"
#include "peg/macros.h"
#include "stack.h"
#include "util.h"

// See https://tools.ietf.org/html/rfc7159

static RULE(array_0);
static RULE(array_1);
static RULE(array_values);
static RULE(array);
static RULE(begin_array);
static RULE(begin_object);
static RULE(character);
static RULE(decimal_point);
static RULE(digit);
static RULE(e);
static RULE(end_array);
static RULE(end_object);
static RULE(escape);
static RULE(escaped_0);
static RULE(escaped_1);
static RULE(escaped);
static RULE(exponent);
static RULE(fraction);
static RULE(hexdigit);
static RULE(integer);
static RULE(JSON);
static RULE(member);
static RULE(minus_plus);
static RULE(minus);
static RULE(name_separator);
static RULE(number);
static RULE(object_0);
static RULE(object_1);
static RULE(object_members);
static RULE(object);
static RULE(plus);
static RULE(quotation_mark);
static RULE(string);
static RULE(unescaped);
static RULE(value_separator);
static RULE(value);
static RULE(ws);
static RULE(zero);

enum JSONCaptureState {
	ACCEPT = 0,
	CAPTURE_ARRAY_BEGIN,
	CAPTURE_ARRAY_END,
	CAPTURE_ARRAY_VALUE,
	CAPTURE_FALSE,
	CAPTURE_NULL,
	CAPTURE_OBJECT_BEGIN,
	CAPTURE_OBJECT_END,
	CAPTURE_OBJECT_VALUE,
	CAPTURE_STRING,
	CAPTURE_TRUE,
	NUMBER_EXPONENT,
	NUMBER_FRACTION,
	NUMBER_INTEGER,
	NUMBER_MINUS,
	NUMBER_FULL,
};

static int
ishexdigit(int c)
{
	c = tolower(c);
	return isdigit(c) || (c >= 'a' && c <= 'f');
}

static int
peg_capture_str_compare(const void *ap, const void *bp, void *userdata)
{
	struct PEGCapture *a = *(struct PEGCapture **)ap;
	struct PEGCapture *b = *(struct PEGCapture **)bp;
	if (a->len > b->len) {
		return 1;
	} else if (a->len < b->len) {
		return -1;
	} else {
		return strncmp(a->buf, b->buf, a->len);
	}
}

CAPTURE_MACHINE(enum JSONCaptureState, struct JSONCaptureMachineData) {
	if (data->values == NULL) {
		data->arrays = GC(stack_new(), stack_free);
		data->objects = GC(stack_new(), stack_free);
		data->values = GC(stack_new(), stack_free);
	}
	switch (state) {
	case ACCEPT: {
		assert(stack_len(data->values) == 1);
		assert(stack_len(data->objects) == 0);
		assert(stack_len(data->arrays) == 0);
		data->json = stack_pop(data->values);
		break;
	} case CAPTURE_ARRAY_BEGIN:
		stack_push(data->arrays, GC(array_new(), array_free));
		break;
	case CAPTURE_ARRAY_END: {
		struct Array *array = stack_pop(data->arrays);
		struct JSON *value = GC(xmalloc(sizeof(struct JSON)), free);
		value->type = JSON_ARRAY;
		value->array = array;
		stack_push(data->values, value);
		break;
	} case CAPTURE_ARRAY_VALUE: {
		struct JSON *value = stack_pop(data->values);
		struct Array *array = stack_peek(data->arrays);
		array_append(array, value);
		break;
	} case CAPTURE_OBJECT_BEGIN:
		stack_push(data->objects, GC(map_new(peg_capture_str_compare, NULL, NULL, NULL), map_free));
		break;
	case CAPTURE_OBJECT_END: {
		struct Map *object = stack_pop(data->objects);
		struct JSON *value = GC(xmalloc(sizeof(struct JSON)), free);
		value->type = JSON_OBJECT;
		value->object = object;
		stack_push(data->values, value);
		break;
	} case CAPTURE_OBJECT_VALUE: {
		struct JSON *value = stack_pop(data->values);
		struct JSON *key = stack_pop(data->values);
		struct Map *object = stack_peek(data->objects);
		map_add(object, key->string, value);
		break;
	} case CAPTURE_FALSE: {
		struct JSON *value = GC(xmalloc(sizeof(struct JSON)), free);
		value->type = JSON_FALSE;
		stack_push(data->values, value);
		break;
	} case CAPTURE_NULL: {
		struct JSON *value = GC(xmalloc(sizeof(struct JSON)), free);
		value->type = JSON_NULL;
		stack_push(data->values, value);
		break;
	} case CAPTURE_STRING: {
		struct JSON *value = GC(xmalloc(sizeof(struct JSON)), free);
		value->type = JSON_STRING;
		value->string = capture;
		stack_push(data->values, value);
		break;
	} case CAPTURE_TRUE: {
		struct JSON *value = GC(xmalloc(sizeof(struct JSON)), free);
		value->type = JSON_TRUE;
		stack_push(data->values, value);
		break;
	} case NUMBER_EXPONENT:
		//data->num.exponent = capture->buf;
		break;
	case NUMBER_FRACTION:
		//data->num.fraction = capture->buf;
		break;
	case NUMBER_INTEGER:
		//data->num.integer = capture->buf;
		break;
	case NUMBER_MINUS:
		//data->num.minus = capture->buf;
		break;
	case NUMBER_FULL: {
		struct JSON *value = GC(xmalloc(sizeof(struct JSON)), free);
		value->type = JSON_NUMBER_INT;
		value->number.i = 1;
		stack_push(data->values, value);
		memset(&data->num, 0, sizeof(data->num));
		break;
	} }

	return PEG_CAPTURE_DISCARD;
}

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
	if (!CAPTURE(STRING("false"), 0, CAPTURE_FALSE))
	if (!CAPTURE(STRING("null"), 0, CAPTURE_NULL))
	if (!CAPTURE(STRING("true"), 0, CAPTURE_TRUE))
	if (!MATCH(object))
	if (!MATCH(array))
	if (!CAPTURE(MATCH(number), 0, NUMBER_FULL))
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
	if (CAPTURE(MATCH(begin_object), 0, CAPTURE_OBJECT_BEGIN))
	if (ANY(ws))
	if (MATCH(member))
	if (ANY(object_members))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_object), 0, CAPTURE_OBJECT_END))
	return 1;
	return 0;
}

RULE(object_1) {
	if (CAPTURE(MATCH(begin_object), 0, CAPTURE_OBJECT_BEGIN))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_object), 0, CAPTURE_OBJECT_END))
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
	if (CAPTURE(MATCH(value), 0, CAPTURE_OBJECT_VALUE))
	return 1;
	return 0;
}

RULE(array_values) {
	if (ANY(ws))
	if (MATCH(value_separator))
	if (ANY(ws))
	if (CAPTURE(MATCH(value), 0, CAPTURE_ARRAY_VALUE))
	return 1;
	return 0;
}

RULE(array_0) {
	if (CAPTURE(MATCH(begin_array), 0, CAPTURE_ARRAY_BEGIN))
	if (ANY(ws))
	if (CAPTURE(MATCH(value), 0, CAPTURE_ARRAY_VALUE))
	if (ANY(array_values))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_array), 0, CAPTURE_ARRAY_END))
	return 1;
	return 0;
}

RULE(array_1) {
	if (CAPTURE(MATCH(begin_array), 0, CAPTURE_ARRAY_BEGIN))
	if (ANY(ws))
	if (CAPTURE(MATCH(end_array), 0, CAPTURE_ARRAY_END))
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

RULE(exponent) {
	if (MATCH(e))
	if (OPT(MATCH(minus_plus)))
	if (SOME(digit))
	return 1;
	return 0;
}

RULE(fraction) {
	if (MATCH(decimal_point))
	if (SOME(digit))
	return 1;
	return 0;
}

RULE(number) {
	if (OPT(CAPTURE(MATCH(minus), 0, NUMBER_MINUS)))
	if (CAPTURE(MATCH(integer), 0, NUMBER_INTEGER))
	if (OPT(CAPTURE(MATCH(fraction), 0, NUMBER_FRACTION)))
	if (OPT(CAPTURE(MATCH(exponent), 0, NUMBER_EXPONENT)))
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
	if (CAPTURE(ANY(character), 0, CAPTURE_STRING))
	if (MATCH(quotation_mark))
	return 1;
	return 0;
}

RULE(peg_json_decode) {
	if (MATCH(JSON))
	if (EOS())
	return 1;
	return 0;
}
