#define _DEFAULT_SOURCE /* keeps _POSIX APIs visible before any header */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "wclipmenu.h"

struct pick {
	char thumb[64];
	size_t es_idx; /* index into es[] the thumbnail was built from */
};

/*
 * Locate the magick binary via the shell and stash its path in *path.
 * Returns 0 on success, -1 if not found (caller errors out: thumbnails
 * are the whole point of the image picker).
 */
static int resolve_magick(char **path)
{
	char *out = NULL;
	const char *argv[] = { "/bin/sh", "-c", "command -v magick 2>/dev/null",
			       NULL };
	if (run_capture(argv, &out) != 0 || !out) {
		free(out);
		return -1;
	}
	size_t l = strlen(out);
	while (l > 0 && (out[l - 1] == '\n' || out[l - 1] == '\r'))
		out[--l] = '\0';
	if (l == 0) {
		free(out);
		return -1;
	}
	*path = out;
	return 0;
}

/*
 * Render a 160x160 PNG thumbnail for entry id into *thumb. Two-step file
 * pipeline keeps the child plumbing trivial: `kapc paste` bytes land in an
 * mkstemp raw file, then `magick <raw> -resize 160x160 <thumb>` resizes it.
 * Returns 0 with thumb filled in, -1 on any failure (raw file and any
 * partial output are unlinked; caller falls back to a plain text row).
 */
