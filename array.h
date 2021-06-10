/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2019 Tobias Kortkamp <tobik@FreeBSD.org>
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
struct ArrayIterator;
struct diff;
struct Mempool;
typedef int (*ArrayCompareFn)(const void *, const void *, void *);

struct Array *array_new(void);
void array_append(struct Array *, const void *);
struct diff *array_diff(struct Array *, struct Array *, struct Mempool *, ArrayCompareFn, void *);
void array_free(struct Array *);
void *array_get(struct Array *, size_t);
ssize_t array_find(struct Array *, const void *, ArrayCompareFn, void *);
size_t array_len(struct Array *);
void *array_pop(struct Array *);
void array_set(struct Array *, size_t, const void *);
void array_sort(struct Array *, ArrayCompareFn, void *);
void array_truncate(struct Array *);
void array_truncate_at(struct Array *array, size_t);

struct ArrayIterator *array_iterator(struct Array *);
void array_iterator_free(struct ArrayIterator **);
void *array_iterator_next(struct ArrayIterator **, size_t *);

#define ARRAY_FOREACH(ARRAY, TYPE, VAR) \
	for (struct ArrayIterator *__##VAR##_iter __cleanup(array_iterator_free) = array_iterator(ARRAY); __##VAR##_iter != NULL; array_iterator_free(&__##VAR##_iter)) \
	for (size_t VAR##_index = 0; __##VAR##_iter != NULL; array_iterator_free(&__##VAR##_iter)) \
	for (TYPE VAR = array_iterator_next(&__##VAR##_iter, &VAR##_index); __##VAR##_iter != NULL; VAR = array_iterator_next(&__##VAR##_iter, &VAR##_index))
