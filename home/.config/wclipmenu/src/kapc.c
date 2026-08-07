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

/* ---- small spawn helpers ------------------------------------------- */

/*
 * Run argv with no input; capture stdout into *out (malloc'd) with its
 * length in *outlen. Returns child exit status, or -1 on spawn failure.
 * The buffer is not NUL-terminated; use run_capture for string output.
 */
int run_capture_raw(const char *const argv[], char **out, size_t *outlen)
{
	int pipefd[2];
	pid_t pid;
	int status;

	*out = NULL;
	*outlen = 0;
	if (pipe(pipefd) == -1)
		return -1;
	pid = fork();
	if (pid == -1) {
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}
	if (pid == 0) {
		close(pipefd[0]);
		if (dup2(pipefd[1], STDOUT_FILENO) == -1)
			_exit(127);
		close(pipefd[1]);
		execv(argv[0], (char *const *)argv);
		_exit(127);
	}
	close(pipefd[1]);

	/* read all output */
	size_t cap = 4096, len = 0;
	char *buf = malloc(cap);
	if (!buf) {
		close(pipefd[0]);
		waitpid(pid, &status, 0);
		return -1;
	}
	for (;;) {
		if (len + 1 >= cap) {
			cap *= 2;
			char *nb = realloc(buf, cap);
			if (!nb) {
				free(buf);
				close(pipefd[0]);
				waitpid(pid, &status, 0);
				return -1;
			}
			buf = nb;
		}
		ssize_t n = read(pipefd[0], buf + len, cap - len - 1);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			free(buf);
			close(pipefd[0]);
			waitpid(pid, &status, 0);
			return -1;
		}
		if (n == 0)
			break;
		len += (size_t)n;
	}
	buf[len] = '\0';
	close(pipefd[0]);
	if (waitpid(pid, &status, 0) == -1)
		status = 0;
	*out = buf;
	*outlen = len;
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/*
 * String variant of run_capture_raw: capture stdout into *out
 * (NUL-terminated). Returns child exit status, or -1 on spawn failure.
 * *out is malloc'd, caller frees.
 */
int run_capture(const char *const argv[], char **out)
{
	size_t len;
	return run_capture_raw(argv, out, &len);
}

/*
 * Run argv in a child and return immediately (do NOT wait).
 * Used for `kapc copy`: kapc daemonizes a clipboard-owner grandchild
 * that must outlive us; waiting would hang until that owner dies.
 */
int spawn_detach(const char *const argv[])
{
	pid_t pid = fork();
	if (pid == -1)
		return -1;
	if (pid == 0) {
		int devnull = open("/dev/null", O_RDONLY);
		if (devnull >= 0) {
			dup2(devnull, STDIN_FILENO);
			close(devnull);
		}
		execv(argv[0], (char *const *)argv);
		_exit(127);
	}
	return 0;
}

/*
 * Run a | b in a child pipe and return immediately (do NOT wait).
 * Used for image copy-back: `kapc paste -i <id> | wl-copy --foreground`.
 * kapc paste writes the entry's raw data to stdout; wl-copy owns the
 * selection and serves it on both wlr-data-control and wl_data_device.
 * A kapd-owned selection (kapc copy) only serves wlr-data-control, which
 * truncates in wl_data_device readers like Waterfox. Both children get
 * stdin from /dev/null; neither may be waited on (wl-copy --foreground
 * lives until the selection is replaced, so waiting would hang).
 */
int pipeline_detach(const char *const a[], const char *const b[])
{
	int fds[2];
	if (pipe(fds) == -1)
		return -1;
	pid_t pa = fork();
	if (pa == -1) {
		close(fds[0]);
		close(fds[1]);
		return -1;
	}
	if (pa == 0) {
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);
		int devnull = open("/dev/null", O_RDONLY);
		if (devnull >= 0) {
			dup2(devnull, STDIN_FILENO);
			close(devnull);
		}
		execv(a[0], (char *const *)a);
		_exit(127);
	}
	pid_t pb = fork();
	if (pb == -1) {
		close(fds[0]);
		close(fds[1]);
		return -1;
	}
	if (pb == 0) {
		close(fds[1]);
		dup2(fds[0], STDIN_FILENO);
		close(fds[0]);
		execvp(b[0], (char *const *)b);
		_exit(127);
	}
	close(fds[0]);
	close(fds[1]);
	return 0;
}

/*
 * Run argv and wait for it, returning the child exit status (or -1 on
 * spawn failure). The child's stderr is discarded — used for magick, where
 * a corrupt source makes it complain about the image header on every run.
 */
int run_quiet(const char *const argv[])
{
	pid_t pid = fork();
	if (pid == -1)
		return -1;
	if (pid == 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
		execv(argv[0], (char *const *)argv);
		_exit(127);
	}
	int status;
	if (waitpid(pid, &status, 0) == -1)
		return -1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* ---- kaprica plumbing ---------------------------------------------- */

/* `kapc search -t <type> -L -l <n> [-D <db>]` -> raw stdout in *out. */
int kapc_search_type(const char *type, int limit, char **out)
{
	char lim[16];
	snprintf(lim, sizeof lim, "%d", limit);
	const char *db = getenv("WCLIPMENU_DB");
	if (db && *db) {
		const char *argv[] = { KAPC_PATH, "search", "-t", type,
				       "-L", "-l", lim, "-D", db, NULL };
		return run_capture(argv, out);
	}
	const char *argv[] = { KAPC_PATH, "search", "-t", type,
			       "-L", "-l", lim, NULL };
	return run_capture(argv, out);
}

int kapc_search(int limit, char **out)
{
	return kapc_search_type("text/plain", limit, out);
}

char *kapc_copy_cmd(long id)
{
	char *cmd = malloc(32);
	if (!cmd)
		return NULL;
	snprintf(cmd, 32, "%ld", id);
	return cmd;
}
