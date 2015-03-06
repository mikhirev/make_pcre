/*
    make_pcre - PCRE plugin for GNU make.
    Copyright (C) 2014  Dmitry Mikhirev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <gnumake.h>
#include <pcre.h>

int plugin_is_GPL_compatible;

static const int MAX_CAP = 256;   /* maximum number of substrings to capture */
static const int MAX_CAP_LEN = 3; /* number of decimal digits in MAX_CAP */

static const int MAX_MSG_LEN = 1024; /* max length of error/warning/info message */

static char *str_extend(char *old, size_t size)
{
	char *new = realloc(old, size);
	if (new == NULL) { /* let make allocate memory or die */
		new = gmk_alloc(size);
		if (new == NULL) { /* should never happen */
			return NULL;
		}
		strncpy(new, old, size);
		gmk_free(old);
	}
	return new;
}

/* esc_str() - escape string before assigning it to make variable */
static char *esc_str(const char *str)
{
	const char *ps; /* pointer to char in unescaped string */
	char *esc;      /* escaped string */
	char *pe;       /* pointer to char in escaped string */

	esc = gmk_alloc(strlen(str) * 2 + 1);
	if (esc == NULL) { /* should never happen */
		return NULL;
	}

	for (ps = str, pe = esc; *ps != '\0'; ps++, pe++) {
		switch (*ps) {
		case '$': /* prepend with '$' */
			*pe = '$';
			pe++;
			break;
		}
		*pe = *ps;
	}
	*pe = '\0';

	return esc;
}

/* vmk_call() - call make function and pass formatted string as its argument */
static int vmk_call(const char *mkfunc, const char *fmt, va_list ap)
{
	char *mkarg, *emkarg, *mk; /* buffer strings */

	mkarg = gmk_alloc(MAX_MSG_LEN + 1);
	if (mkarg == NULL) { /* should never happen */
		return -1;
	}
	vsnprintf(mkarg, MAX_MSG_LEN, fmt, ap);
	mkarg[MAX_MSG_LEN + 1] = '\0';
	emkarg = esc_str(mkarg);
	gmk_free(mkarg);
	if (emkarg == NULL) { /* should never happen */
		return -1;
	}
	mk = gmk_alloc(strlen(emkarg) + strlen(mkfunc) + 30);
	if (mk == NULL) { /* should never happen */
		gmk_free(emkarg);
		return -1;
	}
	sprintf(mk, "__pcre_arg=%s\n$(%s $(__pcre_arg))", emkarg, mkfunc);
	gmk_free(emkarg);
	gmk_eval(mk, NULL);
	gmk_free(mk);
	return 0;
}

#if 0
/* mk_call() - call make function and pass formatted string as its argument */
static int mk_call(const char *mkfunc, const char *fmt, ...)
{
	va_list args; /* function arguments */
	int res;      /* value to return */

	va_start(args, fmt);
	res = vmk_call(mkfunc, fmt, args);
	va_end(args);
	return res;
}
#endif

/* mk_error() - pass formatted string to error make function */
static int mk_error(const char *fmt, ...)
{
	va_list args;          /* function arguments */
	int res;      /* value to return */

	va_start(args, fmt);
	res = vmk_call("error", fmt, args);
	va_end(args);
	return res;
}

/* mk_warning() - pass formatted string to warning make function */
static int mk_warning(const char *fmt, ...)
{
	va_list args;          /* function arguments */
	int res;      /* value to return */

	va_start(args, fmt);
	res = vmk_call("warning", fmt, args);
	va_end(args);
	return res;
}

#if 0
/* mk_info() - pass formatted string to info make function */
static int mk_info(const char *fmt, ...)
{
	va_list args;          /* function arguments */
	int res;      /* value to return */

	va_start(args, fmt);
	res = vmk_call("info", fmt, args);
	va_end(args);
	return res;
}
#endif

/* def_var() - define make variable */
static int def_var(const char *name, const char *value)
{
	char *escv;  /* escaped value */
	char *mkdef; /* variable definition for make */

	escv = esc_str(value);
	if (escv == NULL) {
		return -1;
	};
	mkdef = gmk_alloc(strlen(name) + strlen(escv) + 16);
	if (mkdef == NULL) { /* should never happen */
		gmk_free(escv);
		return -1;
	}
	sprintf(mkdef, "define %s\n%s\nendef\n", name, escv);
	gmk_eval(mkdef, NULL);

	gmk_free(escv);
	gmk_free(mkdef);
	return 0;
}

