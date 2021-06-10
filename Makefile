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
		str.o \
		stack.o \
		utf8.o \
		util.o
ALL_TESTS=	tests/array/array.test \
		tests/diff/diffutil.test \
		tests/json/json.test \
		tests/map/map.test \
		tests/peg/IPv4.test \
		tests/peg/MOVED.test \
		tests/peg/json.test \
		tests/peg/range.test \
		tests/peg/toml.test \
		tests/queue/queue.test \
		tests/stack/stack.test \
		tests/str/str.test
TESTS?=		${ALL_TESTS}

all: libias.a

lint:
	@/bin/sh tests/lint.sh

test: ${TESTS}
	@/bin/sh tests/run.sh ${TESTS}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -o $@ -c $<

${TESTS}: libias.a
.o.test:
	${CC} ${LDFLAGS} -o $@ $< libias.a ${LDADD}

libias.a: ${OBJS}
	${AR} rcs libias.a ${OBJS}

#
array.o: config.h array.h diff.h mempool.h util.h
compats.o: config.h
diff.o: config.h diff.h
diffutil.o: config.h array.h diff.h diffutil.h mempool.h str.h util.h
json.o: config.h array.h json.h map.h mempool.h peg.h peg/json.h peg/objget.h stack.h str.h util.h
map.o: config.h array.h map.h util.h
mempool.o: config.h array.h map.h mempool.h queue.h set.h stack.h util.h
peg.o: config.h array.h mempool.h peg.h queue.h set.h stack.h str.h utf8.h util.h
peg/clang.o: config.h peg.h peg/grammar.h
peg/json.o: config.h peg.h peg/json.h peg/grammar.h
peg/objget.o: config.h peg.h peg/grammar.h peg/objget.h
peg/toml.o: config.h peg.h peg/toml.h peg/grammar.h
queue.o: config.h queue.h util.h
set.o: config.h array.h map.h set.h util.h
stack.o: config.h stack.h util.h
str.o: config.h array.h mempool.h str.h util.h
tests/array/array.o: config.h array.h mempool.h str.h test.h util.h
tests/diff/diffutil.o: config.h array.h diff.h diffutil.h mempool.h str.h test.h util.h
tests/json/json.o: config.h array.h map.h json.h mempool.h str.h test.h util.h
tests/map/map.o: config.h map.h mempool.h test.h str.h util.h
tests/peg/IPv4.o: config.h array.h mempool.h peg.h peg/grammar.h str.h test.h util.h
tests/peg/MOVED.o: config.h mempool.h peg.h peg/grammar.h str.h test.h util.h
tests/peg/json.o: config.h json.h mempool.h peg.h peg/json.h str.h test.h util.h
tests/peg/range.o: config.h mempool.h peg.h peg/grammar.h str.h test.h util.h
tests/peg/toml.o: config.h mempool.h peg.h peg/toml.h str.h test.h util.h
tests/queue/queue.o: config.h mempool.h queue.h str.h test.h util.h
tests/stack/stack.o: config.h mempool.h stack.h str.h test.h util.h
tests/str/str.o: config.h array.h mempool.h str.h test.h util.h
utf8.o: config.h utf8.h
util.o: config.h array.h mempool.h str.h util.h

deps:
	@for f in $$(git ls-files | grep '.*\.c$$' | grep -v '^tests\.c$$' | LC_ALL=C sort); do \
		${CC} ${CFLAGS} -MM -MT "$${f%.c}.o" $${f} | sed 's/[\\ ]/\n/g' | grep -vF "$${f}" | tr -s '\n' ' ' | sed 's/ $$//'; \
		echo; \
	done > Makefile.deps
	@mv Makefile Makefile.bak
	@awk '/^#$$/ { print; deps = 1 } \
	deps && /^$$/ { deps = 0; system("cat Makefile.deps") } \
	!deps { print; }' Makefile.bak > Makefile
	@rm -f Makefile.bak Makefile.deps

clean:
	@find . -name '*.o' | xargs rm -f
	@rm -f *.a ${ALL_TESTS} config.*.old
