#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "strnstr.c"

struct Parser;

typedef void (*CaptureFn)(struct Parser *, const char *, const char *, size_t, void *);
typedef void (*MismatchFn)(struct Parser *, const char *, void *);
typedef void (*RuleFn)(struct Parser *);

enum ParserState {
	PARSER_OK,
	PARSER_MISMATCH,
};

struct Parser {
	const char *buf;
	size_t pos;
	size_t len;
	enum ParserState state;

	CaptureFn on_capture;
	MismatchFn on_mismatch;
	void *userdata;
};

#define __CHECK(x, t, e)	if (x) { t; } else { e; }
#define __BETWEEN(r, a, b, t, e)__CHECK(parser_match_between(parser, __func__, r, a, b), t, e)
#define __CHAR(c, n, t, e)	__CHECK(parser_match_char(parser, __func__, c, n), t, e)
#define __CHARF(f, n, t, e)	__CHECK(parser_match_char_f(parser, __func__, f, n), t, e)
#define __EOS(t, e)		__CHECK(parser_match_eos(parser, __func__), t, e)
#define __RANGE(a, b, t, e)	__CHECK(parser_match_range(parser, __func__, a, b), t, e)
#define __MATCH(r, t, e)	__CHECK(parser_match_rule(parser, __func__, r), t, e)
#define __STRING(s, t, e)	__CHECK(parser_match_string(parser, __func__, s), t, e)
#define __THRU(c, t, e)		__CHECK(parser_match_thruto(parser, __func__, c, 1), t, e)
#define __TO(c, t, e)		__CHECK(parser_match_thruto(parser, __func__, c, 0), t, e)
#define ANY(r)			ATLEAST(r, 0)
#define ATLEAST(r, n)		__BETWEEN(r, n, n, , return)
#define BETWEEN(r, a, b)	__BETWEEN(r, a, b, , return)
#define CHAR(c, n)		__CHAR(c, n, , return)
#define CHARF(f, n)		__CHARF(f, n, , return)
#define EOS()			__EOS(, return)
#define MATCH(r)		__MATCH(r, , return)
#define RANGE(a, b)		__RANGE(a, b, , return)
#define SOME(rule)		ATLEAST(rule, 1)
#define STRING(s)		__STRING(s, , return)
#define THRU(c)			__THRU(c, , return)
#define TO(c)			__TO(c, , return)
#define TRY_ANY(r)		TRY_ATLEAST(c, 0)
#define TRY_ATLEAST(r, n)	__BETWEEN(r, n, n, return, )
#define TRY_BETWEEN(r, a, b)	__BETWEEN(r, a, b, return, )
#define TRY_CHAR(c, n)		__CHAR(c, n, return, )
#define TRY_CHARF(f, n)		__CHARF(f, n, return, )
#define TRY_EOS()		__EOS(return, )
#define TRY_MATCH(r)		__MATCH(r, return, )
#define TRY_RANGE(a, b)		__RANGE(a, b, return, )
#define TRY_SOME(r)		TRY_ATLEAST(rule, 1)
#define TRY_STRING(s)		__STRING(s, return, )
#define TRY_THRU(c)		__THRU(c, return, )
#define TRY_TO(c)		__TO(c, return, )
#define RULE(name)		void name(struct Parser *parser)

static void
parser_capture(struct Parser *parser, const char *rule, size_t len)
{
	assert((parser->pos + len) <= parser->len);
	if (parser->on_capture) {
		parser->on_capture(parser, rule, parser->buf + parser->pos, len, parser->userdata);
	}
}

static int
parser_mismatch(struct Parser *parser, const char *rule)
{
	parser->state = PARSER_MISMATCH;
	if (parser->on_mismatch) {
		parser->on_mismatch(parser, rule, parser->userdata);
	}
	return 0;
}

static int
parser_match_ok(struct Parser *parser)
{
	parser->state = PARSER_OK;
	return 1;
}

static int
parser_match_init(struct Parser *parser, const char *rule)
{
	if (parser->pos > parser->len) {
		return parser_mismatch(parser, rule);
	}
	return parser_match_ok(parser);
}

static int
parser_match_between(struct Parser *parser, const char *rule, RuleFn rulefn, int a, int b)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	size_t pos = parser->pos;
	int i = 0;
	while (1) {
		rulefn(parser);
		if (parser->state != PARSER_OK) {
			break;
		}
		i++;
	}
	if (i < a || i > b) {
		parser->pos = pos;
		return parser_mismatch(parser, rule);
	}
	return parser_match_ok(parser);
}

static int
parser_match_char(struct Parser *parser, const char *rule, char c, int n)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	size_t len = parser->len - parser->pos;
	if (len < n) {
		return parser_mismatch(parser, rule);
	}
	size_t i = parser->pos;
	for (int j = 0; j < n; j++, i++) {
		if (parser->buf[i] != c) {
			return parser_mismatch(parser, rule);
		}
	}
	parser->pos = i;
	return parser_match_ok(parser);
}

