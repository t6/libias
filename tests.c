#if TEST___PROGNAME
int
main(void)
{
	extern char *__progname;

	return !__progname;
}
#endif /* TEST___PROGNAME */
#if TEST_ARC4RANDOM
#include <stdlib.h>

int
main(void)
{
	return (arc4random() + 1) ? 0 : 1;
}
#endif /* TEST_ARC4RANDOM */
#if TEST_CAPSICUM
#include <sys/capsicum.h>

int
main(void)
{
	cap_enter();

	return(0);
}
#endif /* TEST_CAPSICUM */
#if TEST_ERR
/*
 * Copyright (c) 2015 Ingo Schwarze <schwarze@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>

int
main(void)
{
	warnx("%d. warnx", 1);
	warn("%d. warn", 2);
	err(0, "%d. err", 3);
	/* NOTREACHED */
	return 1;
}
#endif /* TEST_ERR */
#if TEST_EXPLICIT_BZERO
#include <string.h>

int
main(void)
{
	char foo[10];

	explicit_bzero(foo, sizeof(foo));
	return(0);
}
#endif /* TEST_EXPLICIT_BZERO */
#if TEST_GETEXECNAME
#include <stdlib.h>

int
main(void)
{
	const char * progname;

	progname = getexecname();
	return progname == NULL;
}
#endif /* TEST_GETEXECNAME */
#if TEST_GETPROGNAME
#include <stdlib.h>

int
main(void)
{
	const char * progname;

	progname = getprogname();
	return progname == NULL;
}
#endif /* TEST_GETPROGNAME */
#if TEST_INFTIM
/*
 * Linux doesn't (always?) have this.
 */

#include <poll.h>
#include <stdio.h>

int
main(void)
{
	printf("INFTIM is defined to be %ld\n", (long)INFTIM);
	return 0;
}
#endif /* TEST_INFTIM */
#if TEST_MEMMEM
#define _GNU_SOURCE
#include <string.h>

int
main(void)
{
	char *a = memmem("hello, world", strlen("hello, world"), "world", strlen("world"));
	return(NULL == a);
}
#endif /* TEST_MEMMEM */
#if TEST_MEMRCHR
#if defined(__linux__) || defined(__MINT__)
#define _GNU_SOURCE	/* See test-*.c what needs this. */
#endif
#include <string.h>

int
main(void)
{
	const char *buf = "abcdef";
	void *res;

	res = memrchr(buf, 'a', strlen(buf));
	return(NULL == res ? 1 : 0);
}
#endif /* TEST_MEMRCHR */
#if TEST_MEMSET_S
#include <string.h>

int main(void)
{
	char buf[10];
	memset_s(buf, 0, 'c', sizeof(buf));
	return 0;
}
#endif /* TEST_MEMSET_S */
#if TEST_PATH_MAX
/*
 * POSIX allows PATH_MAX to not be defined, see
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sysconf.html;
 * the GNU Hurd is an example of a system not having it.
 *
 * Arguably, it would be better to test sysconf(_SC_PATH_MAX),
 * but since the individual *.c files include "config.h" before
 * <limits.h>, overriding an excessive value of PATH_MAX from
 * "config.h" is impossible anyway, so for now, the simplest
 * fix is to provide a value only on systems not having any.
 * So far, we encountered no system defining PATH_MAX to an
 * impractically large value, even though POSIX explicitly
 * allows that.
 *
 * The real fix would be to replace all static buffers of size
 * PATH_MAX by dynamically allocated buffers.  But that is
 * somewhat intrusive because it touches several files and
 * because it requires changing struct mlink in mandocdb.c.
 * So i'm postponing that for now.
 */

#include <limits.h>
#include <stdio.h>

int
main(void)
{
	printf("PATH_MAX is defined to be %ld\n", (long)PATH_MAX);
	return 0;
}
#endif /* TEST_PATH_MAX */
#if TEST_PLEDGE
#include <unistd.h>

int
main(void)
{
	return !!pledge("stdio", NULL);
}
#endif /* TEST_PLEDGE */
#if TEST_PROGRAM_INVOCATION_SHORT_NAME
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <errno.h>

int
main(void)
{

	return !program_invocation_short_name;
}
#endif /* TEST_PROGRAM_INVOCATION_SHORT_NAME */
#if TEST_BSD_QSORT_R
#define _GNU_SOURCE
#include <stdlib.h>
void (qsort_r)(void *base, size_t nmemb, size_t size, void *arg, int (*compar)(void *, const void *, const void *));

static int
int_cmp(void *arg, const void *p1, const void *p2)
{
	int left = *(const int *)p1;
	int right = *(const int *)p2;
	int *flag = arg;
	*flag = 42;
	return ((left > right) - (left < right));
}

