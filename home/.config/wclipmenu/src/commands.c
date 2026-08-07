#define _DEFAULT_SOURCE /* keeps _POSIX APIs visible before any header */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wclipmenu.h"

/* ---- subcommands ---------------------------------------------------- */

int cmd_list(int limit)
{
	char *raw = NULL;
	int st = kapc_search(limit, &raw);
	if (st != 0 || !raw) {
		fprintf(stderr, "wclipmenu: kapc search failed (status %d)\n", st);
		free(raw);
		return 1;
	}
	struct entry *es = calloc((size_t)limit, sizeof *es);
	if (!es) {
		free(raw);
		return 1;
	}
	size_t n = parse_entries(raw, es, (size_t)limit);
	size_t i;
	for (i = 0; i < n; i++)
		printf("%s\n", es[i].snippet);
	free(es);
	free(raw);
	return 0;
}

int cmd_copy(long id)
{
	char *ids = kapc_copy_cmd(id);
	if (!ids)
		return 1;
	const char *db = getenv("WCLIPMENU_DB");
	int st;
	if (db && *db) {
		const char *argv[] = { KAPC_PATH, "copy", "-i", ids, "-D", db,
				       NULL };
		st = spawn_detach(argv);
	} else {
		const char *argv[] = { KAPC_PATH, "copy", "-i", ids, NULL };
		st = spawn_detach(argv);
	}
	if (st == -1) {
		fprintf(stderr, "wclipmenu: failed to spawn kapc copy\n");
		free(ids);
		return 1;
	}
	free(ids);
	return 0;
}

/*
 * Image copy-back must route through wl-copy, not `kapc copy`: a
 * kapd-owned selection is served on wlr-data-control only, and clients
 * that read wl_data_device (Waterfox) pull a truncated PNG with a white
 * bottom. `kapc paste -i <id> | wl-copy --foreground -t image/png` makes
 * wl-copy the owner, which serves both protocols (same mechanism as the
 * dwl-screenshot fix).
 */
int cmd_copy_image(long id)
{
	char *ids = kapc_copy_cmd(id);
	if (!ids)
		return 1;
	const char *db = getenv("WCLIPMENU_DB");
	const char *left[8] = { KAPC_PATH, "paste", "-i", ids, NULL };
	int n = 4;
	if (db && *db) {
		left[n++] = "-D";
		left[n++] = db;
	}
	left[n] = NULL;
	const char *right[] = { "wl-copy", "--foreground", "-t", "image/png",
				NULL };
	int st = pipeline_detach(left, right);
	if (st == -1) {
		fprintf(stderr, "wclipmenu: failed to spawn paste|wl-copy\n");
		free(ids);
		return 1;
	}
	free(ids);
	return 0;
}

int cmd_pick(int limit)
{
	char *raw = NULL;
	int st = kapc_search(limit, &raw);
	if (st != 0 || !raw) {
		fprintf(stderr, "wclipmenu: kapc search failed (status %d)\n", st);
		free(raw);
		return 1;
	}
	struct entry *es = calloc((size_t)limit, sizeof *es);
	if (!es) {
		free(raw);
		return 1;
	}
	size_t n = parse_entries(raw, es, (size_t)limit);
	if (n == 0) {
		fprintf(stderr, "wclipmenu: empty clipboard history\n");
		free(es);
		free(raw);
		return 1;
	}

	char *sel = NULL;
	if (run_wmenu(es, n, &sel) == -1) {
		free(es);
		free(raw);
		return 0; /* user cancelled — not an error */
	}

	/* map selection back to its id */
	size_t i;
	long id = -1;
	for (i = 0; i < n; i++) {
		if (strcmp(es[i].snippet, sel) == 0) {
			id = es[i].id;
			break;
		}
	}
	free(sel);
	if (id < 0) {
		fprintf(stderr, "wclipmenu: selection not found in history\n");
		free(es);
		free(raw);
		return 1;
	}
	free(es);
	free(raw);
	return cmd_copy(id);
}
