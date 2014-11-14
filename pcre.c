#include <stdio.h>
#include <string.h>

#include <gnumake.h>
#include <pcre.h>

int plugin_is_GPL_compatible;

const int MAX_CAP = 256;

static char *mk_resetvars;

char *match(const char *name, int argc, char **argv)
{
	char *pat = NULL;
	char *p;
	int co = 0;
	pcre *re;
	const char *err;
	int erroffset;
	char *str;
	int ncap = 0;
	int ovec[MAX_CAP*3];
	char *retstr = NULL;
	int i;

	if (argc > 2) {
		for (p = argv[2]; *p != '\0'; p++) {
			switch (*p) {
			case 'e':
				pat = gmk_expand(argv[0]);
				break;
			case 'm':
				co |= PCRE_MULTILINE;
				break;
			case 's':
				co |= PCRE_DOTALL;
				break;
			case 'u':
				co |= PCRE_UCP;
				break;
			case 'U':
				co |= PCRE_UNGREEDY;
				break;
			case 'x':
				co |= PCRE_EXTENDED;
				break;
			case 'X':
				co |= PCRE_EXTRA;
				break;
			case '8':
				co |= PCRE_UTF8;
				break;
			default:
				fprintf(stderr, "%s: unknown option `%c'\n",
						name, *p);
				break;
			}
		}
	}

	gmk_eval(mk_resetvars, NULL);

	if (pat == NULL) {
		re = pcre_compile(argv[0], 0, &err, &erroffset, NULL);
	} else {
		re = pcre_compile(pat, 0, &err, &erroffset, NULL);
	}
	if (re == NULL) {
		fprintf(stderr, "%s: %d: %s\n", name, erroffset, err);
		goto end_match;
	}

	str = gmk_expand(argv[1]);

	ncap = pcre_exec(re, NULL, str, strlen(str), 0, 0, ovec, MAX_CAP*3);
	pcre_free(re);

end_match:
	if (ncap) {
		int len = ovec[1] - ovec[0];
		retstr = gmk_alloc(len + 1);
		strncpy(retstr, str + ovec[0], len);
		retstr[len] = '\0';
	}
	for (i = 0; i < ncap; i++) {
		char c = *(str + ovec[i*2 + 1]);
		*(str + ovec[i*2 + 1]) = '\0';
		int len = ovec[i*2 + 1] - ovec[i];
		char mk_set[len + 18];
		sprintf(mk_set, "define %d\n%s\nendef\n", i, str + ovec[i*2]);
		*(str + ovec[i*2 + 1]) = c;
		gmk_eval(mk_set, NULL);
	}
	return retstr;
}

int pcre_gmk_setup()
{
	int i;

	mk_resetvars = malloc(MAX_CAP * 13);
	*mk_resetvars = '\0';
	for (i = 0; i < MAX_CAP; i++) {
		char line[14];
		sprintf(line, "undefine %d\n", i);
		strncat(mk_resetvars, line, 13);
	}

	gmk_add_function("pcre_find", (gmk_func_ptr)match, 2, 3, GMK_FUNC_NOEXPAND);
	gmk_add_function("m", (gmk_func_ptr)match, 2, 3, GMK_FUNC_NOEXPAND);
	return 1;
}
