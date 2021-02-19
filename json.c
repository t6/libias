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

#include "array.h"
#include "json.h"
#include "map.h"
#include "peg.h"
#include "peg/json.h"

struct JSON {
	enum JSONType type;
	union {
		struct Array *array;
		union {
			long long i;
			double f;
			char *u;
		} number;
		struct Map *object;
		struct PEGCapture *string;
	};
};

struct JSONCaptureMachineData {
	struct JSON *json;
	struct {
		char *number;
		char *minus;
		char *integer;
		char *fraction;
		char *exponent;
	} num;
	struct Stack *arrays;
	struct Stack *objects;
	struct Stack *values;
};

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

enum PEGCaptureFlag
json_capture_machine(struct MemoryPool *pool, struct PEGCapture *capture, void *userdata)
{
	struct JSONCaptureMachineData *data = userdata;
	if (data->values == NULL) {
		data->arrays = GC(stack_new(), stack_free);
		data->objects = GC(stack_new(), stack_free);
		data->values = GC(stack_new(), stack_free);
	}
	switch (capture->state) {
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

