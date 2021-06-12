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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mempool.h"
#include "mempool/file.h"
#include "mempool/fileflags.h"
#include "util.h"

FILE *
mempool_fopenat(struct Mempool *pool, int root, const char *path, const char *mode, mode_t perm)
{
	int flags = 0;
	if (!__sflags(mode, &flags)) {
		return NULL;
	}

	int fd = openat(root, path, flags, perm);
	if (fd == -1) {
		return NULL;
	}
	FILE *f = fdopen(fd, mode);
	if (f == NULL) {
		close(fd);
		return NULL;
	}

	return mempool_add(pool, f, fclose);
}

int
mempool_openat(struct Mempool *pool, int root, const char *path, int flags, mode_t perm)
{
	int fd = openat(root, path, flags, perm);
	if (fd == -1) {
		return -1;
	} else {
		return mempool_takefd(pool, fd);
	}
}

static void
mempool_freefd(int *fd)
{
	close(*fd);
	free(fd);
}

int
mempool_takefd(struct Mempool *pool, int fd)
{
	if (pool && fd > -1) {
		int *memfd = xmalloc(sizeof(int));
		*memfd = fd;
		mempool_add(pool, memfd, mempool_freefd);
	}
	return fd;
}
