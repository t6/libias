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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "json.h"
#include "map.h"
#include "mempool.h"
#include "peg.h"
#include "peg/json.h"
#include "peg/objget.h"
#include "stack.h"
#include "str.h"
#include "util.h"

struct JSON {
	struct Mempool *pool;
	enum JSONType type;
	union {
		struct Array *array;
		struct Map *object;
		const char *string;
	};
};

struct JSONCaptureMachineData {
	struct JSON *json;
	char *buf;
	struct Mempool *pool;
	struct Stack *arrays;
	struct Stack *objects;
	struct Stack *values;
};

struct ObjgetCaptureMachineData {
	struct JSON *json;
};

static enum PEGCaptureFlag
json_capture_machine(struct PEGCapture *capture, void *userdata)
{
	struct JSONCaptureMachineData *data = userdata;
	switch ((enum PEGJSONCapture)capture->state) {
	case PEG_JSON_ACCEPT: {
		if (stack_len(data->values) == 1 &&
		    stack_len(data->objects) == 0 &&
		    stack_len(data->arrays) == 0) {
			data->json = stack_pop(data->values);
		} else {
			data->json = NULL;
		}
		break;
	} case PEG_JSON_ARRAY_BEGIN:
		stack_push(data->arrays, mempool_array(data->pool));
		break;
	case PEG_JSON_ARRAY_END: {
		struct Array *array = stack_pop(data->arrays);
		struct JSON *value = mempool_alloc(data->pool, sizeof(struct JSON));
		value->pool = data->pool;
		value->type = JSON_ARRAY;
		value->array = array;
		stack_push(data->values, value);
		break;
	} case PEG_JSON_ARRAY_VALUE: {
		struct JSON *value = stack_pop(data->values);
		struct Array *array = stack_peek(data->arrays);
		array_append(array, value);
		break;
	} case PEG_JSON_OBJECT_BEGIN:
		stack_push(data->objects, mempool_map(data->pool, str_compare, NULL, NULL, NULL));
		break;
	case PEG_JSON_OBJECT_END: {
		struct Map *object = stack_pop(data->objects);
		struct JSON *value = mempool_alloc(data->pool, sizeof(struct JSON));
		value->pool = data->pool;
		value->type = JSON_OBJECT;
		value->object = object;
		stack_push(data->values, value);
		break;
	} case PEG_JSON_OBJECT_VALUE: {
		struct JSON *value = stack_pop(data->values);
		struct JSON *key = stack_pop(data->values);
		struct Map *object = stack_peek(data->objects);
		map_add(object, (char *)key->string, value);
		break;
	} case PEG_JSON_FALSE: {
		struct JSON *value = mempool_alloc(data->pool, sizeof(struct JSON));
		value->pool = data->pool;
		value->type = JSON_FALSE;
		stack_push(data->values, value);
		break;
	} case PEG_JSON_NULL: {
		struct JSON *value = mempool_alloc(data->pool, sizeof(struct JSON));
		value->pool = data->pool;
		value->type = JSON_NULL;
		stack_push(data->values, value);
		break;
	} case PEG_JSON_STRING: {
		struct JSON *value = mempool_alloc(data->pool, sizeof(struct JSON));
		value->pool = data->pool;
		value->type = JSON_STRING;
		// Strings are always followed by ".  We can replace
		// it with 0 to terminate the string without
		// allocating more memory.
		// XXX: deal with unicode escapes etc.
		data->buf[capture->pos + capture->len] = 0;
		value->string = capture->buf;
		stack_push(data->values, value);
		break;
	} case PEG_JSON_TRUE: {
		struct JSON *value = mempool_alloc(data->pool, sizeof(struct JSON));
		value->pool = data->pool;
		value->type = JSON_TRUE;
		stack_push(data->values, value);
		break;
	} case PEG_JSON_NUMBER: {
		struct JSON *value = mempool_alloc(data->pool, sizeof(struct JSON));
		value->pool = data->pool;
		value->type = JSON_NUMBER;
		// Strings are always followed by ".  We can replace
		// it with 0 to terminate the string without
		// allocating more memory.
		data->buf[capture->pos + capture->len] = 0;
		value->string = capture->buf;
		stack_push(data->values, value);
		break;
	} }

	return PEG_CAPTURE_CONTINUE;
}

