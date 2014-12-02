CFLAGS = -Wall -g
PCRE_CONFIG = pcre-config
#PCRE_CONFIG = pkg-config libpcre
PCRE_CFLAGS := $(shell $(PCRE_CONFIG) --cflags)
PCRE_LIBS := $(shell $(PCRE_CONFIG) --libs)
LIBS = $(PCRE_LIBS)

pcre.so: pcre.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MAKE_CFLAGS) -fPIC \
		$(LDFLAGS) -shared -o $@ $< $(LIBS)

check:
	@if [ -n "`which $(GNUMAKE4)`" ] ; then \
		$(GNUMAKE4) -k -f tests.mk ; \
	else \
		echo 'you need GNU make 4.x to run tests' ; \
		return 1 ; \
	fi

clean:
	$(RM) *.o *.so