/* def_nvar() define numbered make variable */
static int def_nvar(int num, const char *value)
{
	char *escv;  /* escaped value */
	char *mkdef; /* variable definition for make */

	escv = esc_str(value);
	if (escv == NULL) {
		return -1;
	};
	mkdef = gmk_alloc(MAX_CAP_LEN + strlen(escv) + 16);
	if (mkdef == NULL) { /* should never happen */
		gmk_free(escv);
		return -1;
	}
	sprintf(mkdef, "define %d\n%s\nendef\n", num, escv);
	gmk_eval(mkdef, NULL);

	gmk_free(escv);
	gmk_free(mkdef);
	return 0;
}

/* parse_comp_opt - return regexp compilation option according to flag */
static int parse_comp_opt(const char flag, const char *func)
{
	int b; /* PCRE configuration option value */

	switch (flag) {
	case 'A': /* anchored regexp */
		return PCRE_ANCHORED;
	case 'D': /* $ matches at the end of string only */
		return PCRE_DOLLAR_ENDONLY;
	case 'i': /* ignore case */
		return PCRE_CASELESS;
	case 'm': /* multi-line */
		return PCRE_MULTILINE;
	case 's': /* single-line */
		return PCRE_DOTALL;
	case 'u': /* use Unicode properties */
		pcre_config(PCRE_CONFIG_UNICODE_PROPERTIES, &b);
		if (b) {
			return PCRE_UCP;
		} else {
			mk_warning("%s: PCRE library does not support "
					"Unicode properties, "
					"`%c' option is ignored",
					func, flag);
		}
		break;
	case 'U': /* ungreedy quantifiers */
		return PCRE_UNGREEDY;
	case 'x': /* extended regexp */
		return PCRE_EXTENDED;
	case 'X': /* PCRE extras */
		return PCRE_EXTRA;
	case '8': /* UTF-8 */
		pcre_config(PCRE_CONFIG_UTF8, &b);
		if (b) {
			return PCRE_UTF8;
		} else {
			mk_warning("%s: PCRE library does not support "
					"UTF-8, "
					"`%c' option is ignored",
					func, flag);
		}
		break;
	default: /* unknown option */
		mk_error("%s: unknown option `%c'", func, flag);
		break;
	}
	return 0;
}

/* set_vars() - set make variables to captured substrings */
static int set_vars(const char *subj, int *ovec, const int ncap)
{
	int i;                           /* loop iterator */
	const char *cap;                 /* captured substring */
	int caplen;                      /* length of captured substring */
	int retval;                      /* number of defined variables */
	char mk_undef[MAX_CAP_LEN + 11]; /* buffer for undefine command */

	for (i = 0; (i < ncap) && (i < MAX_CAP); i++) {
		caplen = pcre_get_substring(subj, ovec, ncap, i, &cap);
		if (caplen < 0) { /* unable to get substring */
			mk_error("cannot get substring: "
					"pcre_get_substring() returned %d",
					caplen);
			break;
		}
		def_nvar(i, cap);
		pcre_free_substring(cap);
	}
	retval = i;
	for (; i < MAX_CAP; i++) { /* udefine remaining make vars */
		sprintf(mk_undef, "undefine %d\n", i);
		gmk_eval(mk_undef, NULL);
	}
	return retval;
}

/* set_named_vars() - set make variables to substrings captured by name */
static int set_named_vars(const pcre *re, const char *subj, int *ovec, const int ncap)
{
	int ncount;      /* name count */
	int nentrysize;  /* size of name entry */
	char *ntable;    /* name table */
	int i;           /* loop iterator */
	char *n;         /* name pointer */
	const char *cap; /* captured substring */
	int caplen;      /* length of captured substring */

	pcre_fullinfo(re, NULL, PCRE_INFO_NAMECOUNT, &ncount);
	if (ncount <= 0) { /* no names defined, nothing to do */
		return ncount;
	}
	pcre_fullinfo(re, NULL, PCRE_INFO_NAMEENTRYSIZE, &nentrysize);
	pcre_fullinfo(re, NULL, PCRE_INFO_NAMETABLE, &ntable);
	for (i = 0; i < ncount; i++) {
		n = ntable + (i * nentrysize) + 2;
		caplen = pcre_get_named_substring(re, subj, ovec,
				ncap, n, &cap);
		if (caplen < 0) { /* unable to get substring */
			mk_error("cannot get substring: "
					"pcre_get_substring() returned %d",
					caplen);
			break;
		}
		def_var(n, cap);
		pcre_free_substring(cap);
	}
	return i;
}