static int render_thumb(const char *magick, long id, char *thumb,
			size_t thumbsz)
{
	char ids[32];
	snprintf(ids, sizeof ids, "%ld", id);
	const char *db = getenv("WCLIPMENU_DB");

	/* cache hit: a thumb from a prior launch is reusable - skip the
	 * expensive kapc paste + magick pipeline entirely (image picker
	 * must open near-instantly on repeat use) */
	snprintf(thumb, thumbsz, "/tmp/wclipmenu-thumb-%ld.png", id);
	if (access(thumb, R_OK) == 0)
		return 0;

	char rawtmpl[] = "/tmp/wclipmenu-raw-XXXXXX";
	int fd = mkstemp(rawtmpl);
	if (fd == -1)
		return -1;

	char *paste = NULL;
	size_t plen = 0;
	int st;
	if (db && *db) {
		const char *argv[] = { KAPC_PATH, "paste", "-i", ids, "-D", db,
				       NULL };
		st = run_capture_raw(argv, &paste, &plen);
	} else {
		const char *argv[] = { KAPC_PATH, "paste", "-i", ids, NULL };
		st = run_capture_raw(argv, &paste, &plen);
	}
	if (st != 0 || !paste) {
		close(fd);
		unlink(rawtmpl);
		free(paste);
		return -1;
	}
	size_t off = 0;
	while (off < plen) {
		ssize_t w = write(fd, paste + off, plen - off);
		if (w < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		off += (size_t)w;
	}
	free(paste);
	if (off != plen) {
		close(fd);
		unlink(rawtmpl);
		return -1;
	}
	close(fd);

	/* .png suffix keeps magick's read policy deterministic (mkstemp's
	 * XXXXXX leaves no extension to sniff) */
	char rawin[sizeof rawtmpl + 4];
	snprintf(rawin, sizeof rawin, "%s.png", rawtmpl);
	if (rename(rawtmpl, rawin) == -1) {
		unlink(rawtmpl);
		return -1;
	}

	/* wmenu's cairo_image_surface_create_from_png needs the .png suffix */
	snprintf(thumb, thumbsz, "/tmp/wclipmenu-thumb-%ld.png", id);
	unlink(thumb); /* drop any stale partial from a prior run */
	const char *argv[] = { magick, rawin, "-resize", "160x160", thumb, NULL };
	st = run_quiet(argv);
	unlink(rawin);
	if (st != 0 || access(thumb, F_OK) == -1) {
		unlink(thumb); /* partial output must not survive */
		return -1;
	}
	return 0;
}

int cmd_image(int limit)
{
	char *magick = NULL;
	if (resolve_magick(&magick) == -1) {
		fprintf(stderr, "wclipmenu: magick not found "
				"(required for image picker)\n");
		return 1;
	}

	char *raw = NULL;
	int st = kapc_search_type("image/png", limit, &raw);
	if (st != 0 || !raw) {
		fprintf(stderr, "wclipmenu: kapc search failed (status %d)\n", st);
		free(raw);
		free(magick);
		return 1;
	}
	struct entry *es = calloc((size_t)limit, sizeof *es);
	if (!es) {
		free(raw);
		free(magick);
		return 1;
	}
	size_t n = parse_entries(raw, es, (size_t)limit);
	if (n == 0) {
		fprintf(stderr, "wclipmenu: empty clipboard history\n");
		free(es);
		free(raw);
		free(magick);
		return 1;
	}

	/* render a thumbnail per entry; failed renders fall back to a plain
	 * text row so the snippet stays visible and pickable */
	struct pick *picks = calloc(n, sizeof *picks);
	if (!picks) {
		free(es);
		free(raw);
		free(magick);
		return 1;
	}
	size_t i;
	/* parallel thumbnail render: kapc paste + magick dominate first-open
	 * time; fork one child per entry so N thumbs build concurrently and
	 * the picker opens near-instantly even on a cold cache */
	size_t nlive = 0;
	for (i = 0; i < n; i++) {
		pid_t pid = fork();
		if (pid == 0) {
			char tbuf[64];
			int rc = render_thumb(magick, es[i].id, tbuf,
					      sizeof tbuf);
			_exit(rc == 0 ? 0 : 1);
		} else if (pid > 0) {
			nlive++;
		}
	}
	while (nlive > 0) {
		waitpid(-1, NULL, 0);
		nlive--;
	}
	for (i = 0; i < n; i++) {
		picks[i].es_idx = i;
		snprintf(picks[i].thumb, sizeof picks[i].thumb,
			 "/tmp/wclipmenu-thumb-%ld.png", es[i].id);
		if (access(picks[i].thumb, R_OK) != 0)
			picks[i].thumb[0] = '\0'; /* no thumbnail, plain row */
	}

	/* wmenu input: [img:<thumbpath>]<snippet> for thumbnailed rows, plain
	 * <snippet> for the rest */
	size_t cap = 4096, len = 0;
	char *lines = malloc(cap);
	if (!lines) {
		free(picks);
		free(es);
		free(raw);
		free(magick);
		return 1;
	}
	for (i = 0; i < n; i++) {
		const char *snippet = es[picks[i].es_idx].snippet;
		size_t pre = picks[i].thumb[0] ? strlen("[img:") +
			      strlen(picks[i].thumb) + 1 : 0;
		size_t need = pre + strlen(snippet) + 2;
		if (len + need >= cap) {
			while (cap < len + need + 1)
				cap *= 2;
			char *nb = realloc(lines, cap);
			if (!nb) {
				free(lines);
				free(picks);
				free(es);
				free(raw);
				free(magick);
				return 1;
			}
			lines = nb;
		}
		int m = picks[i].thumb[0]
			? snprintf(lines + len, cap - len, "[img:%s]%s\n",
				   picks[i].thumb, snippet)
			: snprintf(lines + len, cap - len, "%s\n", snippet);
		if (m < 0 || (size_t)m >= cap - len) {
			free(lines);
			free(picks);
			free(es);
			free(raw);
			free(magick);
			return 1;
		}
		len += (size_t)m;
	}

	char *sel = NULL;
	if (run_wmenu_lines(lines, WMENU_IMAGE_LINES, &sel) == -1) {
		free(lines);
		free(picks);
		free(es);
		free(raw);
		free(magick);
		return 0; /* user cancelled — not an error */
	}
	free(lines);

	/* thumbnailed rows: "[img:<thumbpath>]<snippet>", id in the thumb
	 * filename (/tmp/wclipmenu-thumb-<id>.png). Plain fallback rows are
	 * the bare snippet, matched by text like cmd_pick. */
	long id = -1;
	if (strncmp(sel, "[img:", 5) == 0) {
		const char *close = strchr(sel + 5, ']');
		const char *marker = close ? strstr(sel + 5, "wclipmenu-thumb-")
					   : NULL;
		if (marker) {
			char *end = NULL;
			long v = strtol(marker + strlen("wclipmenu-thumb-"),
					&end, 10);
			if (end != marker + strlen("wclipmenu-thumb-"))
				id = v;
		}
	} else {
		for (i = 0; i < n; i++) {
			if (strcmp(es[i].snippet, sel) == 0) {
				id = es[i].id;
				break;
			}
		}
	}
	free(sel);
	free(picks);
	free(es);
	free(raw);
	free(magick);
	if (id < 0) {
		fprintf(stderr, "wclipmenu: selection not found in history\n");
		return 1;
	}
	return cmd_copy_image(id);
}
