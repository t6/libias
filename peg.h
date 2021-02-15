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
struct Array;

struct PEGCapture {
	char *buf;
	unsigned int tag;
	unsigned int state;
	size_t pos;
	size_t len;
};

enum PEGCaptureFlag {
	PEG_CAPTURE_DISCARD,
	PEG_CAPTURE_KEEP,
};

typedef enum PEGCaptureFlag (*CaptureFn)(struct PEG *, struct PEGCapture *, void *);
typedef int (*RuleFn)(struct PEG *);

struct PEG *peg_new(const char *, size_t);
void peg_free(struct PEG *);

struct Array *peg_captures(struct PEG *, unsigned int);
int peg_match(struct PEG *, RuleFn, void **);

int peg_match_atleast(struct PEG *, const char *, RuleFn, int);
int peg_match_between(struct PEG *, const char *, RuleFn, int, int);
int peg_match_capture_start(struct PEG *);
int peg_match_capture_end(struct PEG *, unsigned int, unsigned int, CaptureFn, size_t, int);
int peg_match_char(struct PEG *, const char *, uint32_t);
int peg_match_char_f(struct PEG *, const char *, int (*)(int));
int peg_match_chars(struct PEG *, const char *, uint32_t[], size_t);
int peg_match_eos(struct PEG *, const char *);
int peg_match_range(struct PEG *, const char *, uint32_t, uint32_t);
int peg_match_rule(struct PEG *, const char *, RuleFn);
int peg_match_string(struct PEG *, const char *, const char *);
int peg_match_thru(struct PEG *, const char *, const char *);
int peg_match_to(struct PEG *, const char *, const char *);