/* match() - function to be attached to make pattern matching function */
static char *match(const char *name, int argc, char **argv)
{
	char *pat = NULL;      /* expanded pattern */
	char *p;               /* iteration pointer */
	int global = 0;        /* global search? */
	int study = 0;         /* study pattern? */
	int co = 0;            /* pattern compilation options */
	pcre *re = NULL;       /* compiled regexp */
	const char *err;       /* compilation error */
	int erroffset;         /* offset in pattern where error occured */
	pcre_extra *sd = NULL; /* pattern study data */
	char *str = NULL;      /* expanded subject string */
	int offset = 0;        /* subject string offset */
	int ncap = 0;          /* number of captured substrings */
	int ovec[MAX_CAP*3];   /* ovector */
	char *retstr = NULL;   /* string to be returned */
	int retlen = 0;        /* length of retstr */

	if (argc > 2) { /* options provided, parse them */
		for (p = argv[2]; *p != '\0'; p++) {
			switch (*p) {
			case 'E': /* expand pattern */
				pat = gmk_expand(argv[0]);
				break;
			case 'g': /* global search */
				global = 1;
				break;
			case 'S': /* study pattern */
				study = 1;
				break;
			default: /* not match-specific option */
				co |= parse_comp_opt(*p, name);
				break;
			}
		}
	}

	if (pat == NULL) { /* compile unexpanded pattern */
		re = pcre_compile(argv[0], co, &err, &erroffset, NULL);
	} else {           /* compile expanded pattern */
		re = pcre_compile(pat, co, &err, &erroffset, NULL);
		gmk_free(pat);
	}
	if (re == NULL) { /* compilation error */
		mk_error("%s: %d: %s", name, erroffset, err);
		goto end_match;
	}

	if (study) { /* study compiled pattern */
		sd = pcre_study(re, 0, &err);
		if (err) {
			mk_warning("%s: %s", name, err);
			sd = NULL;
		}
	}

	/* expand subject string */
	str = gmk_expand(argv[1]);

	do {
		/* execute regexp */
		ncap = pcre_exec(re, sd, str, strlen(str), offset, 0,
				ovec, MAX_CAP*3);
		if ((ncap < 0) && (ncap != PCRE_ERROR_NOMATCH)) { /* error occured */
			mk_error("%s: pattern matching error: %d\n", name, ncap);
		}

		if (ncap > 0) { /* copy or append matched string to retstr */
			int len = ovec[1] - ovec[0];
			int newlen = retlen + len;

			char *s = str_extend(retstr, newlen + 2);
			if (s == NULL) {
				goto end_match;
			}
			retstr = s;

			if (retlen > 0) { /* add whitespace */
				retstr[retlen] = ' ';
				retlen++;
				newlen++;
			}

			strncpy(retstr + retlen, str + ovec[0], len);
			retlen = newlen;
			retstr[retlen] = '\0';

			/* where to start next search */
			if (offset == ovec[1]) { //zero-length match
				if (offset < len) {
					// continue with one character shift
					offset++;
				} else {
					// stop global search
					global = 0;
				}
			} else {
				offset = ovec[1];
			}

			/* set named make vars to captured substrings */
			set_named_vars(re, str, ovec, ncap);
		}
	} while (global && (ncap != PCRE_ERROR_NOMATCH));

end_match:
	if (re != NULL) {
		pcre_free(re);
	}
	if (sd != NULL) {
	#if (PCRE_MAJOR < 8) || ((PCRE_MAJOR == 8) && (PCRE_MINOR < 20))
		pcre_free(sd);
	#else
		pcre_free_study(sd);
	#endif
	}

	/* set make vars to captured substrings */
	set_vars(str, ovec, ncap);

	if (str != NULL) {
		gmk_free(str);
	}
	return retstr;
}