int
main(void)
{
	int xs[] = { 3, 2, 1 };
	int flag = -1;
	qsort_r(&xs, 3, sizeof(xs[0]), &flag, int_cmp);
	return !(xs[0] == 1 && xs[1] == 2 && xs[2] == 3 && flag == 42);
}
#endif /* TEST_BSD_QSORT_R */
#if TEST_GNU_QSORT_R
#define _GNU_SOURCE
#include <stdlib.h>
void (qsort_r)(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *, void *), void *arg);

static int
int_cmp(const void *p1, const void *p2, void *arg)
{
	int left = *(const int *)p1;
	int right = *(const int *)p2;
	int *flag = arg;
	*flag = 42;
	return ((left > right) - (left < right));
}

int
main(void)
{
	int xs[] = { 3, 2, 1 };
	int flag = -1;
	qsort_r(&xs, 3, sizeof(xs[0]), int_cmp, &flag);
	return !(xs[0] == 1 && xs[1] == 2 && xs[2] == 3 && flag == 42);
}
#endif /* TEST_GNU_QSORT_R */
#if TEST_REALLOCARRAY
#include <stdlib.h>

int
main(void)
{
	return !reallocarray(NULL, 2, 2);
}
#endif /* TEST_REALLOCARRAY */
#if TEST_RECALLOCARRAY
#include <stdlib.h>

int
main(void)
{
	return !recallocarray(NULL, 0, 2, 2);
}
#endif /* TEST_RECALLOCARRAY */
#if TEST_STRLCAT
#include <string.h>

int
main(void)
{
	char buf[3] = "a";
	return ! (strlcat(buf, "b", sizeof(buf)) == 2 &&
	    buf[0] == 'a' && buf[1] == 'b' && buf[2] == '\0');
}
#endif /* TEST_STRLCAT */
#if TEST_STRLCPY
#include <string.h>

int
main(void)
{
	char buf[2] = "";
	return ! (strlcpy(buf, "a", sizeof(buf)) == 1 &&
	    buf[0] == 'a' && buf[1] == '\0');
}
#endif /* TEST_STRLCPY */
#if TEST_STRNDUP
#include <string.h>

int
main(void)
{
	const char *foo = "bar";
	char *baz;

	baz = strndup(foo, 1);
	return(0 != strcmp(baz, "b"));
}
#endif /* TEST_STRNDUP */
#if TEST_STRNLEN
#include <string.h>

int
main(void)
{
	const char *foo = "bar";
	size_t sz;

	sz = strnlen(foo, 1);
	return(1 != sz);
}
#endif /* TEST_STRNLEN */
#if TEST_STRNSTR
#include <string.h>

int
main(void)
{
	const char *foo = "bar";
	return(NULL == strnstr(foo, "a", 2);
}
#endif /* TEST_STRNSTR */
#if TEST_STRTONUM
/*
 * Copyright (c) 2015 Ingo Schwarze <schwarze@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>

int
main(void)
{
	const char *errstr;

	if (strtonum("1", 0, 2, &errstr) != 1)
		return 1;
	if (errstr != NULL)
		return 2;
	if (strtonum("1x", 0, 2, &errstr) != 0)
		return 3;
	if (errstr == NULL)
		return 4;
	if (strtonum("2", 0, 1, &errstr) != 0)
		return 5;
	if (errstr == NULL)
		return 6;
	if (strtonum("0", 1, 2, &errstr) != 0)
		return 7;
	if (errstr == NULL)
		return 8;
	return 0;
}
#endif /* TEST_STRTONUM */
#if TEST_SYS_TREE
#include <sys/tree.h>
#include <stdlib.h>

struct node {
	RB_ENTRY(node) entry;
	int i;
};

static int
intcmp(struct node *e1, struct node *e2)
{
	return (e1->i < e2->i ? -1 : e1->i > e2->i);
}

RB_HEAD(inttree, node) head = RB_INITIALIZER(&head);
RB_PROTOTYPE(inttree, node, entry, intcmp)
RB_GENERATE(inttree, node, entry, intcmp)

int testdata[] = {
	20, 16, 17, 13, 3, 6, 1, 8, 2, 4
};

int
main(void)
{
	size_t i;
	struct node *n;

	for (i = 0; i < sizeof(testdata) / sizeof(testdata[0]); i++) {
		if ((n = malloc(sizeof(struct node))) == NULL)
			return 1;
		n->i = testdata[i];
		RB_INSERT(inttree, &head, n);
	}

	return 0;
}

#endif /* TEST_SYS_TREE */
