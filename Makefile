.SUFFIXES: .test
.PHONY: all clean lint test

include Makefile.configure

CFLAGS+=	-std=gnu99 -I.

OBJS=		array.o \
		compats.o \
		diff.o \
		diffutil.o \
		json.o \
		map.o \
		memorypool.o \
		peg.o \
		peg/clang.o \
		peg/json.o \
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
		tests/queue/queue.test \
		tests/stack/stack.test \
		tests/util/str.test
TESTS_ARRAY!=	echo ${TESTS} | tr ' ' '\n' | grep '^tests/array/' | sed 's,\.test$$,.o,'
TESTS_JSON!=	echo ${TESTS} | tr ' ' '\n' | grep '^tests/json/' | sed 's,\.test$$,.o,'
TESTS_PEG!=	echo ${TESTS} | tr ' ' '\n' | grep '^tests/peg/' | sed 's,\.test$$,.o,'
TESTS_QUEUE!=	echo ${TESTS} | tr ' ' '\n' | grep '^tests/queue/' | sed 's,\.test$$,.o,'
TESTS_STACK!=	echo ${TESTS} | tr ' ' '\n' | grep '^tests/stack/' | sed 's,\.test$$,.o,'
TESTS_UTIL!=	echo ${TESTS} | tr ' ' '\n' | grep '^tests/util/' | sed 's,\.test$$,.o,'

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

array.o: config.h array.h diff.h util.h
diff.o: config.h diff.h
diffutil.o: config.h array.h diff.h diffutil.h util.h
json.o: config.h array.h json.h map.h memorypool.h peg.h peg/json.h stack.h util.h
map.o: config.h array.h map.h util.h
memorypool.o: config.h memorypool.h queue.h util.h
peg.o: config.h array.h memorypool.h peg.h queue.h stack.h utf8.h util.h
peg/clang.o: config.h peg.h peg/macros.h
peg/json.o: config.h array.h map.h peg.h peg/json.h peg/macros.h stack.h util.h
queue.o: config.h queue.h util.h
set.o: config.h array.h map.h set.h util.h
stack.o: config.h stack.h util.h
utf8.o: config.h utf8.h
util.o: config.h array.h util.h
${TESTS_ARRAY}: config.h array.h test.h util.h
${TESTS_JSON}: config.h array.h json.h test.h util.h
${TESTS_PEG}: config.h array.h memorypool.h peg.h peg/macros.h test.h tests/peg/common.h util.h
${TESTS_QUEUE}: config.h array.h queue.h test.h util.h
${TESTS_STACK}: config.h array.h stack.h test.h util.h
${TESTS_UTIL}: config.h array.h test.h util.h

clean:
	@find . -name '*.o' | xargs rm -f
	@rm -f *.a ${TESTS} config.*.old
