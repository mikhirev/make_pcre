#include <stdio.h>
#include <string.h>

#include <gnumake.h>
#include <pcre.h>

int plugin_is_GPL_compatible;

const int MAX_CAP = 256;

char *match(const char *name, int argc, char **argv)
{
	pcre *re;
	const char *err;
	int erroffset;
	int ncap = 0;
	int ovec[MAX_CAP*3];
	char *retstr = NULL;

	re = pcre_compile(argv[0], 0, &err, &erroffset, NULL);
	if (err != NULL) {
		fprintf(stderr, "%s: %d: %s\n", name, erroffset, err);
		goto end_match;
	}

	ncap = pcre_exec(re, NULL, argv[1], strlen(argv[1]), 0, 0, ovec, MAX_CAP*3);

end_match:
	if (ncap) {
		int len = ovec[1] - ovec[0];
		retstr = gmk_alloc(len + 1);
		strncpy(retstr, argv[1] + ovec[0], len);
		retstr[len + 1] = '\0';
	}
	return retstr;
}

int pcre_gmk_setup()
{
	gmk_add_function("m", (gmk_func_ptr)match, 2, 2, GMK_FUNC_NOEXPAND);
	return 1;
}
