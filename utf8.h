/*-
 * The utf-8 module is derived from the utf-8 functions in
 * git://git.suckless.org/sent
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
 */
#pragma once

#define UTF_INVALID	0xFFFD
#define UTF_SIZE	4

size_t utf8_decode(const char *, size_t, uint32_t *);
size_t utf8_encode(uint32_t, char *);
