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

#include "tests/peg/common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static RULE(byte);
static RULE(byte_1);
static RULE(byte_2);
static RULE(byte_3);
static RULE(dig);
static RULE(ipv4);
static RULE(ipv4_address);
static RULE(num0_4);
static RULE(num0_5);

struct IPv4Capture {
	char *bytes[5];
	char *full;
};

enum IPv4CaptureState {
	ACCEPT = 0,
	BYTE1,
	BYTE2,
	BYTE3,
	BYTE4,
	BYTE25,
};

CAPTURE_MACHINE(enum IPv4CaptureState, struct IPv4Capture) {
	switch (state) {
	case BYTE1:
		data->bytes[0] = capture->buf;
		break;
	case BYTE2:
		data->bytes[1] = capture->buf;
		break;
	case BYTE3:
		data->bytes[2] = capture->buf;
		break;
	case BYTE4:
		data->bytes[3] = capture->buf;
		break;
	case BYTE25:
		data->bytes[4] = capture->buf;
		break;
	case ACCEPT:
		data->full = capture->buf;
		break;
	}
	return PEG_CAPTURE_KEEP;
}

RULE(dig) { return RANGE('0', '9'); }
RULE(num0_4) { return RANGE('0', '4'); }
RULE(num0_5) { return RANGE('0', '5'); }
RULE(byte_1) { if (CAPTURE(STRING("25"), 2, BYTE25)) if (MATCH(num0_5)) return 1; return 0; }
RULE(byte_2) { if (CHAR('2')) if (MATCH(num0_4)) if (MATCH(dig)) return 1; return 0; }
RULE(byte_3) { if (CHAR('1')) if (MATCH(dig)) if (MATCH(dig)) return 1; return 0; }
RULE(byte) {
	if (!MATCH(byte_1))
	if (!MATCH(byte_2))
	if (!MATCH(byte_3))
	if (!BETWEEN(dig, 1, 2))
	return 0;
	return 1;
}

RULE(ipv4_address) {
	if (CAPTURE(MATCH(byte), 0, BYTE1))
	if (CHAR('.'))
	if (CAPTURE(MATCH(byte), 0, BYTE2))
	if (CHAR('.'))
	if (CAPTURE(MATCH(byte), 0, BYTE3))
	if (CHAR('.'))
	if (CAPTURE(MATCH(byte), 0, BYTE4))
	return 1;
	return 0;
}

RULE(ipv4) {
	if (MATCH(ipv4_address))
	if (EOS())
	return 1;
	return 0;
}

TESTS() {
	TEST(check_match(ipv4, "10.240.250.250", 1));
	TEST(check_match(ipv4, "0.0.0.0", 1));
	TEST_STREQ(check_captures(ipv4, "10.240.250.250", 0, "@"), "10@240@250@250");
	TEST_STREQ(check_captures(ipv4, "10.240.250.250", 2, "@"), "25@25");

	struct PEG *peg;
	struct IPv4Capture capture;
	memset(&capture, 0, sizeof(capture));
	TEST_IF((peg = peg_new("1.2.3.4", 7)) && peg_match(peg, ipv4, &capture)) {
		TEST_STREQ(capture.full, "1.2.3.4");
		TEST_STREQ(capture.bytes[0], "1");
		TEST_STREQ(capture.bytes[1], "2");
		TEST_STREQ(capture.bytes[2], "3");
		TEST_STREQ(capture.bytes[3], "4");
		TEST(capture.bytes[4] == NULL);
	}
	peg_free(peg);
	peg = NULL;

	memset(&capture, 0, sizeof(capture));
	TEST_IF((peg = peg_new("255.2.3.4", 9)) && peg_match(peg, ipv4, &capture)) {
		TEST_STREQ(capture.full, "255.2.3.4");
		TEST_STREQ(capture.bytes[0], "255");
		TEST_STREQ(capture.bytes[1], "2");
		TEST_STREQ(capture.bytes[2], "3");
		TEST_STREQ(capture.bytes[3], "4");
		TEST_STREQ(capture.bytes[4], "25");
	}
	peg_free(peg);
	peg = NULL;

	// Can we partially match an ipv4 address with trailing garbage?
	// Does the capture machine return the correct full match?
	memset(&capture, 0, sizeof(capture));
	TEST_IF((peg = peg_new("255.2.3.4garbage", 16)) && peg_match(peg, ipv4_address, &capture)) {
		TEST_STREQ(capture.full, "255.2.3.4");
		TEST_STREQ(capture.bytes[0], "255");
		TEST_STREQ(capture.bytes[1], "2");
		TEST_STREQ(capture.bytes[2], "3");
		TEST_STREQ(capture.bytes[3], "4");
		TEST_STREQ(capture.bytes[4], "25");
	}
	peg_free(peg);
	peg = NULL;

	TEST(check_match(ipv4, "256.0.0.0", 0));
	TEST(check_match(ipv4, "256.2.3.4", 0));
	TEST(check_match(ipv4, "256.2.3.2514", 0));
}
