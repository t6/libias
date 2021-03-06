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
struct Mempool;

char *str_common_prefix(struct Mempool *, const char *, const char *);
int str_casecompare(const void *, const void *, void *);
int str_compare(const void *, const void *, void *);
char *str_dup(struct Mempool *, const char *);
char *str_ndup(struct Mempool *, const char *, size_t);
int str_endswith(const char *, const char *);
char *str_join(struct Mempool *, struct Array *, const char *);
char *str_map(struct Mempool *, const char *, size_t, int (*)(int));
char *str_printf(struct Mempool *, const char *, ...) __printflike(2, 3);
char *str_repeat(struct Mempool *, const char *, const size_t);
char *str_slice(struct Mempool *, const char *, const ssize_t, const ssize_t);
int str_startswith(const char *, const char *);
char *str_trim(struct Mempool *, const char *);
char *str_triml(struct Mempool *, const char *);
char *str_trimr(struct Mempool *, const char *);