static char *subst(const char *name, int argc, char **argv)
{
	char *pat = NULL;      /* expanded pattern */
	char *p;               /* iteration pointer */
	int global = 0;        /* global search? */
	int study = 0;         /* study pattern? */
	int co = 0;            /* pattern compilation options */
	pcre *re = NULL;       /* compiled regexp */
	const char *err;       /* compilation error */
	int erroffset;         /* offset in pattern where error occured */
	pcre_extra *sd = NULL; /* pattern study data */
	char *str = NULL;      /* expanded subject string */
	char *rep = NULL;      /* expanded replacement string */
	int subjlen;           /* length of subject string */
	int replen;            /* length of replacement string */
	int offset = 0;        /* subject string offset */
	int ncap = 0;          /* number of captured substrings */
	int ovec[MAX_CAP*3];   /* ovector */
	char *retstr = NULL;   /* string to be returned */
	int retlen = 0;        /* length of retstr */
	int newlen;            /* length of retstr after appending new part */
	char *s;               /* temporary string */

	if (argc > 3) { /* options provided, parse them */
		for (p = argv[3]; *p != '\0'; p++) {
			switch (*p) {
			case 'E': /* expand pattern */
				pat = gmk_expand(argv[0]);
				break;
			case 'g': /* global search */
				global = 1;
				break;
			case 'S': /* study pattern */
				study = 1;
				break;
			default: /* not subst-specific option */
				co |= parse_comp_opt(*p, name);
				break;
			}
		}
	}

	if (pat == NULL) { /* compile unexpanded pattern */
		re = pcre_compile(argv[0], co, &err, &erroffset, NULL);
	} else {           /* compile expanded pattern */
		re = pcre_compile(pat, co, &err, &erroffset, NULL);
		gmk_free(pat);
	}
	if (re == NULL) { /* compilation error */
		mk_error("%s: %d: %s", name, erroffset, err);
		goto end_subst;
	}

	if (study) { /* study compiled pattern */
		sd = pcre_study(re, 0, &err);
		if (err) {
			mk_warning("%s: %s", name, err);
			sd = NULL;
		}
	}

	/* expand subject string */
	str = gmk_expand(argv[2]);
	subjlen = strlen(str);

	do {
		/* execute regexp */
		ncap = pcre_exec(re, sd, str, subjlen, offset, 0,
				ovec, MAX_CAP*3);
		if ((ncap < 0) && (ncap != PCRE_ERROR_NOMATCH)) { /* error occured */
			mk_error("%s: pattern matching error: %d\n", name, ncap);
			goto end_subst;
		}

		if (ncap > 0) { /* match found */
			/* set make vars to captured substrings */
			set_vars(str, ovec, ncap);
			/* set named make vars to captured substrings */
			set_named_vars(re, str, ovec, ncap);

			/* expand replacement string */
			rep = gmk_expand(argv[1]);
			replen = strlen(rep);

			newlen = retlen + (ovec[0] - offset) + replen;
			s = str_extend(retstr, newlen + 1);
			if (s == NULL) {
				goto end_subst;
			}
			retstr = s;

			strncpy(retstr + retlen, str + offset, ovec[0] - offset);
			strncpy(retstr + retlen + ovec[0] - offset, rep, replen + 1);
			retlen += ovec[0] - offset + replen;

			/* free expanded replacement string */
			gmk_free(rep);
			rep = NULL;

			/* where to start next search */
			if (offset == ovec[1]) { //zero-length match
				if (offset < subjlen) {
					// continue with one character shift
					s = str_extend(retstr, retlen + 1);
					if (s == NULL) {
						goto end_subst;
					}
					retstr = s;
					strncpy(retstr + retlen, str + offset, 1);
					retlen++;
					offset++;
				} else {
					// stop global search
					global = 0;
				}
			} else {
				offset = ovec[1];
			}
		}
	} while (global && (ncap != PCRE_ERROR_NOMATCH));

	newlen = retlen + subjlen - offset;
	s = str_extend(retstr, newlen + 1);
	if (s == NULL) {
		goto end_subst;
	}
	retstr = s;
	strncpy(retstr + retlen, str + offset, subjlen - offset + 1);

end_subst:
	if (re != NULL) {
		pcre_free(re);
	}
	if (sd != NULL) {
	#if (PCRE_MAJOR < 8) || ((PCRE_MAJOR == 8) && (PCRE_MINOR < 20))
		pcre_free(sd);
	#else
		pcre_free_study(sd);
	#endif
	}

	if (str != NULL) {
		gmk_free(str);
	}
	if (rep != NULL) {
		gmk_free(rep);
	}

	return retstr;
}

int pcre_gmk_setup()
{
	/* add function for pattern matching */
	gmk_add_function("pcre_find", (gmk_func_ptr)match, 2, 3,
			GMK_FUNC_NOEXPAND);
	gmk_add_function("m", (gmk_func_ptr)match, 2, 3, GMK_FUNC_NOEXPAND);

	/* add function for pattern substitution */
	gmk_add_function("pcre_subst", (gmk_func_ptr)subst, 3, 4,
			GMK_FUNC_NOEXPAND);
	gmk_add_function("s", (gmk_func_ptr)subst, 3, 4, GMK_FUNC_NOEXPAND);

	return 1;
}
