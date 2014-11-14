MAKE_INCLUDE_DIR := $(HOME)/src/make-4.1
CFLAGS = -Wall

tests = test001 test002 test003

-load pcre.so

.PHONY: check $(tests) clean

pcre.so: CFLAGS += -I$(MAKE_INCLUDE_DIR)
pcre.so: CFLAGS += $(shell pcre-config --cflags)
pcre.so: LDFLAGS += $(shell pcre-config --libs)
pcre.so: pcre.c
	$(CC) $(CFLAGS) -fPIC $(LDFLAGS) -shared -o $@ $<

check: $(tests)

test001:
	@if [ '$(m ^test$,test)' = test ] ; then \
	  echo '$@ PASSED'; \
	else \
	  echo '$@ FAILED'; \
	  return 1; \
	fi

test002: var = st
test002:
	@if [ '$(m ^te$(var)$$,test,e)' = test ] ; then \
	  echo '$@ PASSED'; \
	else \
	  echo '$@ FAILED'; \
	  return 1; \
	fi

test003:
	@if [ '$(m te(st),test)$0$1' = testtestst -a -z '$(m a,b)$0$1' ] ; then \
	  echo '$@ PASSED'; \
	else \
	  echo '$@ FAILED'; \
	  return 1; \
	fi

clean:
	$(RM) *.o *.so
