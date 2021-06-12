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

struct LineIterator;
struct Mempool;

struct LineIterator *line_iterator(FILE *);
void line_iterator_free(struct LineIterator **);
char *line_iterator_next(struct LineIterator **, size_t *, size_t *);
char *slurp(FILE *, struct Mempool *);

#define LINE_FOREACH(FILE, VAR) \
	for (struct LineIterator *__##VAR##_iter __cleanup(line_iterator_free) = line_iterator(FILE); __##VAR##_iter != NULL; line_iterator_free(&__##VAR##_iter)) \
	for (size_t VAR##_index = 0, VAR##_len = 0; __##VAR##_iter != NULL; line_iterator_free(&__##VAR##_iter)) \
	for (char *VAR = line_iterator_next(&__##VAR##_iter, &VAR##_index, &VAR##_len); __##VAR##_iter != NULL; VAR = line_iterator_next(&__##VAR##_iter, &VAR##_index, &VAR##_len))
