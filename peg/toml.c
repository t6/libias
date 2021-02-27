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

#include <stdint.h>

#include "peg.h"
#include "peg/toml.h"
#include "peg/macros.h"

// See RFC 5234 -- https://www.ietf.org/rfc/rfc5234.txt
// See https://github.com/toml-lang/toml/blob/f7b5741dad165585dc38cec30df0634132aad422/toml.abnf

//
static RULE(ALPHA);
static RULE(DIGIT);
static RULE(HEXDIG);
static RULE(TOML);
static RULE(TOML_0);
static RULE(apostrophe);
static RULE(array);
static RULE(array_close);
static RULE(array_open);
static RULE(array_sep);
static RULE(array_table);
static RULE(array_table_close);
static RULE(array_table_open);
static RULE(array_values);
static RULE(array_values_0);
static RULE(array_values_1);
static RULE(basic_char);
static RULE(basic_string);
static RULE(basic_unescaped);
static RULE(bin_int);
static RULE(bin_int_0);
static RULE(bin_int_1);
static RULE(bin_prefix);
static RULE(boolean);
static RULE(comment);
static RULE(comment_start_symbol);
static RULE(date_fullyear);
static RULE(date_mday);
static RULE(date_month);
static RULE(date_time);
static RULE(dec_int);
static RULE(dec_int_0);
static RULE(decimal_point);
static RULE(digit0_1);
static RULE(digit0_7);
static RULE(digit1_9);
static RULE(dot_sep);
static RULE(dotted_key);
static RULE(dotted_key_0);
static RULE(escape);
static RULE(escape_seq_char);
static RULE(escape_seq_char_0);
static RULE(escape_seq_char_1);
static RULE(escape_seq_char_2);
static RULE(escaped);
static RULE(exponent);
static RULE(expression);
static RULE(expression_0);
static RULE(expression_1);
static RULE(expression_2);
static RULE(false_);
static RULE(float_exp_part);
static RULE(float_exp_part_0);
static RULE(float_int_part);
static RULE(floating);
static RULE(floating_0);
static RULE(floating_1);
static RULE(floating_2);
static RULE(frac);
static RULE(full_date);
static RULE(full_time);
static RULE(hex_int);
static RULE(hex_int_0);
static RULE(hex_int_1);
static RULE(hex_prefix);
static RULE(inf);
static RULE(inline_table);
static RULE(inline_table_close);
static RULE(inline_table_keyvals);
static RULE(inline_table_keyvals_0);
static RULE(inline_table_open);
static RULE(inline_table_sep);
static RULE(integer);
static RULE(key);
static RULE(keyval);
static RULE(keyval_sep);
static RULE(literal_char);
static RULE(literal_string);
static RULE(local_date);
static RULE(local_date_time);
static RULE(local_time);
static RULE(minus);
static RULE(ml_basic_body);
static RULE(ml_basic_body_0);
static RULE(ml_basic_string);
static RULE(ml_basic_string_delim);
static RULE(ml_literal_body);
static RULE(ml_literal_body_0);
static RULE(ml_literal_string);
static RULE(ml_literal_string_delim);
static RULE(mlb_char);
static RULE(mlb_content);
static RULE(mlb_escaped_nl);
static RULE(mlb_escaped_nl_0);
static RULE(mlb_quotes);
static RULE(mlb_quotes_0);
static RULE(mlb_unescaped);
static RULE(mll_char);
static RULE(mll_content);
static RULE(mll_quotes);
static RULE(mll_quotes_0);
static RULE(nan_);
static RULE(newline);
static RULE(non_ascii);
static RULE(non_eol);
static RULE(oct_int);
static RULE(oct_int_0);
static RULE(oct_int_1);
static RULE(oct_prefix);
static RULE(offset_date_time);
static RULE(partial_time);
static RULE(plus);
static RULE(quotation_mark);
static RULE(quoted_key);
static RULE(simple_key);
static RULE(special_float);
static RULE(special_float_0);
static RULE(special_float_1);
static RULE(std_table);
static RULE(std_table_close);
static RULE(std_table_open);
static RULE(string);
static RULE(table);
static RULE(time_delim);
static RULE(time_hour);
static RULE(time_minute);
static RULE(time_numoffset);
static RULE(time_numoffset_0);
static RULE(time_offset);
static RULE(time_secfrac);
static RULE(time_second);
static RULE(true_);
static RULE(underscore);
static RULE(unquoted_key);
static RULE(unquoted_key_0);
static RULE(unsigned_dec_int);
static RULE(unsigned_dec_int_0);
static RULE(unsigned_dec_int_1);
static RULE(unsigned_dec_int_2);
static RULE(val);
static RULE(ws);
static RULE(ws_comment_newline);
static RULE(ws_comment_newline_0);
static RULE(ws_comment_newline_1);
static RULE(wschar);
static RULE(zero_prefixable_int);
static RULE(zero_prefixable_int_0);
static RULE(zero_prefixable_int_1);

