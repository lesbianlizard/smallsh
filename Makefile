CC = gcc
CFLAGS = -ggdb -Wall -Wextra -std=gnu11 -lreadline

GETTEXT_DOMAIN = smallsh-translations
GETTEXT_DIR = translations-dir

smallsh: main.c array.c config.h
	$(CC) $(CFLAGS) $< -o $@

locales: po/fr/$(GETTEXT_DOMAIN).mo

po/fr/$(GETTEXT_DOMAIN).mo: po/fr/$(GETTEXT_DOMAIN).po
	msgfmt --output-file=$@ $<

po/fr/$(GETTEXT_DOMAIN).po: po/$(GETTEXT_DOMAIN).pot
	msgmerge --update $@ $<

po/$(GETTEXT_DOMAIN).pot: main.c 
	xgettext --keyword=_ --join-existing --output=po/$(GETTEXT_DOMAIN).pot main.c

.PHONY: clean
clean:
	rm -v smallsh po/fr/*.mo