static enum PEGCaptureFlag
objget_capture_machine(struct PEGCapture *capture, void *userdata)
{
	SCOPE_MEMPOOL(pool);
	struct ObjgetCaptureMachineData *data = userdata;
	switch ((enum ObjgetCaptureState)capture->state) {
	case PEG_OBJGET_ACCEPT: {
		break;
	} case PEG_OBJGET_INDEX: {
		if (json_type(data->json) == JSON_ARRAY) {
			const char *error;
			char *buf = str_ndup(pool, capture->buf, capture->len);
			size_t i = strtonum(buf, 0, INT64_MAX, &error);
			if (error) {
				data->json = NULL;
			} else {
				data->json = array_get(json_unwrap_array(data->json), i);
			}
		} else if (json_type(data->json) == JSON_OBJECT) {
			char *key = str_ndup(pool, capture->buf, capture->len);
			data->json = map_get(json_unwrap_object(data->json), key);
		} else {
			data->json = NULL;
		}
		break;
	} case PEG_OBJGET_KEY: {
		if (json_type(data->json) == JSON_OBJECT) {
			char *key = xmalloc(capture->len + 1);
			char *ptr = key;
			int escape = 0;
			for (size_t i = 0; i < capture->len; i++) {
				char c = capture->buf[i];
				if (escape) {
					if (c == '.' || c == '\\') {
						*ptr++ = c;
					} else {
						abort();
					}
					escape = 0;
				} else if (c == '\\') {
					escape = 1;
				} else {
					*ptr++ = c;
				}
			}
			data->json = map_get(json_unwrap_object(data->json), key);
			free(key);
		} else {
			data->json = NULL;
		}
		break;
	} }

	if (data->json) {
		return PEG_CAPTURE_CONTINUE;
	} else {
		return PEG_CAPTURE_STOP;
	}
}

struct JSON *
json_new(const char *buf, size_t len)
{
	struct JSONCaptureMachineData data;
	memset(&data, 0, sizeof(data));
	data.pool = mempool_new();
	data.arrays = mempool_stack(data.pool);
	data.objects = mempool_stack(data.pool);
	data.values = mempool_stack(data.pool);
	data.buf = str_ndup(data.pool, buf, len);

	struct PEG *peg = peg_new(data.buf, len);
	int status = peg_match(peg, peg_json_decode, json_capture_machine, &data);
	peg_free(peg);

	if (status && data.json) {
		return data.json;
	} else {
		mempool_free(data.pool);
		return NULL;
	}
}

void
json_free(struct JSON *json)
{
	mempool_free(json->pool);
}

struct JSON *
json_get(struct JSON *json, const char *key)
{
	struct PEG *peg = peg_new(key, strlen(key));
	struct ObjgetCaptureMachineData data;
	data.json = json;
	int status = peg_match(peg, peg_objget_decode, objget_capture_machine, &data);
	peg_free(peg);
	if (status) {
		return data.json;
	} else {
		return NULL;
	}
}

enum JSONType
json_type(struct JSON *json)
{
	return json->type;
}

struct Array *
json_unwrap_array(struct JSON *json)
{
	if (json == NULL) {
		return NULL;
	} else if (json->type == JSON_ARRAY) {
		return json->array;
	} else {
		return NULL;
	}
}

const char *
json_unwrap_number(struct JSON *json)
{
	if (json == NULL) {
		return NULL;
	} else if (json->type == JSON_NUMBER) {
		return json->string;
	} else {
		return NULL;
	}
}

struct Map *
json_unwrap_object(struct JSON *json)
{
	if (json == NULL) {
		return NULL;
	} else if (json->type == JSON_OBJECT) {
		return json->object;
	} else {
		return NULL;
	}
}

const char *
json_unwrap_string(struct JSON *json)
{
	if (json == NULL) {
		return NULL;
	} else if (json->type == JSON_STRING) {
		return json->string;
	} else {
		return NULL;
	}
}