// Overall Structure

RULE(TOML_0) {
	if (MATCH(newline))
	if (MATCH(expression))
	return 1;
	return 0;
}

RULE(TOML) {
	if (MATCH(expression))
	if (ANY(TOML_0))
	return 1;
	return 0;
}

RULE(expression_0) {
	if (MATCH(ws))
	if (MATCH(keyval))
	if (MATCH(ws))
	if (OPT(MATCH(comment)))
	return 1;
	return 0;
}

RULE(expression_1) {
	if (MATCH(ws))
	if (MATCH(table))
	if (MATCH(ws))
	if (OPT(MATCH(comment)))
	return 1;
	return 0;
}

RULE(expression_2) {
	if (MATCH(ws))
	if (OPT(MATCH(comment)))
	return 1;
	return 0;
}

RULE(expression) {
	if (!MATCH(expression_0))
	if (!MATCH(expression_1))
	if (!MATCH(expression_2))
	return 0;
	return 1;
}

// Whitespace

RULE(ws) { return ANY(wschar); }

RULE(wschar) { return SET(0x20, 0x09); } // space or horizontal tab

// Newline

RULE(newline) {
	if (!CHAR(0x0A)) // LF
	if (!STRING("\x0D\x0A")) // CRLF
	return 0;
	return 1;
}

// Comment

RULE(comment_start_symbol) { return CHAR(0x23); } // #

RULE(non_ascii) {
	if (!RANGE(0x80, 0xD7FF))
	if (!RANGE(0xE000, 0x10FFFF))
	return 0;
	return 1;
}

RULE(non_eol) {
	if (!CHAR(0x09))
	if (!RANGE(0x20, 0x7F))
	if (!MATCH(non_ascii))
	return 0;
	return 1;
}

RULE(comment) {
	if (MATCH(comment_start_symbol))
	if (ANY(non_eol))
	return 1;
	return 0;
}

// Key-Value pairs

RULE(keyval) {
	if (MATCH(key))
	if (MATCH(keyval_sep))
	if (MATCH(val))
	return 1;
	return 0;
}

RULE(key) {
	if (!MATCH(dotted_key))
	if (!MATCH(simple_key))
	return 0;
	return 1;
}

RULE(simple_key) {
	if (!MATCH(quoted_key))
	if (!MATCH(unquoted_key))
	return 0;
	return 1;
}

RULE(unquoted_key_0) {
	if (!MATCH(ALPHA))
	if (!MATCH(DIGIT))
	if (!SET(0x2D, 0x5F)) // - _
	return 0;
	return 1;
}

RULE(unquoted_key) { return SOME(unquoted_key_0); }

RULE(quoted_key) {
	if (!MATCH(basic_string))
	if (!MATCH(literal_string))
	return 0;
	return 1;
}

RULE(dotted_key_0) {
	if (MATCH(dot_sep))
	if (MATCH(simple_key))
	return 1;
	return 0;
}