static int
parser_match_char_f(struct Parser *parser, const char *rule, int (*f)(int), int n)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	size_t len = parser->len - parser->pos;
	if (len < n) {
		return parser_mismatch(parser, rule);
	}
	size_t i = parser->pos;
	for (int j = 0; j < n; j++, i++) {
		if (!f(parser->buf[i])) {
			return parser_mismatch(parser, rule);
		}
	}
	parser->pos = i;
	return parser_match_ok(parser);
}

static int
parser_match_eos(struct Parser *parser, const char *rule)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	if (parser->pos != parser->len) {
		return parser_mismatch(parser, rule);
	}
	return parser_match_ok(parser);
}

static int
parser_match_range(struct Parser *parser, const char *rule, char a, char b)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	if ((parser->len - parser->pos) < 1) {
		return parser_mismatch(parser, rule);
	}
	char c = parser->buf[parser->pos];
	if (a > c || c > b) {
		printf("%c %c %c\n", a, b, c);
		return parser_mismatch(parser, rule);
	}
	parser->pos++;
	return parser_match_ok(parser);
}
static int
parser_match_rule(struct Parser *parser, const char *rule, RuleFn rulefn)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	rulefn(parser);
	if (parser->state == PARSER_MISMATCH) {
		return parser_mismatch(parser, rule);
	}
	return parser_match_ok(parser);
}

static int
parser_match_string(struct Parser *parser, const char *rule, const char *needle)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	size_t len = strlen(needle);
	if ((parser->len - parser->pos) < len ||
	    strncmp(parser->buf + parser->pos, needle, len) != 0) {
		return parser_mismatch(parser, rule);
	}
	parser->pos += len;
	return parser_match_ok(parser);
}

static int
parser_match_thruto(struct Parser *parser, const char *rule, const char *needle, int thru)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	if (strcmp(needle, "") == 0) {
		return parser_mismatch(parser, rule);
	}

	char *ptr = strnstr(parser->buf + parser->pos, needle, parser->len);
	if (ptr == NULL) {
		return parser_mismatch(parser, rule);
	}

	size_t offset = 0;
	if (thru) {
		offset = strlen(needle);
	}
	size_t len = ptr - (parser->buf + parser->pos) + offset;
	if (parser->on_capture) {
		parser_capture(parser, rule, len);
	}
	parser->pos += len;

	return parser_match_ok(parser);
}

struct Parser *
parser_new(const char *buf, size_t len, CaptureFn on_capture, MismatchFn on_mismatch)
{
	struct Parser *parser = malloc(sizeof(struct Parser));
	if (parser == NULL) {
		return NULL;
	}
	memset(parser, sizeof(parser), 0);
	parser->buf = buf;
	parser->len = len;
	parser->state = PARSER_OK;
	parser->on_capture = on_capture;
	parser->on_mismatch = on_mismatch;
}

RULE(comment) {
	CHAR('#', 1); THRU("\n");
}

RULE(date) {
	CHARF(isdigit, 4); CHAR('-', 1); CHARF(isdigit, 2); CHAR('-', 1); CHARF(isdigit, 2);
}


RULE(entry) {
	TO("|"); CHAR('|', 1); TO("|"); CHAR('|', 1); MATCH(date); CHAR('|', 1); TO("\n"); CHAR('\n', 1);
}

RULE(entry_comment) { // (+ :comment :entry)
	TRY_MATCH(comment);
	TRY_MATCH(entry);
}

RULE(moved_grammar) {
	ANY(entry_comment);
	EOS();
}

void
on_capture(struct Parser *parser, const char *rule, const char *buf, size_t len, void *userdata)
{
	char *s = strndup(buf, len);
	printf("captured in %s: %s\n", rule, s);
	free(s);
}

void
on_mismatch(struct Parser *parser, const char *rule, void *userdata)
{
	printf("mismatch %s\n", rule);
}

#include "ipv4.c"

RULE(foo) {
	MATCH(byte);
	printf("here\n");
	EOS();
}

int
main(int argc, char *argv[])
{
	const char *buf = "#\naudio/polypaudio|audio/pulseaudio|2008-01-01|Project renamed\n# asdjflasdk\n"
	"audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n"
"audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n";
	struct Parser *parser = parser_new(buf, strlen(buf), on_capture, on_mismatch);
	//moved_grammar(parser);
	printf("parser state: %d\n", parser->state);
	buf = "256";
	parser = parser_new(buf, strlen(buf), on_capture, on_mismatch);
	foo(parser);
	printf("parser state: %d\n", parser->state);
	return 0;
}
