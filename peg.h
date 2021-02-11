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

struct PEG;

typedef void (*CaptureFn)(struct PEG *, const char *, const char *, size_t, void *);
typedef void (*MismatchFn)(struct PEG *, const char *, void *);
typedef int (*RuleFn)(struct PEG *, const char *);

struct PEG *peg_new(const char *, size_t, CaptureFn, MismatchFn);
void peg_free(struct PEG *);

int peg_match_atleast(struct PEG *, const char *, RuleFn, int);
int peg_match_between(struct PEG *, const char *, RuleFn, int, int);
int peg_match_char(struct PEG *, const char *, char);
int peg_match_char_f(struct PEG *, const char *, int (*)(int));
int peg_match_eos(struct PEG *, const char *);
int peg_match_range(struct PEG *, const char *, char, char);
int peg_match_repeat(struct PEG *, const char *, RuleFn, int);
int peg_match_rule(struct PEG *, const char *, RuleFn);
int peg_match_string(struct PEG *, const char *, const char *);
int peg_match_thru(struct PEG *, const char *, const char *);
int peg_match_to(struct PEG *, const char *, const char *);

#define ANY(r)			ATLEAST(r, 0)
#define ATLEAST(r, n)		peg_match_atleast(peg, __func__, r, n)
#define BETWEEN(r, a, b)	peg_match_between(peg, __func__, r, a, b)
#define CHAR(c)			peg_match_char(peg, __func__, c)
#define CHARF(f)		peg_match_char_f(peg, __func__, f)
#define EOS()			peg_match_eos(peg, __func__)
#define MATCH(r)		peg_match_rule(peg, __func__, r)
#define RANGE(a, b)		peg_match_range(peg, __func__, a, b)
#define REPEAT(r, n)		peg_match_repeat(peg, __func__, r, n)
#define SOME(rule)		ATLEAST(rule, 1)
#define STRING(s)		peg_match_string(peg, __func__, s)
#define THRU(c)			peg_match_thru(peg, __func__, c)
#define TO(c)			peg_match_to(peg, __func__, c)
#define RULE(name)		static int name(struct PEG *peg, __unused const char *__rule_name)