RULE(dotted_key) {
	if (MATCH(simple_key))
	if (SOME(dotted_key_0))
	return 1;
	return 0;
}

RULE(dot_sep) {
	if (MATCH(ws))
	if (CHAR(0x2E)) // . Period
	if (MATCH(ws))
	return 1;
	return 0;
}

RULE(keyval_sep) {
	if (MATCH(ws))
	if (CHAR(0x3D)) // =
	if (MATCH(ws))
	return 1;
	return 0;
}

RULE(val) {
	if (!MATCH(string))
	if (!MATCH(boolean))
	if (!MATCH(array))
	if (!MATCH(inline_table))
	if (!MATCH(date_time))
	if (!MATCH(floating))
	if (!MATCH(integer))
	return 0;
	return 1;
}

// String

RULE(string) {
	if (!MATCH(ml_basic_string))
	if (!MATCH(basic_string))
	if (!MATCH(ml_literal_string))
	if (!MATCH(literal_string))
	return 0;
	return 1;
}

// Basic String

RULE(basic_string) {
	if (MATCH(quotation_mark))
	if (ANY(basic_char))
	if (MATCH(quotation_mark))
	return 1;
	return 0;
}

RULE(quotation_mark) { return CHAR(0x22); } // "

RULE(basic_char) {
	if (!MATCH(basic_unescaped))
	if (!MATCH(escaped))
	return 0;
	return 1;
}

RULE(basic_unescaped) {
	if (!MATCH(wschar))
	if (!CHAR(0x21))
	if (!RANGE(0x23, 0x5B))
	if (!RANGE(0x5D, 0x7E))
	if (!MATCH(non_ascii))
	return 0;
	return 1;
}

RULE(escaped) {
	if (MATCH(escape))
	if (MATCH(escape_seq_char))
	return 1;
	return 0;
}

RULE(escape) { return CHAR(0x5C); } // backslash

// "    quotation mark  U+0022
// \    reverse solidus U+005C
// b    backspace       U+0008
// f    form feed       U+000C
// n    line feed       U+000A
// r    carriage return U+000D
// t    tab             U+0009
RULE(escape_seq_char_0) { return SET(0x22, 0x5C, 0x62, 0x66, 0x6E, 0x72, 0x74); }

RULE(escape_seq_char_1) {
	// uXXXX                U+XXXX
	if (CHAR(0x75))
	if (REPEAT(HEXDIG, 4))
	return 1;
	return 0;
}

RULE(escape_seq_char_2) {
	// UXXXXXXXX            U+XXXXXXXX
	if (CHAR(0x55))
	if (REPEAT(HEXDIG, 8))
	return 1;
	return 0;
}

RULE(escape_seq_char) {
	if (!MATCH(escape_seq_char_0))
	if (!MATCH(escape_seq_char_1))
	if (!MATCH(escape_seq_char_2))
	return 0;
	return 1;
}

// Multiline Basic String

RULE(ml_basic_string) {
	if (MATCH(ml_basic_string_delim))
	if (OPT(MATCH(newline)))
	if (MATCH(ml_basic_body))
	if (MATCH(ml_basic_string_delim))
	return 1;
	return 0;
}

RULE(ml_basic_string_delim) { return REPEAT(quotation_mark, 3); }

RULE(ml_basic_body_0) {
	if (MATCH(mlb_quotes))
	if (SOME(mlb_content))
	return 1;
	return 0;
}

RULE(ml_basic_body) {
	if (ANY(mlb_content))
	if (ANY(ml_basic_body_0))
	if (OPT(MATCH(mlb_quotes)))
	return 1;
	return 0;
}

RULE(mlb_content) {
	if (!MATCH(mlb_char))
	if (!MATCH(newline))
	if (!MATCH(mlb_escaped_nl))
	return 0;
	return 1;
}

RULE(mlb_char) {
	if (!MATCH(mlb_unescaped))
	if (!MATCH(escaped))
	return 0;
	return 1;
}

