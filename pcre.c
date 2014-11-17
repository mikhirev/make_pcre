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

#include <stdio.h>
#include <string.h>

#include <gnumake.h>
#include <pcre.h>

int plugin_is_GPL_compatible;

const int MAX_CAP = 256;   /* maximum number of substrings to capture */
const int MAX_CAP_LEN = 3; /* number of decimal digits in MAX_CAP */

/* match() - function to be attached to make pattern matching function */
char *match(const char *name, int argc, char **argv)
{
	char *pat = NULL;    /* expanded pattern */
	char *p;             /* iteration pointer */
	int co = 0;          /* pattern compilation options */
	int b;               /* PCRE configuration option value */
	pcre *re;            /* compiled regexp */
	const char *err;     /* compilation error */
	int erroffset;       /* offset of pattern character where error occured */
	char *str = NULL;    /* expanded subject string */
	int ncap = 0;        /* number of captured substrings */
	int ovec[MAX_CAP*3]; /* ovector */
	char *retstr = NULL; /* string to be returned */
	int i;               /* iterator */

	if (argc > 2) { /* options provided, parse them */
		for (p = argv[2]; *p != '\0'; p++) {
			switch (*p) {
			case 'E': /* expand pattern */
				pat = gmk_expand(argv[0]);
				break;
			case 'i': /* ignore case */
				co |= PCRE_CASELESS;
				break;
			case 'm': /* multi-line */
				co |= PCRE_MULTILINE;
				break;
			case 's': /* single-line */
				co |= PCRE_DOTALL;
				break;
			case 'u': /* use Unicode properties */
				pcre_config(PCRE_CONFIG_UNICODE_PROPERTIES,
						&b);
				if (b) {
					co |= PCRE_UCP;
				} else {
					fprintf(stderr, "%s: PCRE library "
							"does not support "
							"Unicode properties, "
							"`%c' option is "
							"unavailable\n",
							name, *p);
				}
				break;
			case 'U': /* ungreedy quantifiers */
				co |= PCRE_UNGREEDY;
				break;
			case 'x': /* extended regexp */
				co |= PCRE_EXTENDED;
				break;
			case 'X': /* PCRE extras */
				co |= PCRE_EXTRA;
				break;
			case '8': /* UTF-8 */
				pcre_config(PCRE_CONFIG_UTF8, &b);
				if (b) {
					co |= PCRE_UTF8;
				} else {
					fprintf(stderr, "%s: PCRE library "
							"does not support "
							"UTF-8, "
							"`%c' option is "
							"unavailable\n",
							name, *p);
				}
				break;
			default: /* unknown option */
				fprintf(stderr, "%s: unknown option `%c'\n",
						name, *p);
				break;
			}
		}
	}

	if (pat == NULL) { /* compile unexpanded pattern */
		re = pcre_compile(argv[0], 0, &err, &erroffset, NULL);
	} else {           /* compile expanded pattern */
		re = pcre_compile(pat, 0, &err, &erroffset, NULL);
		gmk_free(pat);
	}
	if (re == NULL) { /* compilation error */
		fprintf(stderr, "%s: %d: %s\n", name, erroffset, err);
		goto end_match;
	}

	/* expand subject string and execute regexp */
	str = gmk_expand(argv[1]);
	ncap = pcre_exec(re, NULL, str, strlen(str), 0, 0, ovec, MAX_CAP*3);
	pcre_free(re);
	if ((ncap < 0) && (ncap != PCRE_ERROR_NOMATCH)) { /* error occured */
		fprintf(stderr, "%s: pattern matching error: %d\n", name, ncap);
	}

end_match:
	if (ncap > 0) { /* set retstr to matched substring */
		int len = ovec[1] - ovec[0];
		retstr = gmk_alloc(len + 1);
		strncpy(retstr, str + ovec[0], len);
		retstr[len] = '\0';
	}
	for (i = 0; (i < ncap) && (i < MAX_CAP); i++) { /* set make vars to captured substrings */
		char c = *(str + ovec[i*2 + 1]);
		*(str + ovec[i*2 + 1]) = '\0';
		int len = ovec[i*2 + 1] - ovec[i];
		char mk_set[len + MAX_CAP_LEN + 16];
		sprintf(mk_set, "define %d\n%s\nendef\n", i, str + ovec[i*2]);
		*(str + ovec[i*2 + 1]) = c;
		gmk_eval(mk_set, NULL);
	}
	for (; i < MAX_CAP; i++) { /* udefine remaining make vars */
		char mk_set[MAX_CAP_LEN + 11];
		sprintf(mk_set, "undefine %d\n", i);
		gmk_eval(mk_set, NULL);
	}
	if (str != NULL) {
		gmk_free(str);
	}
	return retstr;
}

int pcre_gmk_setup()
{
	/* add function for pattern matching */
	gmk_add_function("pcre_find", (gmk_func_ptr)match, 2, 3, GMK_FUNC_NOEXPAND);
	gmk_add_function("m", (gmk_func_ptr)match, 2, 3, GMK_FUNC_NOEXPAND);
	return 1;
}
