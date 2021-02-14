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

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#define ANY(r)			ATLEAST((r), 0)
#define ATLEAST(r, n)		peg_match_atleast(peg, __func__, (r), (n))
#define ATMOST(r, n)		peg_match_between(peg, __func__, (r), 0, (n))
#define BETWEEN(r, a, b)	peg_match_between(peg, __func__, (r), (a), (b))
#define CAPTURE(x, t)		(peg_match_capture_start(peg) ? peg_match_capture_end(peg, (t), (x)) : 0)
#define CHAR(c)			peg_match_char(peg, __func__, (c))
#define CHARF(f)		peg_match_char_f(peg, __func__, (f))
#define EOS()			peg_match_eos(peg, __func__)
#define MATCH(r)		peg_match_rule(peg, __func__, (r))
#define OPT(r)			((r) || 1)
#define RANGE(a, b)		peg_match_range(peg, __func__, (a), (b))
#define REPEAT(r, n)		peg_match_between(peg, __func__, (r), (n), (n))
#define SET(...)		peg_match_chars(peg, __func__, (uint32_t[]){__VA_ARGS__}, nitems(((uint32_t[]){__VA_ARGS__})))
#define SOME(rule)		ATLEAST((rule), 1)
#define STRING(s)		peg_match_string(peg, __func__, (s))
#define THRU(c)			peg_match_thru(peg, __func__, (c))
#define TO(c)			peg_match_to(peg, __func__, (c))
#define RULE(name)		static int name(struct PEG *peg, __unused const char *__rule_name)
