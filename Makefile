CC = gcc
CFLAGS = -ggdb -Wall -Wextra -std=gnu11 -lreadline

GETTEXT_DOMAIN = smallsh-translations
GETTEXT_DIR = translations-dir
PO_DIR = po
LANGS = fr es
#@echo asdf
$(info $(LANGS))

.PHONY: all clean
# FIXME: why does
.PRECIOUS: $(PO_DIR)/%/$(GETTEXT_DOMAIN).po
# work, but this doesn't?
#.SECONDARY: $(PO_DIR)/%/$(GETTEXT_DOMAIN).po

all: locales smallsh

smallsh: main.c array.c config.h
	$(CC) $(CFLAGS) $< -o $@

locales: $(PO_DIR)/$(LANGS)/$(GETTEXT_DOMAIN).mo

$(PO_DIR)/%/$(GETTEXT_DOMAIN).mo: $(PO_DIR)/%/$(GETTEXT_DOMAIN).po
	msgfmt --output-file=$@ $<
	ln -srvf $@ $(GETTEXT_DIR)/$*/LC_MESSAGES

$(PO_DIR)/%/$(GETTEXT_DOMAIN).po: $(PO_DIR)/$(GETTEXT_DOMAIN).pot
  ifeq ($(wildcard $@), )
		mkdir -pv $(PO_DIR)/$*
		msginit --input=$< --locale=$* --output=$@
  else
		msgmerge --update $@ $<
  endif

$(PO_DIR)/$(GETTEXT_DOMAIN).pot: main.c 
	xgettext --keyword=_ --join-existing --output=$(PO_DIR)/$(GETTEXT_DOMAIN).pot main.c

clean:
	rm -v smallsh $(PO_DIR)/$(LANGS)/*.mo $(GETTEXT_DIR)/$(LANGS)/LC_MESSAGES/*.mo
