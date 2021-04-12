#!/bin/sh
set -u
ROOT="${PWD}"

tests_failed=0
tests_run=0

cd "${ROOT}"
if [ $# -gt 0 ]; then
	TESTS="$(echo "$@" | tr ' ' '\n' | sed 's,^tests/,,' | sort)"
else
	TESTS="$(find tests -name '*.test' | sed 's,^tests/,,' | sort)"
fi
for test in ${TESTS}; do
	echo -n "${test%*.test}: "
	if ! "tests/${test}"; then
		tests_run=$((tests_run + 1))
		tests_failed=$((tests_failed + 1))
		continue
	fi
	tests_run=$((tests_run + 1))
done

printf "fail: %s ok: %s/%s\n" "${tests_failed}" "$((tests_run - tests_failed))" "${tests_run}"
if [ "${tests_failed}" -gt 0 ]; then
	exit 1
fi
