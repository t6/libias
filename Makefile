.SUFFIXES: .test
.PHONY: all clean lint test

include Makefile.configure

CFLAGS+=	-std=gnu99 -I.

OBJS=		array.o compats.o diff.o diffutil.o json.o map.o peg.o set.o \
		utf8.o util.o
TESTS=		tests/array/array.test \
		tests/peg/IPv4.test \
		tests/peg/MOVED.test \
		tests/peg/json.test \
		tests/peg/range.test \
		tests/util/str.test
TESTS_PEG!=	echo ${TESTS} | tr ' ' '\n' | grep '^tests/peg/' | sed 's,\.test$$,.o,'
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
map.o: config.h array.h map.h util.h
peg.o: config.h array.h peg.h utf8.h util.h
peg/clang.o: config.h peg.h peg/macros.h
peg/json.o: config.h peg.h peg/macros.h
set.o: config.h array.h map.h set.h util.h
utf8.o: config.h utf8.h
util.o: config.h array.h util.h
${TESTS_PEG}: config.h array.h peg.h peg/macros.h test.h tests/peg/common.h util.h
${TESTS_UTIL}: config.h array.h test.h util.h

clean:
	@find . -name '*.o' | xargs rm -f
	@rm -f *.a ${TESTS} config.*.old
