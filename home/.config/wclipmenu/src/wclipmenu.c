/*
 * wclipmenu — wmenu-based clipboard picker over kaprica (Wayland).
 *
 * Runs on demand from a keybind and exits after one selection:
 *   wclipmenu            -> pick from kaprica text history via wmenu, copy choice
 *   wclipmenu list       -> print snippet lines to stdout (debug/CLI path)
 *   wclipmenu copy <id>  -> kapc copy -i <id> (debug/CLI path)
 *   wclipmenu image      -> pick from kaprica image/png history via wmenu
 *                           (PNG thumbnails), copy choice
 *
 * Zero resident footprint: no daemon, no polling, no config files.
 * Depends only on libc + external wmenu + kaprica CLI (kapd/kapc).
 *
 * Env overrides (for testing):
 *   WCLIPMENU_WMENU  path to wmenu binary (default: wmenu from PATH)
 *   WCLIPMENU_LIMIT  max entries from kaprica (default: 100)
 *   WCLIPMENU_DB     db file path passed to kapc as -D (default: kaprica's)
 */
#define _DEFAULT_SOURCE /* usleep-free; keeps PATH_MAX, etc. before any header */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wclipmenu.h"

static int parse_limit(const char *s)
{
	char *end = NULL;
	long v = strtol(s, &end, 10);
	if (end == s || v < 1 || v > 10000)
		return -1;
	return (int)v;
}

int main(int argc, char **argv)
{
	int limit = DEFAULT_LIMIT;
	const char *envlim = getenv("WCLIPMENU_LIMIT");
	if (envlim && *envlim) {
		int v = parse_limit(envlim);
		if (v < 0) {
			fprintf(stderr, "wclipmenu: bad WCLIPMENU_LIMIT\n");
			return 1;
		}
		limit = v;
	}

	if (argc >= 2 && strcmp(argv[1], "list") == 0) {
		if (argc >= 4 && strcmp(argv[2], "-n") == 0) {
			int v = parse_limit(argv[3]);
			if (v < 0) {
				fprintf(stderr, "usage: wclipmenu list [-n count]\n");
				return 1;
			}
			limit = v;
		}
		return cmd_list(limit);
	}
	if (argc >= 2 && strcmp(argv[1], "copy") == 0) {
		if (argc < 3) {
			fprintf(stderr, "usage: wclipmenu copy <id>\n");
			return 1;
		}
		char *end = NULL;
		long id = strtol(argv[2], &end, 10);
		if (end == argv[2] || *end != '\0') {
			fprintf(stderr, "wclipmenu: bad id '%s'\n", argv[2]);
			return 1;
		}
		return cmd_copy(id);
	}
	if (argc >= 2 && strcmp(argv[1], "image") == 0) {
		if (argc >= 4 && strcmp(argv[2], "-n") == 0) {
			int v = parse_limit(argv[3]);
			if (v < 0) {
				fprintf(stderr, "usage: wclipmenu image [-n count]\n");
				return 1;
			}
			limit = v;
		}
		return cmd_image(limit);
	}
	if (argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
		printf("usage: wclipmenu [list [-n count] | copy <id> | image]\n");
		return 0;
	}
	if (argc > 1) {
		fprintf(stderr, "usage: wclipmenu [list [-n count] | copy <id> | image]\n");
		return 1;
	}
	return cmd_pick(limit);
}
