#define _DEFAULT_SOURCE /* keeps _POSIX APIs visible before any header */

#include <stdlib.h>
#include <string.h>

#include "wclipmenu.h"

/*
 * Parse `<id>\t<snippet>` lines from `kapc search -L` output into es[].
 * Snippets point into raw (mutated in place). Tabs inside a snippet are
 * replaced with spaces for single-line wmenu display. Lines that do not
 * start with digits immediately followed by a tab are skipped — this keeps
 * multi-line snippets from misaligning the parse (their continuation lines
 * carry no leading id).
 */
size_t parse_entries(char *raw, struct entry *es, size_t max)
{
	size_t n = 0;
	char *line = raw;

	while (line && n < max) {
		char *nl = strchr(line, '\n');
		if (nl)
			*nl = '\0';

		char *tab = strchr(line, '\t');
		if (!tab) {
			if (!nl)
				break;
			line = nl + 1;
			continue;
		}
		char *end = NULL;
		long id = strtol(line, &end, 10);
		if (end != tab || end == line) {
			if (!nl)
				break;
			line = nl + 1;
			continue;
		}

		char *s = tab + 1;
		size_t ll = strlen(s);
		while (ll > 0 && s[ll - 1] == '\r')
			s[--ll] = '\0';
		for (char *p = s; *p; p++) {
			if (*p == '\t')
				*p = ' ';
		}

		es[n].id = id;
		es[n].snippet = s;
		n++;
		if (!nl)
			break;
		line = nl + 1;
	}
	return n;
}
