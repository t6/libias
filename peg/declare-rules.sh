#!/bin/sh
set -u
file="$1"
tmp="$(mktemp)"
awk '{if (match($1, /^RULE\([A-Za-z0-9_]+\)/)) {
	name = substr($1, RSTART + 5, RLENGTH - 6)
	if (name !~ /^peg_/) {
		printf "static RULE(%s);\n", name
	}
}}' "${file}" | LC_ALL=C sort -u >"${tmp}"
mv "$1" "$1.bak"
awk -v tmp="${tmp}" '!mark && /^\/\/$/ { print; mark = 1 } \
mark && /^$/ { mark = 0; system("cat " tmp) } \
!mark { print; }' "$1.bak" >"$1"
rm -f "${tmp}" "$1.bak"
