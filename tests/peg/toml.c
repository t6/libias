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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "peg.h"
#include "peg/toml.h"
#include "test.h"
#include "util.h"

#define TOML_TEST_PATH "tests/peg/toml/toml-test/tests"

static const char *toml_tests[] = {
	// "invalid/array-mixed-types-arrays-and-ints.toml", // Grammar does not check this
	// "invalid/array-mixed-types-ints-and-floats.toml", // Grammar does not check this
	// "invalid/array-mixed-types-strings-and-ints.toml", // Grammar does not check this
	"invalid/bool-wrong-case-false.toml",
	"invalid/bool-wrong-case-true.toml",
	"invalid/datetime-malformed-no-leads.toml",
	"invalid/datetime-malformed-no-secs.toml",
	"invalid/datetime-malformed-no-t.toml",
	"invalid/datetime-malformed-with-milli.toml",
	// "invalid/duplicate-key-table.toml", // Grammar does not check this
	// "invalid/duplicate-keys.toml", // Grammar does not check this
	// "invalid/duplicate-tables.toml", // Grammar does not check this
	"invalid/empty-implicit-table.toml",
	"invalid/empty-table.toml",
	"invalid/float-leading-zero-neg.toml",
	"invalid/float-leading-zero-pos.toml",
	"invalid/float-leading-zero.toml",
	"invalid/float-no-leading-zero.toml",
	"invalid/float-no-trailing-digits.toml",
	"invalid/float-underscore-after-point.toml",
	"invalid/float-underscore-after.toml",
	"invalid/float-underscore-before-point.toml",
	"invalid/float-underscore-before.toml",
	"invalid/inline-table-linebreak.toml",
	"invalid/integer-leading-zero-neg.toml",
	"invalid/integer-leading-zero-pos.toml",
	"invalid/integer-leading-zero.toml",
	"invalid/integer-underscore-after.toml",
	"invalid/integer-underscore-before.toml",
	"invalid/integer-underscore-double.toml",
	"invalid/key-after-array.toml",
	"invalid/key-after-table.toml",
	"invalid/key-empty.toml",
	"invalid/key-hash.toml",
	"invalid/key-newline.toml",
	"invalid/key-no-eol.toml",
	"invalid/key-open-bracket.toml",
	"invalid/key-single-open-bracket.toml",
	"invalid/key-space.toml",
	"invalid/key-start-bracket.toml",
	"invalid/key-two-equals.toml",
	"invalid/llbrace.toml",
	"invalid/multi-line-inline-table.toml",
	"invalid/multi-line-string-no-close.toml",
	"invalid/rrbrace.toml",
	"invalid/string-bad-byte-escape.toml",
	// "invalid/string-bad-codepoint.toml", // Grammar does not check this
	"invalid/string-bad-escape.toml",
	"invalid/string-bad-slash-escape.toml",
	"invalid/string-bad-uni-esc.toml",
	"invalid/string-byte-escapes.toml",
	"invalid/string-no-close.toml",
	// "invalid/table-array-implicit.toml", // Grammar does not check this
	"invalid/table-array-malformed-bracket.toml",
	"invalid/table-array-malformed-empty.toml",
	"invalid/table-empty.toml",
	"invalid/table-nested-brackets-close.toml",
	"invalid/table-nested-brackets-open.toml",
	"invalid/table-whitespace.toml",
	"invalid/table-with-pound.toml",
	"invalid/text-after-array-entries.toml",
	"invalid/text-after-integer.toml",
	"invalid/text-after-string.toml",
	"invalid/text-after-table.toml",
	"invalid/text-before-array-separator.toml",
	"invalid/text-in-array.toml",
	"valid/array-empty.toml",
	"valid/array-nospaces.toml",
	"valid/array-string-quote-comma-2.toml",
	"valid/array-string-quote-comma.toml",
	"valid/array-string-with-comma.toml",
	"valid/array-table-array-string-backslash.toml",
	"valid/arrays-hetergeneous.toml",
	"valid/arrays-nested.toml",
	"valid/arrays.toml",
	"valid/bool.toml",
	"valid/comments-at-eof.toml",
	"valid/comments-at-eof2.toml",
	"valid/comments-everywhere.toml",
	"valid/datetime-timezone.toml",
	"valid/datetime.toml",
	"valid/double-quote-escape.toml",
	"valid/empty.toml",
	"valid/escaped-escape.toml",
	"valid/example.toml",
	"valid/exponent-part-float.toml",
	"valid/float-exponent.toml",
	"valid/float-underscore.toml",
	"valid/float.toml",
	"valid/implicit-and-explicit-after.toml",
	"valid/implicit-and-explicit-before.toml",
	"valid/implicit-groups.toml",
	"valid/inline-table-array.toml",
	"valid/inline-table.toml",
	"valid/integer-underscore.toml",
	"valid/integer.toml",
	"valid/key-equals-nospace.toml",
	"valid/key-numeric.toml",
	"valid/key-space.toml",
	"valid/key-special-chars.toml",
	"valid/keys-with-dots.toml",
	"valid/long-float.toml",
	"valid/long-integer.toml",
	"valid/multiline-string.toml",
	"valid/nested-inline-table-array.toml",
	"valid/newline-crlf.toml",
	"valid/newline-lf.toml",
	"valid/raw-multiline-string.toml",
	"valid/raw-string.toml",
	"valid/right-curly-brace-after-boolean.toml",
	"valid/string-empty.toml",
	"valid/string-escapes.toml",
	"valid/string-nl.toml",
	"valid/string-simple.toml",
	"valid/string-with-pound.toml",
	"valid/table-array-implicit.toml",
	"valid/table-array-many.toml",
	"valid/table-array-nest.toml",
	"valid/table-array-one.toml",
	"valid/table-array-table-array.toml",
	"valid/table-empty.toml",
	"valid/table-no-eol.toml",
	"valid/table-sub-empty.toml",
	"valid/table-whitespace.toml",
	"valid/table-with-literal-string.toml",
	"valid/table-with-pound.toml",
	"valid/table-with-single-quotes.toml",
	"valid/table-without-super.toml",
	"valid/underscored-float.toml",
	"valid/underscored-integer.toml",
	"valid/unicode-escape.toml",
	"valid/unicode-literal.toml",

	"../../integers.toml",
};

#define tests toml_tests

// static const char *selected_tests[] = {
// };
// #define tests selected_tests

static int
check_match(const char *s, int expected)
{
	struct PEG *peg = peg_new(s, strlen(s));
	int result = peg_match(peg, peg_toml_decode, NULL, NULL);
	peg_free(peg);
	return result == expected;
}

TESTS() {
	chdir(TOML_TEST_PATH);
	for (size_t i = 0; i < nitems(tests); i++) {
		const char *name = tests[i];
		int valid = *name != 'i';
		int fd = open(name, O_RDONLY);
		char *buf;
		if (fd == -1) {
			char *msg = str_printf("%s: %s", name, strerror(errno));
			TEST_FAIL(msg);
			free(msg);
			continue;
		}
		buf = slurp(fd);
		if (buf == NULL) {
			TEST_FAIL(strerror(errno));
		} else if (check_match(buf, valid)) {
			TEST_OK();
		} else {
			const char *reason;
			if (valid) {
				reason = "not accepted";
			} else {
				reason = "not rejected";
			}
			char *filename = str_printf("%s/%s", TOML_TEST_PATH, name);
			TEST_FAIL_LOC(filename, 1, 1, reason);
			free(filename);
		}
		free(buf);
		close(fd);
	}
}
