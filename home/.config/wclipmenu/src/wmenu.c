#define _DEFAULT_SOURCE /* keeps _POSIX APIs visible before any header */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "wclipmenu.h"

/* ---- wmenu --------------------------------------------------------- */

/*
 * Feed raw preformatted lines (NUL-terminated) to wmenu, read the chosen
 * line back. Returns 0 and writes selection (NUL-terminated, malloc'd) on
 * success, -1 if wmenu failed or the user cancelled.
 */
int run_wmenu_lines(const char *input, int wmenu_lines, char **sel)
{
	const char *wmenu = getenv("WCLIPMENU_WMENU");
	if (!wmenu || !*wmenu)
		wmenu = "wmenu";

	int inpipe[2], outpipe[2];
	pid_t pid;
	int status;

	if (pipe(inpipe) == -1 || pipe(outpipe) == -1) {
		if (inpipe[0] >= 0) {
			close(inpipe[0]);
			close(inpipe[1]);
		}
		return -1;
	}
	pid = fork();
	if (pid == -1) {
		close(inpipe[0]);
		close(inpipe[1]);
		close(outpipe[0]);
		close(outpipe[1]);
		return -1;
	}
	if (pid == 0) {
		dup2(inpipe[0], STDIN_FILENO);
		dup2(outpipe[1], STDOUT_FILENO);
		close(inpipe[0]);
		close(inpipe[1]);
		close(outpipe[0]);
		close(outpipe[1]);
		char lines[16];
		snprintf(lines, sizeof lines, "%d", wmenu_lines);
/* -N normal background alpha from RRGGBBAA (cc = minimal transparency) */
		const char *argv[] = { wmenu, "-l", lines, "-i", "-c", "-p",
				       WMENU_PROMPT, "-N", "#222222cc",
				       "-S", "#3b3b3bf2", "-s", "#ffffff", NULL };
		execv(wmenu, (char *const *)argv);
		/* fall back to PATH lookup for a bare name */
		if (strchr(wmenu, '/') == NULL) {
			execvp(wmenu, (char *const *)argv);
		}
		_exit(127);
	}
	close(inpipe[0]);
	close(outpipe[1]);

	/* feed input lines */
	size_t len = strlen(input);
	size_t off = 0;
	while (off < len) {
		ssize_t w = write(inpipe[1], input + off, len - off);
		if (w < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		off += (size_t)w;
	}
	close(inpipe[1]);

	/* read selection */
	size_t cap = 4096, len2 = 0;
	char *buf = malloc(cap);
	if (!buf) {
		close(outpipe[0]);
		waitpid(pid, &status, 0);
		return -1;
	}
	for (;;) {
		if (len2 + 1 >= cap) {
			cap *= 2;
			char *nb = realloc(buf, cap);
			if (!nb) {
				free(buf);
				close(outpipe[0]);
				waitpid(pid, &status, 0);
				return -1;
			}
			buf = nb;
		}
		ssize_t r = read(outpipe[0], buf + len2, cap - len2 - 1);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			free(buf);
			close(outpipe[0]);
			waitpid(pid, &status, 0);
			return -1;
		}
		if (r == 0)
			break;
		len2 += (size_t)r;
	}
	buf[len2] = '\0';
	close(outpipe[0]);
	if (waitpid(pid, &status, 0) == -1)
		status = 0;

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		free(buf);
		return -1; /* cancelled or crashed */
	}
	/* trim trailing newline */
	size_t bl = strlen(buf);
	while (bl > 0 && (buf[bl - 1] == '\n' || buf[bl - 1] == '\r'))
		buf[--bl] = '\0';
	if (bl == 0) {
		free(buf);
		return -1;
	}
	*sel = buf;
	return 0;
}

/*
 * Feed snippets to wmenu, read the chosen line back.
 * Returns 0 and writes selection (NUL-terminated, malloc'd) on success,
 * -1 if wmenu failed or the user cancelled.
 */
int run_wmenu(const struct entry *es, size_t n, char **sel)
{
	size_t total = 0;
	size_t i;
	for (i = 0; i < n; i++)
		total += strlen(es[i].snippet) + 1;
	char *input = malloc(total + 1);
	if (!input)
		return -1;
	size_t off = 0;
	for (i = 0; i < n; i++) {
		size_t sl = strlen(es[i].snippet);
		memcpy(input + off, es[i].snippet, sl);
		off += sl;
		input[off++] = '\n';
	}
	input[off] = '\0';
	int st = run_wmenu_lines(input, WMENU_LINES, sel);
	free(input);
	return st;
}