RULE(mlb_quotes_0) { return REPEAT(quotation_mark, 2); }
RULE(mlb_quotes) {
	if (LOOKAHEAD(ml_basic_string_delim)) return 0;
	return SOME(mlb_quotes_0);
}

RULE(mlb_unescaped) {
	if (!MATCH(wschar))
	if (!CHAR(0x21))
	if (!RANGE(0x23, 0x5B))
	if (!RANGE(0x5D, 0x7E))
	if (!MATCH(non_ascii))
	return 0;
	return 1;
}

RULE(mlb_escaped_nl_0) {
	if (!MATCH(wschar))
	if (!MATCH(newline))
	return 0;
	return 1;
}

RULE(mlb_escaped_nl) {
	if (MATCH(escape))
	if (MATCH(ws))
	if (MATCH(newline))
	if (ANY(mlb_escaped_nl_0))
	return 1;
	return 0;
}

// Literal String

RULE(literal_string) {
	if (MATCH(apostrophe))
	if (ANY(literal_char))
	if (MATCH(apostrophe))
	return 1;
	return 0;
}

RULE(apostrophe) { return CHAR(0x27); } // '

RULE(literal_char) {
	if (!CHAR(0x09))
	if (!RANGE(0x20, 0x26))
	if (!RANGE(0x28, 0x7E))
	if (!MATCH(non_ascii))
	return 0;
	return 1;
}

// Multiline Literal String

RULE(ml_literal_string) {
	if (MATCH(ml_literal_string_delim))
	if (OPT(MATCH(newline)))
	if (MATCH(ml_literal_body))
	if (MATCH(ml_literal_string_delim))
	return 1;
	return 0;
}

RULE(ml_literal_string_delim) { return REPEAT(apostrophe, 3); }

RULE(ml_literal_body_0) {
	if (MATCH(mll_quotes))
	if (SOME(mll_content))
	return 1;
	return 0;
}

RULE(ml_literal_body) {
	if (ANY(mll_content))
	if (ANY(ml_literal_body_0))
	if (OPT(MATCH(mll_quotes)))
	return 1;
	return 0;
}

RULE(mll_content) {
	if (!MATCH(mll_char))
	if (!MATCH(newline))
	return 0;
	return 1;
}

RULE(mll_char) {
	if (!CHAR(0x09))
	if (!RANGE(0x20, 0x26)) {
		if (LOOKAHEAD(ml_literal_string_delim)) return 0;
		// XXX: Bug in grammar?  Range does not include 0x27
		// (apostrophe) but without it we will not match
		// raw-multiline-string.toml
		if (!MATCH(apostrophe))
		if (!RANGE(0x28, 0x7E))
		if (!MATCH(non_ascii))
		return 0;
		return 1;
	}
	return 1;
}

RULE(mll_quotes_0) { return REPEAT(apostrophe, 2); }
RULE(mll_quotes) {
	if (LOOKAHEAD(ml_literal_string_delim)) return 0;
	return SOME(mll_quotes_0);
}

// Integer

RULE(integer) {
	if (!MATCH(hex_int))
	if (!MATCH(oct_int))
	if (!MATCH(bin_int))
	if (!MATCH(dec_int))
	return 0;
	return 1;
}

RULE(minus) { return CHAR(0x2D); } // -
RULE(plus) { return CHAR(0x2B); } // +
RULE(underscore) { return CHAR(0x5F); } // _
RULE(digit1_9) { return RANGE(0x31, 0x39); } // 1-9
RULE(digit0_7) { return RANGE(0x30, 0x37); } // 0-7
RULE(digit0_1) { return RANGE(0x30, 0x31); } // 0-1

RULE(hex_prefix) { return STRING("\x30\x78"); } // 0x
RULE(oct_prefix) { return STRING("\x30\x6F"); } // 0o
RULE(bin_prefix) { return STRING("\x30\x62"); } // 0b

