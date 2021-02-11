#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "strnstr.c"

struct PEG;

typedef void (*CaptureFn)(struct PEG *, const char *, const char *, size_t, void *);
typedef void (*MismatchFn)(struct PEG *, const char *, void *);
typedef int (*RuleFn)(struct PEG *);

enum PEGState {
	PARSER_OK,
	PARSER_MISMATCH,
};

struct PEG {
	const char *buf;
	size_t pos;
	size_t len;
	enum PEGState state;

	CaptureFn on_capture;
	MismatchFn on_mismatch;
	void *userdata;
};

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
#define THRU(c)			peg_match_thruto(peg, __func__, c, 1)
#define TO(c)			peg_match_thruto(peg, __func__, c, 0)
#define RULE(name)		int name(struct PEG *peg)

static void
peg_capture(struct PEG *peg, const char *rule, size_t len)
{
	assert((peg->pos + len) <= peg->len);
	if (peg->on_capture) {
		peg->on_capture(peg, rule, peg->buf + peg->pos, len, peg->userdata);
	}
}

static int
peg_match_init(struct PEG *peg, const char *rule)
{
	return peg->pos <= peg->len;
}

static int
peg_match_atleast(struct PEG *peg, const char *rule, RuleFn rulefn, int n)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	size_t pos = peg->pos;
	int i = 0;
	while (1) {
		if (!rulefn(peg)) {
			break;
		}
		i++;
	}
	if (i < n) {
		peg->pos = pos;
		return 0;
	}
	return 1;
}

static int
peg_match_between(struct PEG *peg, const char *rule, RuleFn rulefn, int a, int b)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	size_t pos = peg->pos;
	int i = 0;
	while (1) {
		if (!rulefn(peg)) {
			break;
		}
		i++;
	}
	if (i >= a && i < b) {
		return 1;
	} else {
		peg->pos = pos;
		return 0;
	}
}

static int
peg_match_char(struct PEG *peg, const char *rule, char c)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	size_t len = peg->len - peg->pos;
	if (len < 1) {
		return 0;
	}
	if (peg->buf[peg->pos] == c) {
		peg->pos++;
		return 1;
	} else {
		return 0;
	}
}

static int
peg_match_char_f(struct PEG *peg, const char *rule, int (*f)(int))
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	size_t len = peg->len - peg->pos;
	if (len < 1) {
		return 0;
	}
	if (f(peg->buf[peg->pos])) {
		peg->pos++;
		return 1;
	} else {
		return 0;
	}
}

static int
peg_match_eos(struct PEG *peg, const char *rule)
{
	return peg_match_init(peg, rule) && peg->pos == peg->len;
}

static int
peg_match_range(struct PEG *peg, const char *rule, char a, char b)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	if ((peg->len - peg->pos) < 1) {
		return 0;
	}
	char c = peg->buf[peg->pos];
	if (a > c || c > b) {
		return 0;
	}
	peg->pos++;
	return 1;
}

static int
peg_match_repeat(struct PEG *peg, const char *rule, RuleFn rulefn, int n)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	size_t pos = peg->pos;
	for (int i = 0; i < n; i++) {
		if (!rulefn(peg)) {
			peg->pos = pos;
			return 0;
		}
	}
	return 1;
}

static int
peg_match_rule(struct PEG *peg, const char *rule, RuleFn rulefn)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	return rulefn(peg);
}

static int
peg_match_string(struct PEG *peg, const char *rule, const char *needle)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	size_t len = strlen(needle);
	if ((peg->len - peg->pos) < len ||
	    strncmp(peg->buf + peg->pos, needle, len) != 0) {
		return 0;
	}
	peg->pos += len;
	return 1;
}

static int
peg_match_thruto(struct PEG *peg, const char *rule, const char *needle, int thru)
{
	if (!peg_match_init(peg, rule)) {
		return 0;
	}
	if (strcmp(needle, "") == 0) {
		return 0;
	}

	char *ptr = strnstr(peg->buf + peg->pos, needle, peg->len);
	if (ptr == NULL) {
		return 0;
	}

	size_t offset = 0;
	if (thru) {
		offset = strlen(needle);
	}
	size_t len = ptr - (peg->buf + peg->pos) + offset;
	if (peg->on_capture) {
		peg_capture(peg, rule, len);
	}
	peg->pos += len;

	return 1;
}

struct PEG *
peg_new(const char *buf, size_t len, CaptureFn on_capture, MismatchFn on_mismatch)
{
	struct PEG *peg = xmalloc(sizeof(struct PEG));
	peg->buf = buf;
	peg->len = len;
	peg->state = PARSER_OK;
	peg->on_capture = on_capture;
	peg->on_mismatch = on_mismatch;
	return peg;
}

RULE(comment) {
	if (CHAR('#'))
	if (THRU("\n"))
	return 1;
	return 0;
}

RULE(digit) {
	return CHARF(isdigit);
}

RULE(date) {
	if (REPEAT(digit, 4))
	if (CHAR('-'))
	if (REPEAT(digit, 2))
	if (CHAR('-'))
	if (REPEAT(digit, 2))
	return 1;
	return 0;
}

RULE(entry) {
	if (TO("|"))
	if (CHAR('|'))
	if (TO("|"))
	if (CHAR('|'))
	if (MATCH(date))
	if (CHAR('|'))
	if (TO("\n"))
	if (CHAR('\n'))
	return 1;
	return 0;
}

RULE(entry_comment) { // (+ :comment :entry)
	if (!MATCH(comment))
	if (!MATCH(entry))
	return 0;
	return 1;
}

RULE(moved_grammar) {
	if (ANY(entry_comment))
	if (EOS())
	return 1;
	return 0;
}

void
on_capture(struct PEG *peg, const char *rule, const char *buf, size_t len, void *userdata)
{
	char *s = strndup(buf, len);
	printf("captured in %s: %s\n", rule, s);
	free(s);
}

void
on_mismatch(struct PEG *peg, const char *rule, void *userdata)
{
	printf("mismatch %s\n", rule);
}

#include "ipv4.c"
RULE(foobar) {
	if (MATCH(byte))
	if (EOS())
	return 1;
	return 0;
}

int
main(int argc, char *argv[])
{
	const char *buf;
	int state;
	//= "#\naudio/polypaudio|audio/pulseaudio|2008-01-01|Project renamed\n# asdjflasdk\n"
// 	"audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n"
// "audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n";
	buf = "# asfdsa\naudio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n";
	struct PEG *peg = peg_new(buf, strlen(buf), on_capture, on_mismatch);
	state = moved_grammar(peg);
	printf("peg state: %s\n", state ? "matched" : "mismatch");

	buf = "256";
	peg = peg_new(buf, strlen(buf), on_capture, on_mismatch);
	state = foobar(peg);
	printf("peg state: %s\n", state ? "matched" : "mismatch");
	return 0;
}
