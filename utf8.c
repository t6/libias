/*-
 * utf8_{de,en}code are derived from st.c from git://git.suckless.org/st:
 * 
 * ISC-License
 * 
 * (c) 2014-2017 Markus Teich <markus.teich@stusta.mhn.de>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
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
 * 
 * (c) 2016,2017 Laslo Hunhold <dev@frign.de>
 * (c) 2016 ssd <ssd@mailless.org>
 * (c) 2016 Hiltjo Posthuma <hiltjo@codemadness.org>
 * (c) 2015 David Phillips <dbphillipsnz@gmail.com>
 * (c) 2015 Grant Mathews <grant.m.mathews@gmail.com>
 * (c) 2015 Dimitris Papastamos <sin@2f30.org>
 * (c) 2015 Alexis <surryhill@gmail.com>
 * (c) 2015 Quentin Rameau <quinq@fifth.space>
 * (c) 2015 Ivan Tham <pickfire@riseup.net>
 * (c) 2015 Jan Christoph Ebersbach <jceb@e-jc.de>
 * (c) 2015 Tony Lainson <t.lainson@gmail.com>
 * (c) 2015 Szabolcs Nagy <nsz@port70.net>
 * (c) 2015 Jonas Jelten <jj@sft.mx>
 * 
 */

#include "config.h"

#include <string.h>

#include "utf8.h"

#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

static const unsigned char utfbyte[UTF_SIZE + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static const unsigned char utfmask[UTF_SIZE + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const uint32_t utfmin[UTF_SIZE + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static const uint32_t utfmax[UTF_SIZE + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

static uint32_t
utf8_decodebyte(const char c, size_t *i)
{
	for (*i = 0; *i < (UTF_SIZE + 1); ++(*i)) {
		if (((unsigned char)c & utfmask[*i]) == utfbyte[*i]) {
			return (unsigned char)c & ~utfmask[*i];
		}
	}
	return 0;
}

static size_t
utf8_validate(uint32_t *u, size_t i)
{
	if (!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF)) {
		*u = UTF_INVALID;
	}
	for (i = 1; *u > utfmax[i]; ++i);
	return i;
}

size_t
utf8_decode(const char *c, size_t clen, uint32_t *u)
{
	size_t i, j, len, type;
	uint32_t udecoded;

	*u = UTF_INVALID;
	if (!clen) {
		return 0;
	}
	udecoded = utf8_decodebyte(c[0], &len);
	if (!BETWEEN(len, 1, UTF_SIZE)) {
		return 1;
	}
	for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
		udecoded = (udecoded << 6) | utf8_decodebyte(c[i], &type);
		if (type) {
			return j;
		}
	}
	if (j < len) {
		return 0;
	}
	*u = udecoded;
	utf8_validate(u, len);

	return len;
}

static char
utf8_encodebyte(uint32_t u, size_t i)
{
	return utfbyte[i] | (u & ~utfmask[i]);
}

size_t
utf8_encode(uint32_t u, char *c)
{
	memset(c, 0, UTF_SIZE + 1);

	size_t len = utf8_validate(&u, 0);
	if (len > UTF_SIZE) {
		return 0;
	}

	for (size_t i = len - 1; i != 0; --i) {
		c[i] = utf8_encodebyte(u, 0);
		u >>= 6;
	}
	c[0] = utf8_encodebyte(u, len);

	return len;
}
