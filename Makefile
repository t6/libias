include Makefile.configure

CFLAGS+=	-std=gnu99 -I.

OBJS=		array.o compats.o diff.o diffutil.o map.o peg.o set.o util.o

all: libias.a

test: peg-test-IPv4 peg-test-MOVED
	./peg-test-IPv4
	./peg-test-MOVED

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -o $@ -c $<

libias.a: ${OBJS}
	${AR} rcs libias.a ${OBJS}

peg-test-IPv4: peg-test-IPv4.o libias.a
	${CC} ${LDFLAGS} -o peg-test-IPv4 peg-test-IPv4.o libias.a ${LDADD}

peg-test-MOVED: peg-test-MOVED.o libias.a
	${CC} ${LDFLAGS} -o peg-test-MOVED peg-test-MOVED.o libias.a ${LDADD}

array.o: config.h array.h diff.h util.h
diff.o: config.h diff.h
diffutil.o: config.h array.h diff.h diffutil.h util.h
map.o: config.h array.h map.h util.h
peg.o: config.h peg.h util.h
set.o: config.h array.h map.h set.h util.h
util.o: config.h array.h util.h

clean:
	@rm -f *.a *.o peg-test-MOVED config.*.old

.PHONY: clean test
