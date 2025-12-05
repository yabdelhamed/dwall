# dwall - dynamic wallpaper
# See LICENSE file for copyright and license details.

include config.mk

SRC = dwall.c
OBJ = ${SRC:.c=.o}

all: dwall

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	cp config.def.h $@

dwall: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dwall ${OBJ}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwall ${DESTDIR}${PREFIX}/bin
	cp -f gen-transitions.sh ${DESTDIR}${PREFIX}/bin/dwall-gen-transitions
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwall
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwall-gen-transitions

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwall
	rm -f ${DESTDIR}${PREFIX}/bin/dwall-gen-transitions

.PHONY: all clean install uninstall
