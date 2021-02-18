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

struct Array;
struct Map;
struct Stack;

enum JSONType {
	JSON_ARRAY,
	JSON_FALSE,
	JSON_NULL,
	JSON_NUMBER_FLOAT,
	JSON_NUMBER_INT,
	JSON_NUMBER_UNREPRESENTABLE,
	JSON_OBJECT,
	JSON_STRING,
	JSON_TRUE,
};

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

int peg_json_decode(struct PEG *);
