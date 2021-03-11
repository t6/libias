.SUFFIXES: .test
.PHONY: all clean deps lint test

include Makefile.configure

CFLAGS+=	-std=gnu99 -I.

OBJS=		array.o \
		compats.o \
		diff.o \
		diffutil.o \
		json.o \
		map.o \
		mempool.o \
		peg.o \
		peg/clang.o \
		peg/json.o \
		peg/objget.o \
		peg/toml.o \
		queue.o \
		set.o \
		stack.o \
		utf8.o \
		util.o
TESTS=		tests/array/array.test \
		tests/json/json.test \
		tests/peg/IPv4.test \
		tests/peg/MOVED.test \
		tests/peg/json.test \
		tests/peg/range.test \
		tests/peg/toml.test \
		tests/queue/queue.test \
		tests/stack/stack.test \
		tests/util/str.test

all: libias.a

lint:
	@/bin/sh tests/lint.sh

test: ${TESTS}
	@/bin/sh tests/run.sh

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -o $@ -c $<

${TESTS}: libias.a
.o.test:
	${CC} ${LDFLAGS} -o $@ $< libias.a ${LDADD}

libias.a: ${OBJS}
	${AR} rcs libias.a ${OBJS}

#
array.o: config.h array.h diff.h util.h
compats.o: config.h
diff.o: config.h diff.h
diffutil.o: config.h array.h diff.h diffutil.h mempool.h util.h
json.o: config.h array.h json.h map.h mempool.h peg.h peg/json.h peg/objget.h stack.h util.h
map.o: config.h array.h map.h util.h
mempool.o: config.h map.h mempool.h stack.h util.h
peg.o: config.h array.h mempool.h peg.h queue.h set.h stack.h utf8.h util.h
peg/clang.o: config.h peg.h peg/grammar.h
peg/json.o: config.h peg.h peg/json.h peg/grammar.h
peg/objget.o: config.h peg.h peg/grammar.h peg/objget.h
peg/toml.o: config.h peg.h peg/toml.h peg/grammar.h
queue.o: config.h queue.h util.h
set.o: config.h array.h map.h set.h util.h
stack.o: config.h stack.h util.h
tests/array/array.o: config.h array.h test.h util.h
tests/diff/diffutil.o: config.h array.h diff.h diffutil.h test.h util.h
tests/json/json.o: config.h array.h map.h json.h test.h util.h
tests/peg/IPv4.o: config.h array.h mempool.h peg.h peg/grammar.h test.h util.h
tests/peg/MOVED.o: config.h peg.h peg/grammar.h test.h util.h
tests/peg/json.o: config.h json.h peg.h peg/json.h test.h util.h
tests/peg/range.o: config.h peg.h peg/grammar.h test.h util.h
tests/peg/toml.o: config.h peg.h peg/toml.h test.h util.h
tests/queue/queue.o: config.h queue.h test.h util.h
tests/stack/stack.o: config.h stack.h test.h util.h
tests/util/str.o: config.h array.h test.h util.h
utf8.o: config.h utf8.h
util.o: config.h array.h util.h

deps:
	@for f in $$(git ls-files | grep '.*\.c$$' | LC_ALL=C sort); do \
	awk '/^#include "/ { \
		if (!filename) { \
			printf("%s.o:", substr(FILENAME, 1, length(FILENAME) - 2)); \
			filename = 1; \
		} \
		printf(" %s", substr($$2, 2, length($$2) - 2)) \
	} \
	END { if (filename) { print "" } }' $$f; done > Makefile.deps
	@mv Makefile Makefile.bak
	@awk '/^#$$/ { print; deps = 1 } \
	deps && /^$$/ { deps = 0; system("cat Makefile.deps") } \
	!deps { print; }' Makefile.bak > Makefile
	@rm -f Makefile.bak Makefile.deps

clean:
	@find . -name '*.o' | xargs rm -f
	@rm -f *.a ${TESTS} config.*.old
