#ifndef KT_UTIL_H
#define KT_UTIL_H

/* util.h - process/daemon/clipboard helpers for the kaprica test harness.
 *
 * Isolation contract: every kapc/kapd invocation from the harness must go
 * through kt_kapc()/kt_kapd_start() so that -D <isolated db> is always in
 * play. The user's real history db (~/.local/share/kaprica/history.db) is
 * never touched; all state lives under /tmp/kaprica-tests-<pid>/.
 */

/* Feature-test macro: usleep/ssize_t/select need POSIX exposure even with
 * -std=c11 (strict ANSI hides them). Must precede all system headers. */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stddef.h>
#include <sys/types.h>

/* Lazily create /tmp/kaprica-tests-<pid>/ (only on first call) and return
 * the static path string. */
const char *kt_scratch_dir(void);

/* Fill buf with "<scratch>/history.db". Returns 0 on success, -1 if the
 * buffer is too small. */
int kt_db_path(char *buf, size_t n);

/* Start kapd on the isolated db: fork/exec /usr/local/bin/kapd with
 * -D <db_path> plus any extra args (NULL-terminated array). Refuses (prints
 * "another kapd is running; stop it before running tests" and returns -1)
 * if any kapd is already running. Polls readiness up to 5s by repeatedly
 * running `kapc -D <db> search -l 1` (via kt_kapc) until exit status 0.
 * Returns the child pid, or -1 on failure. */
pid_t kt_kapd_start(const char *db_path, char *const extra_args[]);

/* SIGTERM + waitpid, SIGKILL fallback after ~1s. No-op for pid <= 0. */
void kt_kapd_stop(pid_t pid);

/* Run a kapc command. argv[0] may be "kapc" (resolved via PATH) or an
 * absolute path like "/usr/local/bin/kapc" (execvp resolves it). If
 * stdin_data != NULL it is fed to the child's stdin (write-then-close).
 * Stdout is captured into a malloc'd NUL-terminated buffer (*out,
 * *out_len). Reading uses an idle select() timeout of ~3s: on timeout the
 * spawned child is SIGKILLed and its status returned - this is what keeps
 * `kapc copy` (which forks a clipboard-owner child inheriting stdout) from
 * hanging the harness forever. Returns the child's exit status (127 on
 * exec failure; 128+sig when killed by a signal; -1 on setup failure). */
int kt_kapc(char *const argv[], const void *stdin_data, size_t stdin_len,
            char **out, size_t *out_len);

/* Convenience wrapper for kt_kapc with no stdin data. */
int kt_kapc_simple(char *const argv[], char **out, size_t *out_len);

/* Hand-rolled SHA-256 (no external crypto libs): lowercase hex digest of
 * data, written into out[65]. */
void kt_sha256_hex(const unsigned char *data, size_t len, char out[65]);

/* Run `wl-paste --no-newline` and store the result in *text (malloc'd).
 * Returns 0 if the clipboard had text, 1 if it was empty, -1 on error. */
int kt_clipboard_snapshot(char **text);

/* Run `wl-copy` with the given text (restores the user's clipboard).
 * Returns 0 on success, -1 on error. Called at the end of a run. */
int kt_clipboard_restore(const char *text);

/* Registered via atexit from util.c: (1) kill recorded kapc child pids;
 * (2) scan /proc/<pid>/cmdline for any process whose cmdline contains the
 * scratch-dir path (unique per run - catches forked clipboard-owner
 * orphans) and SIGTERM then SIGKILL it; (3) rm -rf the scratch dir.
 * Never touches anything outside the scratch path. */
void kt_cleanup(void);

#endif /* KT_UTIL_H */
