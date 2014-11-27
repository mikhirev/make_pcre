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

const int MAX_CAP = 256;   /* maximum number of substrings to capture */
const int MAX_CAP_LEN = 3; /* number of decimal digits in MAX_CAP */

const int MAX_MSG_LEN = 1024; /* max length of error/warning/info message */

/* esc_str() - escape string before assigning it to make variable */
char *esc_str(const char *str)
{
	const char *ps; /* pointer to char in unescaped string */
	char *esc;      /* escaped string */
	char *pe;       /* pointer to char in escaped string */

	esc = gmk_alloc(strlen(str) * 2 + 1);
	if (esc == NULL) {
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

int mk_error(const char *fmt, ...)
{
	va_list args;
	char *msg, *emsg, *mk;

	va_start(args, fmt);

	msg = gmk_alloc(MAX_MSG_LEN);
	vsnprintf(msg, MAX_MSG_LEN, fmt, args);
	emsg = esc_str(msg);
	gmk_free(msg);
	mk = gmk_alloc(strlen(emsg) + 10);
	sprintf(mk, "$(error %s)", emsg);
	gmk_free(emsg);
	gmk_eval(mk, NULL);
	gmk_free(mk);
	return 0;
}

int mk_warning(const char *fmt, ...)
{
	va_list args;
	char *msg, *emsg, *mk;

	va_start(args, fmt);

	msg = gmk_alloc(MAX_MSG_LEN);
	vsnprintf(msg, MAX_MSG_LEN, fmt, args);
	emsg = esc_str(msg);
	gmk_free(msg);
	mk = gmk_alloc(strlen(emsg) + 12);
	sprintf(mk, "$(warning %s)", emsg);
	gmk_free(emsg);
	gmk_eval(mk, NULL);
	gmk_free(mk);
	return 0;
}

int mk_info(const char *fmt, ...)
{
	va_list args;
	char *msg, *emsg, *mk;

	va_start(args, fmt);

	msg = gmk_alloc(MAX_MSG_LEN);
	vsnprintf(msg, MAX_MSG_LEN, fmt, args);
	emsg = esc_str(msg);
	gmk_free(msg);
	mk = gmk_alloc(strlen(emsg) + 9);
	sprintf(mk, "$(info %s)", emsg);
	gmk_free(emsg);
	gmk_eval(mk, NULL);
	gmk_free(mk);
	return 0;
}

/* def_var() - define make variable */
int def_var(const char *name, const char *value)
{
	char *escv;  /* escaped value */
	char *mkdef; /* variable definition for make */

	escv = esc_str(value);
	if (escv == NULL) {
		return -1;
	};
	mkdef = gmk_alloc(strlen(name) + strlen(escv) + 16);
	if (mkdef == NULL) {
		return -1;
	}
	sprintf(mkdef, "define %s\n%s\nendef\n", name, escv);
	gmk_eval(mkdef, NULL);

	gmk_free(escv);
	gmk_free(mkdef);
	return 0;
}

/* def_nvar() define numbered make variable */
int def_nvar(int num, const char *value)
{
	char *escv;  /* escaped value */
	char *mkdef; /* variable definition for make */

	escv = esc_str(value);
	if (escv == NULL) {
		return -1;
	};
	mkdef = gmk_alloc(MAX_CAP_LEN + strlen(escv) + 16);
	if (mkdef == NULL) {
		return -1;
	}
	sprintf(mkdef, "define %d\n%s\nendef\n", num, escv);
	gmk_eval(mkdef, NULL);

	gmk_free(escv);
	gmk_free(mkdef);
	return 0;
}

/* set_comp_opt - set regexp option */
int parse_comp_opt(const char flag, const char *func)
{
	int b; /* PCRE configuration option value */

	switch (flag) {
	case 'i': /* ignore case */
		return PCRE_CASELESS;  break;
	case 'm': /* multi-line */
		return PCRE_MULTILINE; break;
	case 's': /* single-line */
		return PCRE_DOTALL;    break;
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
		return PCRE_UNGREEDY;  break;
	case 'x': /* extended regexp */
		return PCRE_EXTENDED;  break;
	case 'X': /* PCRE extras */
		return PCRE_EXTRA;     break;
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
int set_vars(const char *subj, int *ovec, const int ncap)
{
	int i;           /* loop iterator */
	const char *cap; /* captured substring */
	int caplen;      /* length of captured substring */
	int retval;      /* number of defined variables to return */

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
		char mk_set[MAX_CAP_LEN + 11];
		sprintf(mk_set, "undefine %d\n", i);
		gmk_eval(mk_set, NULL);
	}
	return retval;
}

/* set_named_vars() - set make variables to substrings captured by name */
int set_named_vars(const pcre *re, const char *subj, int *ovec, const int ncap)
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
char *match(const char *name, int argc, char **argv)
{
	char *pat = NULL;    /* expanded pattern */
	char *p;             /* iteration pointer */
	int co = 0;          /* pattern compilation options */
	pcre *re;            /* compiled regexp */
	const char *err;     /* compilation error */
	int erroffset;       /* offset in pattern where error occured */
	char *str = NULL;    /* expanded subject string */
	int ncap = 0;        /* number of captured substrings */
	int ovec[MAX_CAP*3]; /* ovector */
	char *retstr = NULL; /* string to be returned */

	if (argc > 2) { /* options provided, parse them */
		for (p = argv[2]; *p != '\0'; p++) {
			switch (*p) {
			case 'E': /* expand pattern */
				pat = gmk_expand(argv[0]);
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

	/* expand subject string and execute regexp */
	str = gmk_expand(argv[1]);
	ncap = pcre_exec(re, NULL, str, strlen(str), 0, 0, ovec, MAX_CAP*3);
	if ((ncap < 0) && (ncap != PCRE_ERROR_NOMATCH)) { /* error occured */
		mk_error("%s: pattern matching error: %d\n", name, ncap);
	}

	if (ncap > 0) {
		/* set retstr to matched substring */
		int len = ovec[1] - ovec[0];
		retstr = gmk_alloc(len + 1);
		strncpy(retstr, str + ovec[0], len);
		retstr[len] = '\0';

		/* set named make vars to captured substrings */
		set_named_vars(re, str, ovec, ncap);
	}

	pcre_free(re);

end_match:
	/* set make vars to captured substrings */
	set_vars(str, ovec, ncap);

	if (str != NULL) {
		gmk_free(str);
	}
	return retstr;
}

int pcre_gmk_setup()
{
	/* add function for pattern matching */
	gmk_add_function("pcre_find", (gmk_func_ptr)match, 2, 3,
			GMK_FUNC_NOEXPAND);
	gmk_add_function("m", (gmk_func_ptr)match, 2, 3, GMK_FUNC_NOEXPAND);
	return 1;
}
