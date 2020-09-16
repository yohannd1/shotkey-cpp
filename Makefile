# shotkey - Suckless hotkey daemon
# See LICENSE file for copyright and license details.

include config.mk

DEP := shotkey
SRC := $(patsubst %,%.cc,$(DEP))
OBJ := $(patsubst %,%.o,$(DEP))

all: clean options shotkey

options:
	@echo shotkey build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.o: %.cpp
	${CC} -c ${CFLAGS} $<

config.hh:
	cp config.def.hh config.hh

shotkey: $(OBJ)
	${CC} -o $@ $(OBJ) ${LDFLAGS}

clean:
	rm -f shotkey $(OBJ)

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f shotkey ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/shotkey
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < shotkey.1 > ${DESTDIR}${MANPREFIX}/man1/shotkey.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/shotkey.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/shotkey \
		${DESTDIR}${MANPREFIX}/man1/shotkey.1

run: shotkey
	./shotkey

.PHONY: all options clean dist install uninstall
