CFLAGS = -Wall -g
PCRE_CONFIG = pcre-config
#PCRE_CONFIG = pkg-config libpcre
PCRE_CFLAGS := $(shell $(PCRE_CONFIG) --cflags)
PCRE_LIBS := $(shell $(PCRE_CONFIG) --libs)
LIBS = $(PCRE_LIBS)
GNUMAKE4 = $(MAKE)

.PHONY: check clean

pcre.so: pcre.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MAKE_CFLAGS) -fPIC \
		$(LDFLAGS) -shared -o $@ $< $(LIBS)

check: pcre.so
	$(GNUMAKE4) -k -f tests.mk

clean:
	$(RM) *.o *.so