RULE(dec_int_0) {
	if (!MATCH(minus))
	if (!MATCH(plus))
	return 0;
	return 1;
}

RULE(dec_int) {
	if (OPT(MATCH(dec_int_0)))
	if (MATCH(unsigned_dec_int))
	return 1;
	return 0;
}

RULE(unsigned_dec_int_0) {
	if (MATCH(digit1_9))
	if (SOME(unsigned_dec_int_1))
	return 1;
	return 0;
}

RULE(unsigned_dec_int_1) {
	if (!MATCH(DIGIT))
	if (!MATCH(unsigned_dec_int_2))
	return 0;
	return 1;
}

RULE(unsigned_dec_int_2) {
	if (MATCH(underscore))
	if (MATCH(DIGIT))
	return 1;
	return 0;
}

RULE(unsigned_dec_int) {
	if (!MATCH(unsigned_dec_int_0))
	if (!MATCH(DIGIT))
	return 0;
	return 1;
}

RULE(hex_int_0) {
	if (!MATCH(HEXDIG))
	if (!MATCH(hex_int_1))
	return 0;
	return 1;
}

RULE(hex_int_1) {
	if (MATCH(underscore))
	if (MATCH(HEXDIG))
	return 1;
	return 0;
}

RULE(hex_int) {
	if (MATCH(hex_prefix))
	if (MATCH(HEXDIG))
	if (ANY(hex_int_0))
	return 1;
	return 0;
}

RULE(oct_int_0) {
	if (!MATCH(digit0_7))
	if (!MATCH(oct_int_1))
	return 0;
	return 1;
}

RULE(oct_int_1) {
	if (MATCH(underscore))
	if (MATCH(digit0_7))
	return 1;
	return 0;
}

RULE(oct_int) {
	if (MATCH(oct_prefix))
	if (MATCH(digit0_7))
	if (ANY(oct_int_0))
	return 1;
	return 0;
}

RULE(bin_int_0) {
	if (!MATCH(digit0_1))
	if (!MATCH(bin_int_1))
	return 0;
	return 1;
}

RULE(bin_int_1) {
	if (MATCH(underscore))
	if (MATCH(digit0_1))
	return 1;
	return 0;
}

RULE(bin_int) {
	if (MATCH(bin_prefix))
	if (MATCH(digit0_1))
	if (ANY(bin_int_0))
	return 1;
	return 0;
}

// Float

RULE(floating_0) {
	if (MATCH(float_int_part))
	if (MATCH(floating_1))
	return 1;
	return 0;
}

RULE(floating_1) {
	if (!MATCH(exponent))
	if (!MATCH(floating_2))
	return 0;
	return 1;
}

RULE(floating_2) {
	if (MATCH(frac))
	if (OPT(MATCH(exponent)))
	return 1;
	return 0;
}

RULE(floating) {
	if (MATCH(floating_0))
	if (MATCH(special_float))
	return 1;
	return 0;
}

RULE(float_int_part) { return MATCH(dec_int); }

RULE(frac) {
	if (MATCH(decimal_point))
	if (MATCH(zero_prefixable_int))
	return 1;
	return 0;
}

RULE(decimal_point) { return CHAR(0x2E); } // .

RULE(zero_prefixable_int_0) {
	if (!MATCH(DIGIT))
	if (!MATCH(zero_prefixable_int_1))
	return 0;
	return 1;
}

RULE(zero_prefixable_int_1) {
	if (MATCH(underscore))
	if (MATCH(DIGIT))
	return 1;
	return 0;
}

RULE(zero_prefixable_int) {
	if (MATCH(DIGIT))
	if (ANY(zero_prefixable_int_0))
	return 1;
	return 0;
}

RULE(exponent) {
	if (SET('e', 'E'))
	if (MATCH(float_exp_part))
	return 1;
	return 0;
}

RULE(float_exp_part_0) {
	if (!MATCH(minus))
	if (!MATCH(plus))
	return 0;
	return 1;
}

