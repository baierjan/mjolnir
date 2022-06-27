CC=gcc
CFLAGS+=-Wall -Werror -Wpedantic
LDLIBS+=-lhidapi-hidraw

DESTDIR?=/usr/local
BINDIR?=/bin

.PHONY: build
build: mjolnir

.PHONY: install
install: build
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 755 mjolnir "$(DESTDIR)$(BINDIR)/mjolnir"

.PHONY: clean
clean:
	$(RM) mjolnir
