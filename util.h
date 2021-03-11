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

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef __printflike
#define __printflike(x, y)	__attribute__((__format__(__printf__, x, y)))
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

struct Array;
typedef int (*CompareFn)(const void *, const void *, void *);

char *read_symlink(int, const char *);
char *slurp(int);
int update_symlink(int, const char *, const char *, char **);

char *str_common_prefix(const char *, const char *);
int str_compare(const void *, const void *, void *);
int str_endswith(const char *, const char *);
char *str_join(struct Array *, const char *);
char *str_printf(const char *, ...) __printflike(1, 2);
char *str_repeat(const char *, const size_t);
int str_startswith(const char *, const char *);
char *str_substr(const char *, const size_t, const size_t);
char *str_trim(const char *);
char *str_triml(const char *);
char *str_trimr(const char *);

void sort(void *, size_t, size_t, CompareFn, void *);
void *xmalloc(size_t);
void *xrecallocarray(void *, size_t, size_t, size_t);
char *xstrdup(const char *);
char *xstrndup(const char *, size_t);
size_t xstrlcat(char *, const char *, size_t);
size_t xstrlcpy(char *, const char *, size_t);
