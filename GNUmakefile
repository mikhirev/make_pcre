MAKE_INCLUDE := -I$(HOME)/src/make-4.1
CFLAGS = -Wall

tests = test001 test002 test003

-load pcre.so

.PHONY: check clean

pcre.so: CFLAGS += $(shell pcre-config --cflags)
pcre.so: LDFLAGS += $(shell pcre-config --libs)
pcre.so: pcre.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MAKE_INCLUDE) -fPIC $(LDFLAGS) -shared -o $@ $<

check: $(tests)


### TEST EXPRESSIONS ###
# each expression is passed to test(1)

# simple test for pattern matching
test001 = '$(m ^test$,test)' = test

# test for pattern expansion
test002: var = es
test002 = '$(m ^t$(var)t$$,test,E)' = test

# test for string capturing by number
test003 = '$(m ^t(es)t$,test)$0$1' = testtestes -a -z '$(m a,b)$0$1'

### END OF TEST EXPRESSIONS ###


test%:
	@if [ $($@) ] ; then \
		echo '$@ PASSED'; \
	else \
		echo '$@ FAILED'; \
		return 1; \
	fi

clean:
	$(RM) *.o *.so
