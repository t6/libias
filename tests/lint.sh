#!/bin/sh
set -u
CPPCHECK="cppcheck"
MANDOC="mandoc"

lint_failed=0
lint_run=0

echo -n "cppcheck: "
srcs="$(git ls-files | grep '\.c$' | grep -v '^tests.c$')"
if [ -z "${srcs}" ] || ! type ${CPPCHECK} >/dev/null 2>&1; then
	echo "skipped"
else
	lint_run=$((lint_run + 1))
	if ${CPPCHECK} --error-exitcode=1 --library=posix --quiet --force --inconclusive ${srcs}; then
		echo "ok"
	else
		echo
		lint_failed=$((lint_failed + 1))
	fi
fi

echo -n "mandoc: "
mans="$(git ls-files | grep '\.1$')"
if [ -z "${mans}" ] || ! type ${MANDOC} >/dev/null 2>&1; then
	echo "skipped"
else
	lint_run=$((lint_run + 1))
	if ${MANDOC} -Tlint -Wstyle ${mans}; then
		echo "ok"
	else
		echo
		lint_failed=$((lint_failed + 1))
	fi
fi

printf "fail: %s ok: %s/%s\n" "${lint_failed}" "$((lint_run - lint_failed))" "${lint_run}"
if [ "${lint_failed}" -gt 0 ]; then
	exit 1
fi
