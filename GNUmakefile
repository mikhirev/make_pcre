-load pcre.so

MAKE_INCLUDE_DIR := $(HOME)/src/make-4.1
CFLAGS += -I$(MAKE_INCLUDE_DIR)

pcre.so: CFLAGS += $(shell pcre-config --cflags)
pcre.so: LDFLAGS += $(shell pcre-config --libs)
pcre.so: pcre.c
	$(CC) $(CFLAGS) -fPIC $(LDFLAGS) -shared -o $@ $<

ifeq ($(m ^pattern$,pattern), pattern)
check:
	@echo test PASSED
else
check:
	@echo test FAILED
endif
