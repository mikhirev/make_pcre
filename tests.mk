ifneq ($(findstring 4.,$(MAKE_VERSION)),4.)
    $(error you need GNU make 4.x to run tests)
endif

NUMTESTS = 36
tests := $(foreach num,$(shell seq -f%03g $(NUMTESTS)),test$(num))

load pcre.so

all: $(tests)


### TEST EXPRESSIONS ###
# each expression is passed to test(1)
# don't use single quote character in expressions!

# simple test for pattern matching
test001 = "$(m ^test$,test)" = test

# test pattern expansion
test002: var = es
test002 = "$(m ^t$(var)t$$,test,E)" = test

# test string capturing by number
test003 = "$(m ^t(es)t$,test)$0$1" = testtestes -a -z "$(m a,b)$0$1"

# test named string capturing
test004 = "$(m ^t(?<var>es)t$,test)" = test -a "$(var)" = es

# test with large number of strings to capture
test005: pat = (1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(14)(15)(16)(17)(18)(19)(20)(21)(22)(23)(24)(25)(26)(27)(28)(29)(30)(31)(32)(33)(34)(35)(36)(37)(38)(39)(40)(41)(42)(43)(44)(45)(46)(47)(48)(49)(50)(51)(52)(53)(54)(55)(56)(57)(58)(59)(60)(61)(62)(63)(64)(65)(66)(67)(68)(69)(70)(71)(72)(73)(74)(75)(76)(77)(78)(79)(80)(81)(82)(83)(84)(85)(86)(87)(88)(89)(90)(91)(92)(93)(94)(95)(96)(97)(98)(99)(100)(101)(102)(103)(104)(105)(106)(107)(108)(109)(110)(111)(112)(113)(114)(115)(116)(117)(118)(119)(120)(121)(122)(123)(124)(125)(126)(127)(128)(129)(130)(131)(132)(133)(134)(135)(136)(137)(138)(139)(140)(141)(142)(143)(144)(145)(146)(147)(148)(149)(150)(151)(152)(153)(154)(155)(156)(157)(158)(159)(160)(161)(162)(163)(164)(165)(166)(167)(168)(169)(170)(171)(172)(173)(174)(175)(176)(177)(178)(179)(180)(181)(182)(183)(184)(185)(186)(187)(188)(189)(190)(191)(192)(193)(194)(195)(196)(197)(198)(199)(200)(201)(202)(203)(204)(205)(206)(207)(208)(209)(210)(211)(212)(213)(214)(215)(216)(217)(218)(219)(220)(221)(222)(223)(224)(225)(226)(227)(228)(229)(230)(231)(232)(233)(234)(235)(236)(237)(238)(239)(240)(241)(242)(243)(244)(245)(246)(247)(248)(249)(250)(251)(252)(253)(254)(255)
test005: subj = 123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899100101102103104105106107108109110111112113114115116117118119120121122123124125126127128129130131132133134135136137138139140141142143144145146147148149150151152153154155156157158159160161162163164165166167168169170171172173174175176177178179180181182183184185186187188189190191192193194195196197198199200201202203204205206207208209210211212213214215216217218219220221222223224225226227228229230231232233234235236237238239240241242243244245246247248249250251252253254255
test005 = "$(m $(pat),$(subj),E)" = "$(subj)" -a "$(1)" = 1 -a "$(255)" = 255

# test parsing pattern options
test006 = "$(m ^TEST+,testtttt,iU)" = test

# test passing `$' characters to variable
test007 = "$(m a(.*)b,a\$$b)" = "a\$$b" -a "$(1)" = "\$$"

# test global search
test008 = "$(m test\d,test1test2test3,g)" = "test1 test2 test3" -a \
          "$(m test,TestesT,gi)" = "Test"

# check that search performed only once without `g" option
test009 = "$(m test\d,test1test2test3)" = "test1"

# test regexp compilation error
#load pcre.so
#a := $(m {2}test,test)
#all:
#	@true
test010 = -n "$(shell \
    ( \
    echo load\ pcre.so ; \
    echo a\ :=\ \$$\(m\ \{2\}test,test\) ; \
    echo all: ; \
    echo \	@true; \
    ) | $(MAKE) -f - 2>&1 | \
    fgrep \*\*\*\ m:\ 2:)"

# test pattern substitution
test011 = "$(s a,x,a)" = "x"
test012 = "$(s a,x,abcd)" = "xbcd"
test013 = "$(s a,x,dcba)" = "dcbx"
test014 = "$(s a,x,abba,g)" = "xbbx"
test015 = "$(s a,xxx,a)" = "xxx"
test016 = "$(s a,xxx,abcd)" = "xxxbcd"
test017 = "$(s a,xxx,dcba)" = "dcbxxx"
test018 = "$(s a,xxx,abba,g)" = "xxxbbxxx"
test019 = "$(s aaa,x,aaa)" = "x"
test020 = "$(s aaa,x,aaabcd)" = "xbcd"
test021 = "$(s aaa,x,dcbaaa)" = "dcbx"
test022 = "$(s aaa,x,aaabbaaa,g)" = "xbbx"
test023 = "$(s a,x,aaa,g)" = "xxx"

# test expansion of substituted string
test024 = "$(s a(.),$(1),aaabac,g)" = "abc"
test025 = "$(s (?<var>.)a,$(var),aabaca,g)" = "abc"

# test extended regexp
define re026
[abc] # comment
[def] # comment
endef
test026 = "$(s $(re026),x,dadfbe,Egx)" = dxfx

# test Unicode properties and UTF-8 support
test027 = "$(s \w+,слово,тест тест,gu8)" = "слово слово"

# test compilation error in pcre_subst and uncoupled brackets in message
#load pcre.so
#a := $(s (?a),b,a)
#all:
#	@true
test028 = -n "$(shell \
    ( \
    echo load\ pcre.so ; \
    echo a\ :=\ \$$\(s\ \(?a\),b,a\) ; \
    echo all: ; \
    echo \	@true; \
    ) | $(MAKE) -f - 2>&1 | \
    fgrep \*\*\*\ s:\ 2:)"

# test multi-line subject string
define subj029
test
test
endef
test029 = -z "$(m ^test$,$(subj029))" -a "$(m ^test$,$(subj029),m)" = test

# test `s' option
test030 = -z "$(m test.test,$(subj029))" -a \
	  "$(s test.test,passed,$(subj029),s)" = passed

# test zero-length matching
test031 = "$(m x?,aaa,g)" = ""
test032 = "$(s x?,!,abcd,g)" = "!a!b!c!d!"

# test `A' option
test033 = -z "$(m test,atest,A)" -a "$(m test,test,A)" = "test"

# test `D' option
define subj034
line1
line2

endef
test034 = "$(m line\d$,$(subj034))" = "line2" -a -z "$(m line\d$,$(subj034),D)" -a \
	  "$(m test$,test,D)" = "test"

# test `S' option
# no way to check directly that it really works, so just enshure that
# it does not break anything
# TODO: add indirect (speed) test
test035 = "$(m test,test,S)" = test
test036 = "$(s a,x,a,S)" = "x"

### END OF TEST EXPRESSIONS ###

test%:
	@if [ $($@) ] ; then \
		echo '$@ PASSED'; \
	else \
		echo '$@ FAILED'; \
		echo '### expression:'; \
		echo '$(value $@)'; \
		echo '### expanded to:'; \
		echo '$($@)'; \
		return 1; \
	fi