RULE(float_exp_part) {
	if (OPT(MATCH(float_exp_part_0)))
	if (MATCH(zero_prefixable_int))
	return 1;
	return 0;
}

RULE(special_float_0) {
	if (!MATCH(minus))
	if (!MATCH(plus))
	return 0;
	return 1;
}

RULE(special_float_1) {
	if (!MATCH(inf))
	if (!MATCH(nan_))
	return 0;
	return 1;
}

RULE(special_float) {
	if (!OPT(MATCH(special_float_0)))
	if (!MATCH(special_float_1))
	return 0;
	return 1;
}

RULE(inf) { return STRING("\x69\x6e\x66"); } // inf
RULE(nan_) { return STRING("\x6e\x61\x6e"); } // nan

// Boolean

RULE(boolean) {
	if (!MATCH(true_))
	if (!MATCH(false_))
	return 0;
	return 1;
}

RULE(true_) { return STRING("\x74\x72\x75\x65"); } // true
RULE(false_) { return STRING("\x66\x61\x6c\x73\x65"); } // false

// Date and Time (as defined in RFC 3339)

RULE(date_time) {
	if (!MATCH(offset_date_time))
	if (!MATCH(local_date_time))
	if (!MATCH(local_date))
	if (!MATCH(local_time))
	return 0;
	return 1;
}

RULE(date_fullyear) { return REPEAT(DIGIT, 4); }
RULE(date_month) { return REPEAT(DIGIT, 2); } // 01-12
RULE(date_mday) { return REPEAT(DIGIT, 2); } // 01-28, 01-29, 01-30, 01-31 based on month/year

RULE(time_delim) { return SET('T', 't', 0x20); }
RULE(time_hour) { return REPEAT(DIGIT, 2); } // 00-23
RULE(time_minute) { return REPEAT(DIGIT, 2); } // 00-59
RULE(time_second) { return REPEAT(DIGIT, 2); } // 00-58, 00-59, 00-60 based on leap second rules

RULE(time_secfrac) {
	if (CHAR('.'))
	if (SOME(DIGIT))
	return 1;
	return 0;
}

RULE(time_numoffset_0) { return SET('+', '-'); }

RULE(time_numoffset) {
	if (MATCH(time_numoffset_0))
	if (MATCH(time_hour))
	if (CHAR(':'))
	if (MATCH(time_minute))
	return 1;
	return 0;
}

RULE(time_offset) {
	if (!SET('Z', 'z'))
	if (!MATCH(time_numoffset))
	return 0;
	return 1;
}

RULE(partial_time) {
	if (MATCH(time_hour))
	if (CHAR(':'))
	if (MATCH(time_minute))
	if (CHAR(':'))
	if (MATCH(time_second))
	if (OPT(MATCH(time_secfrac)))
	return 1;
	return 0;
}

RULE(full_date) {
	if (MATCH(date_fullyear))
	if (CHAR('-'))
	if (MATCH(date_month))
	if (CHAR('-'))
	if (MATCH(date_mday))
	return 1;
	return 0;
}

RULE(full_time) {
	if (MATCH(partial_time))
	if (MATCH(time_offset))
	return 1;
	return 0;
}

// Offset Date-Time

RULE(offset_date_time) {
	if (MATCH(full_date))
	if (MATCH(time_delim))
	if (MATCH(full_time))
	return 1;
	return 0;
}

// Local Date-Time

RULE(local_date_time) {
	if (MATCH(full_date))
	if (MATCH(time_delim))
	if (MATCH(partial_time))
	return 1;
	return 0;
}

// Local Date

RULE(local_date) { return MATCH(full_date); }

// Local Time

RULE(local_time) { return MATCH(partial_time); }

// Array

RULE(array) {
	if (MATCH(array_open))
	if (OPT(MATCH(array_values)))
	if (MATCH(ws_comment_newline))
	if (MATCH(array_close))
	return 1;
	return 0;
}

