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

struct Stack;
struct StackIterator;

struct Stack *stack_new(void);
void stack_free(struct Stack *);
size_t stack_len(struct Stack *);
int stack_contains(struct Stack *, const void *);
void *stack_peek(struct Stack *);
void *stack_pop(struct Stack *);
void stack_push(struct Stack *, const void *);
void stack_truncate(struct Stack *);

struct StackIterator *stack_iterator(struct Stack *);
void stack_iterator_free(struct StackIterator **);
void *stack_iterator_next(struct StackIterator **, size_t *);

#define STACK_FOREACH(STACK, TYPE, VAR) \
	for (struct StackIterator *__##VAR##_iter __cleanup(stack_iterator_free) = stack_iterator(STACK); __##VAR##_iter != NULL; stack_iterator_free(&__##VAR##_iter)) \
	for (size_t VAR##_index = 0; __##VAR##_iter != NULL; stack_iterator_free(&__##VAR##_iter)) \
	for (TYPE VAR = stack_iterator_next(&__##VAR##_iter, &VAR##_index); __##VAR##_iter != NULL; VAR = stack_iterator_next(&__##VAR##_iter, &VAR##_index))
