#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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
	// for (size_t i = 0; i < array_len(parser->captures); i++) {
	// 	free(array_get(parser->captures, i));
	// }
	// array_truncate(parser->captures);
}

#define __CHECK(x, t, e)	if (x) { t; } else { e; }
#define __ATLEAST(r, n, t, e)	__CHECK(parser_match_atleast(parser, __func__, r, n), t, e)
#define __CHAR(c, n, t, e)	__CHECK(parser_match_char(parser, __func__, c, n), t, e)
#define __CHARF(f, n, t, e)	__CHECK(parser_match_char_f(parser, __func__, f, n), t, e)
#define __EOS(t, e)		__CHECK(parser_match_eos(parser, __func__), t, e)
#define __MATCH(r, t, e)	__CHECK(parser_match_rule(parser, __func__, r), t, e)
#define __THRU(c, t, e)		__CHECK(parser_match_thruto(parser, __func__, c, 1), t, e)
#define __TO(c, t, e)		__CHECK(parser_match_thruto(parser, __func__, c, 0), t, e)
#define ANY(r)			ATLEAST(r, 0)
#define ATLEAST(c, n)		__ATLEAST(c, n, , return)
#define CHAR(c, n)		__CHAR(c, n, , return)
#define CHARF(f, n)		__CHARF(f, n, , return)
#define EOS()			__EOS(, return)
#define MATCH(r)		__MATCH(r, , return)
#define SOME(rule)		ATLEAST(rule, 1)
#define THRU(c)			__THRU(c, , return)
#define TO(c)			__TO(c, , return)
#define TRY_ANY(r)		TRY_ATLEAST(c, 0)
#define TRY_ATLEAST(c, n)	__ATLEAST(c, n, return, )
#define TRY_CHAR(c, n)		__CHAR(c, n, return, )
#define TRY_CHARF(f, n)		__CHARF(f, n, return, )
#define TRY_EOS()		__EOS(return, )
#define TRY_MATCH(r)		__MATCH(r, return, )
#define TRY_SOME(r)		TRY_ATLEAST(rule, 1)
#define TRY_THRU(c)		__THRU(c, return, )
#define TRY_TO(c)		__TO(c, return, )
#define RULE(name)		void name(struct Parser *parser)

static void
parser_capture(struct Parser *parser, const char *rule, size_t len)
{
	assert((parser->pos + len) < parser->len);
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
parser_match_atleast(struct Parser *parser, const char *rule, RuleFn rulefn, int n)
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
	if (i < n) {
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
parser_match_thruto(struct Parser *parser, const char *rule, char c, int thru)
{
	if (!parser_match_init(parser, rule)) {
		return 0;
	}
	int ok = 0;
	size_t i = parser->pos;
	for (; i < parser->len; i++) {
		if (parser->buf[i] == c) {
			ok = 1;
			break;
		}
	}
	if (!ok) {
		return parser_mismatch(parser, rule);
	} else {
		if (parser->on_capture) {
			parser_capture(parser, rule, i + thru - parser->pos);
		}
		parser->pos = i + thru;
	}
	return parser_match_ok(parser);
}

struct Parser *
parser_new(const char *buf, size_t len)
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

RULE(foobar) { // (+ 'f' 'g')
	TRY_CHAR('f', 1);
	CHAR('g', 1);
}

RULE(comment) {
	CHAR('#', 1); THRU('\n');
}

RULE(date) {
	CHARF(isdigit, 4); CHAR('-', 1); CHARF(isdigit, 2); CHAR('-', 1); CHARF(isdigit, 2);
}


RULE(entry) {
	TO('|'); CHAR('|', 1); TO('|'); CHAR('|', 1); MATCH(date); CHAR('|', 1); TO('\n'); CHAR('\n', 1);
}

RULE(entry_comment) { // (+ :comment :entry)
	TRY_MATCH(comment);
	TRY_MATCH(entry);
}

RULE(moved_grammar) {
	ANY(entry_comment);
	EOS();
}

RULE(foo) { // (* (some :foobar) -1)
	MATCH(foobar);
	MATCH(foobar);
	EOS();
}

int
main(int argc, char *argv[])
{
//     :main (* (any (+ :comment :entry)) -1)})
	const char *buf = "#\naudio/polypaudio|audio/pulseaudio|2008-01-01|Project renamed\n# asdjflasdk\n"
	"audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n"
"audio/akode-plugins-polypaudio||2008-01-01|Polypaudio is obsolete in favor of Pulseaudio\n";

	//const char *buf = "2008-08-08\n";
	//const char *buf = "audio/polypaudio|audio/pulseaudio|2008-01-01|Project renamed\n";
	struct Parser *parser = parser_new(buf, strlen(buf));
	moved_grammar(parser);
	printf("parser state: %d [%c]\n", parser->state, parser->buf[parser->pos]);
	return 0;
}