RULE(array_open) { return CHAR(0x5B); } // [
RULE(array_close) { return CHAR(0x5D); } // ]

RULE(array_values_0) {
	if (MATCH(ws_comment_newline))
	if (MATCH(val))
	if (MATCH(ws_comment_newline))
	if (MATCH(array_sep))
	if (MATCH(array_values))
	return 1;
	return 0;
}

RULE(array_values_1) {
	if (MATCH(ws_comment_newline))
	if (MATCH(val))
	if (MATCH(ws_comment_newline))
	if (OPT(MATCH(array_sep)))
	return 1;
	return 0;
}

RULE(array_values) {
	if (!MATCH(array_values_0))
	if (!MATCH(array_values_1))
	return 0;
	return 1;
}

RULE(array_sep) { return CHAR(0x2C); } // , Comma

RULE(ws_comment_newline_0) {
	if (!MATCH(wschar))
	if (!MATCH(ws_comment_newline_1))
	return 0;
	return 1;
}

RULE(ws_comment_newline_1) {
	if (OPT(MATCH(comment)))
	if (MATCH(newline))
	return 1;
	return 0;
}

RULE(ws_comment_newline) { return ANY(ws_comment_newline_0); }

// Table

RULE(table) {
	if (!MATCH(std_table))
	if (!MATCH(array_table))
	return 0;
	return 1;
}

// Standard Table

RULE(std_table) {
	if (MATCH(std_table_open))
	if (MATCH(key))
	if (MATCH(std_table_close))
	return 1;
	return 0;
}

RULE(std_table_open) {
	if (CHAR(0x5B)) // [ Left square bracket
	if (MATCH(ws))
	return 1;
	return 0;
}

RULE(std_table_close) {
	if (MATCH(ws))
	if (CHAR(0x5D)) // ] Right square bracket
	return 1;
	return 0;
}

// Inline Table

RULE(inline_table) {
	if (MATCH(inline_table_open))
	if (OPT(MATCH(inline_table_keyvals)))
	if (MATCH(inline_table_close))
	return 1;
	return 0;
}

RULE(inline_table_open) {
	if (CHAR(0x7B)) // {
	if (MATCH(ws))
	return 1;
	return 0;
}
RULE(inline_table_close) {
	if (MATCH(ws))
	if (CHAR(0x7D)) // }
	return 1;
	return 0;
}

RULE(inline_table_sep) {
	if (MATCH(ws))
	if (CHAR(0x2C)) // , Comma
	if (MATCH(ws))
	return 1;
	return 0;
}

RULE(inline_table_keyvals_0) {
	if (MATCH(inline_table_sep))
	if (MATCH(inline_table_keyvals))
	return 1;
	return 0;
}

RULE(inline_table_keyvals) {
	if (MATCH(keyval))
	if (OPT(MATCH(inline_table_keyvals_0)))
	return 1;
	return 0;
}

// Array Table

RULE(array_table) {
	if (MATCH(array_table_open))
	if (MATCH(key))
	if (MATCH(array_table_close))
	return 1;
	return 0;
}

RULE(array_table_open) {
	if (STRING("\x5B\x5B")) // [[ Double left square bracket
	if (MATCH(ws))
	return 1;
	return 0;
}
RULE(array_table_close) {
	if (MATCH(ws))
	if (STRING("\x5D\x5D")) // ]] Double right square bracket
	return 1;
	return 0;
}

RULE(ALPHA) {
	if (!RANGE(0x41,0x5A)) // A-Z
	if (!RANGE(0x61,0x7A)) // a-z
	return 0;
	return 1;
}

RULE(DIGIT) { return RANGE(0x30, 0x39); } // 0-9

RULE(HEXDIG) {
	if (!MATCH(DIGIT))
	if (!RANGE('a', 'f'))
	if (!RANGE('A', 'F'))
	return 0;
	return 1;
}

RULE(peg_toml_decode) {
	if (MATCH(TOML))
	if (EOS())
	return 1;
	return 0;
}
