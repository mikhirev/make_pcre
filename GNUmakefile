MAKE_INCLUDE := -I$(HOME)/src/make-4.1
CFLAGS = -Wall -g

tests = test001 test002 test003 test004 test005

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

# test named string capturing
test004 = '$(m ^t(?<var>es)t$,test)' = test -a '$(var)' = es

# test with large number of strings to capture
test005: pat = (1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(14)(15)(16)(17)(18)(19)(20)(21)(22)(23)(24)(25)(26)(27)(28)(29)(30)(31)(32)(33)(34)(35)(36)(37)(38)(39)(40)(41)(42)(43)(44)(45)(46)(47)(48)(49)(50)(51)(52)(53)(54)(55)(56)(57)(58)(59)(60)(61)(62)(63)(64)(65)(66)(67)(68)(69)(70)(71)(72)(73)(74)(75)(76)(77)(78)(79)(80)(81)(82)(83)(84)(85)(86)(87)(88)(89)(90)(91)(92)(93)(94)(95)(96)(97)(98)(99)(100)(101)(102)(103)(104)(105)(106)(107)(108)(109)(110)(111)(112)(113)(114)(115)(116)(117)(118)(119)(120)(121)(122)(123)(124)(125)(126)(127)(128)(129)(130)(131)(132)(133)(134)(135)(136)(137)(138)(139)(140)(141)(142)(143)(144)(145)(146)(147)(148)(149)(150)(151)(152)(153)(154)(155)(156)(157)(158)(159)(160)(161)(162)(163)(164)(165)(166)(167)(168)(169)(170)(171)(172)(173)(174)(175)(176)(177)(178)(179)(180)(181)(182)(183)(184)(185)(186)(187)(188)(189)(190)(191)(192)(193)(194)(195)(196)(197)(198)(199)(200)(201)(202)(203)(204)(205)(206)(207)(208)(209)(210)(211)(212)(213)(214)(215)(216)(217)(218)(219)(220)(221)(222)(223)(224)(225)(226)(227)(228)(229)(230)(231)(232)(233)(234)(235)(236)(237)(238)(239)(240)(241)(242)(243)(244)(245)(246)(247)(248)(249)(250)(251)(252)(253)(254)(255)
test005: subj = 123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899100101102103104105106107108109110111112113114115116117118119120121122123124125126127128129130131132133134135136137138139140141142143144145146147148149150151152153154155156157158159160161162163164165166167168169170171172173174175176177178179180181182183184185186187188189190191192193194195196197198199200201202203204205206207208209210211212213214215216217218219220221222223224225226227228229230231232233234235236237238239240241242243244245246247248249250251252253254255
test005 = '$(m $(pat),$(subj),E)' = '$(subj)' -a '$(1)' = 1 -a '$(255)' = 255

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
