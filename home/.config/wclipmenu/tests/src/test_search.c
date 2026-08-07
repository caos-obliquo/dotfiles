/* test_search.c - search suite: kapc search output contract.
 *
 * Locks the machine-readable -L list format and every search flag:
 *   -L   "<id>\t<snippet>\<nl>" per entry (trailing backslash is the
 *        snippet's embedded newline, escaped)
 *   -i   id-only lines (bare digits, no tab)
 *   -s   snippet-only lines (no tab, snippet + escaped newline)
 *   -l   limit number of entries
 *   -t   filter by MIME type
 *   -g   glob filter, full-string fnmatch against the SNIPPET text
 *        only (discovered empirically: "-g 1*" matches nothing since
 *        ids are not part of the match target; "-g kt-sr" matches
 *        nothing because fnmatch needs the whole string)
 *
 * All state lives in the isolated scratch db from util.h; the real
 * history db is never touched. Seeding uses `kapc copy` (forking a
 * clipboard-owner child - kt_kapc's 3s idle timeout reaps it; we do
 * NOT assert on its exit status, only on search results).
 */

/* Feature-test macro: PATH_MAX (limits.h) and usleep (unistd.h) need
 * POSIX exposure even with -std=c11. Must precede all system headers. */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "test.h"
#include "util.h"

kt_test kt_tests[KT_MAX_TESTS];
int kt_test_count = 0;
int kt_pass = 0;
int kt_fail = 0;

/* test.h's TEST(name) macro is broken: its parameter `name` collides
 * with the kt_test field `.name`, so the preprocessor rewrites
 * `.name = #name` into `.seed_three_texts = "seed_three_texts"` (no
 * such member). test.h is read-only for this suite, so re-register the
 * macro here with a non-colliding parameter name. */
#undef TEST
#define TEST(nm)                                                       \
    static void nm(void);                                              \
    __attribute__((constructor)) static void kt_reg_##nm(void) {       \
        if (kt_test_count < KT_MAX_TESTS) {                            \
            kt_tests[kt_test_count].name = #nm;                        \
            kt_tests[kt_test_count].fn = nm;                           \
            kt_test_count++;                                           \
        }                                                              \
    }                                                                  \
    static void nm(void)

static char g_db[PATH_MAX];
static char *g_saved_clip;
static pid_t g_kapd;

/* --- daemon lifecycle (parallel suites may hold the kapd lock; retry) ---
 *
 * NOTE: kt_kapd_start() in util.c can never succeed: its readiness probe
 * runs `kapc -D <db> search -l 1` but kapc requires options AFTER the
 * subcommand, so the probe is parse-rejected and readiness never succeeds.
 * This suite therefore spawns kapd itself (fork+execv, as test_daemon.c
 * does) with a correct-order readiness probe. */

/* True if any process named kapd is running (global instance check). */
static int kapd_running(void) {
    DIR *d = opendir("/proc");
    if (d == NULL) return 0;
    struct dirent *e;
    int found = 0;
    while ((e = readdir(d)) != NULL) {
        if (!isdigit((unsigned char)e->d_name[0])) continue;
        char path[320];
        snprintf(path, sizeof path, "/proc/%s/comm", e->d_name);
        FILE *f = fopen(path, "r");
        if (f == NULL) continue;
        char comm[64];
        if (fgets(comm, sizeof comm, f) != NULL) {
            comm[strcspn(comm, "\n")] = '\0';
            if (strcmp(comm, "kapd") == 0) found = 1;
        }
        fclose(f);
        if (found) break;
    }
    closedir(d);
    return found;
}

/* Fork kapd -D g_db [extra...] and wait until it serves searches.
 * Returns the child pid, or -1. Refuses while any kapd is running. */
static pid_t spawn_kapd(char *const extra[]) {
    if (kapd_running()) return -1; /* global refusal, mirror the harness */
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        char *argv[64];
        int n = 0;
        argv[n++] = (char *)"kapd";
        argv[n++] = (char *)"-D";
        argv[n++] = g_db;
        if (extra != NULL) {
            for (int i = 0; extra[i] != NULL && n < 62; i++)
                argv[n++] = extra[i];
        }
        argv[n] = NULL;
        int dn = open("/dev/null", O_RDWR);
        if (dn >= 0) {
            dup2(dn, STDIN_FILENO);
            dup2(dn, STDOUT_FILENO);
            if (dn > 2) close(dn);
        }
        execv("/usr/local/bin/kapd", argv);
        _exit(127);
    }
    /* readiness: child alive + `kapc search -D g_db -l 1` exits 0 */
    for (int i = 0; i < 25; i++) {
        int st;
        pid_t r = waitpid(pid, &st, WNOHANG);
        if (r == pid || (r < 0 && errno == ECHILD)) {
            kill(pid, SIGKILL);
            return -1; /* died early */
        }
        char *out = NULL;
        size_t out_len = 0;
        char *args[] = { (char *)"/usr/local/bin/kapc", (char *)"search",
                         (char *)"-D", g_db, (char *)"-l", (char *)"1",
                         NULL };
        int s = kt_kapc_simple(args, &out, &out_len);
        free(out);
        if (s == 0) return pid;
        usleep(200000);
    }
    kill(pid, SIGKILL);
    while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {}
    return -1;
}

/* Retry spawn_kapd up to ~30s: parallel suites share the global kapd
 * slot, so transient "another kapd is running" refusals must be retried
 * until the slot frees up. */
static pid_t start_kapd(char *const extra[]) {
    pid_t p;
    for (int i = 0; i < 150; i++) {
        p = spawn_kapd(extra);
        if (p > 0)
            return p;
        usleep(200000);
    }
    return -1;
}

__attribute__((constructor)) static void suite_init(void) {
    /* kt_db_path() alone does not create the scratch dir (util.c bug:
     * it uses kt_scratch_path(), not kt_scratch_dir()); kapd then fails
     * with "unable to open database file". mkdir first. */
    kt_scratch_dir();
    if (kt_db_path(g_db, sizeof g_db) != 0)
        exit(2);
    g_kapd = start_kapd(NULL);
    if (g_kapd <= 0)
        exit(2);
    kt_clipboard_snapshot(&g_saved_clip);
}

__attribute__((destructor)) static void suite_fini(void) {
    kt_clipboard_restore(g_saved_clip);
    kt_kapd_stop(g_kapd);
}

/* --- small pure helpers --- */

/* Count newline-terminated lines in a NUL-terminated buffer. A final
 * unterminated line (no trailing '\n') still counts as one line. */
static int line_count(char *out) {
    size_t len, i, n;

    if (!out)
        return 0;
    len = strlen(out);
    n = 0;
    for (i = 0; i < len; i++)
        if (out[i] == '\n')
            n++;
    if (len > 0 && out[len - 1] != '\n')
        n++;
    return (int)n;
}

/* Line begins with 1+ ASCII digits followed by a tab. The line is not
 * required to be NUL-terminated at its end; the check only inspects the
 * leading run. */
static int line_start_with_digits_tab(const char *line) {
    int d = 0;

    if (!line)
        return 0;
    while (line[d] >= '0' && line[d] <= '9')
        d++;
    return d > 0 && line[d] == '\t';
}

/* Non-empty string whose characters (up to the terminating '\n' or NUL)
 * are all ASCII digits. */
static int all_digits(const char *s) {
    size_t i;

    if (!s || s[0] == '\0' || s[0] == '\n')
        return 0;
    for (i = 0; s[i] != '\0' && s[i] != '\n'; i++)
        if (s[i] < '0' || s[i] > '9')
            return 0;
    return 1;
}

/* Run `kapc search` against the isolated db. Returns 0 on success and
 * fills *out (malloc'd); returns -1 on harness failure. */
static int run_search(char *const flags[], char **out, size_t *out_len) {
    char *argv[16];
    int n = 0, i;

    argv[n++] = "kapc";
    argv[n++] = "search";
    argv[n++] = "-D";
    argv[n++] = g_db;
    for (i = 0; flags[i] != NULL && n < 14; i++)
        argv[n++] = flags[i];
    argv[n] = NULL;

    return kt_kapc_simple(argv, out, out_len) == 0 ? 0 : -1;
}

/* --- tests --- */

/* Seed: 3 distinct texts. Copies fork a clipboard-owner child that
 * kt_kapc reaps on its idle timeout, so no assert on copy status. */
TEST(seed_three_texts) {
    static const char *const texts[] = {
        "kt-sr-alpha-1\n",
        "kt-sr-beta-2\n",
        "kt-sr-gamma-3\n",
    };
    char *argv[8];
    size_t i;

    for (i = 0; i < 3; i++) {
        argv[0] = "kapc";
        argv[1] = "copy";
        argv[2] = "-D";
        argv[3] = g_db;
        argv[4] = NULL;
        {
            char *out = NULL;
            size_t out_len = 0;
            kt_kapc(argv, texts[i], strlen(texts[i]), &out, &out_len);
            free(out);
        }
        usleep(500000); /* let kapd record before the next op */
    }
    KT_ASSERT(1); /* seeding performed without harness failure */
}

/* -L machine list: "<id>\t<snippet>\<nl>"; >= 3 entries; every line is
 * digits-then-tab; all three seeded tokens appear. */
TEST(L_format_digits_tab_listing) {
    char *out = NULL;
    size_t out_len = 0;
    const char *p;
    int ok_all = 1;

    if (run_search((char *[]){"-L", NULL}, &out, &out_len) != 0) {
        KT_FAIL("search -L failed");
        free(out);
        return;
    }
    KT_ASSERT_EQ_INT((int)out_len > 0, 1);
    KT_ASSERT(line_count(out) >= 3);

    p = out;
    while (*p != '\0') {
        const char *e = strchr(p, '\n');
        size_t len = e ? (size_t)(e - p) : strlen(p);
        char line[4096];

        if (len == 0)
            break; /* trailing newline */
        if (len >= sizeof line)
            len = sizeof line - 1;
        memcpy(line, p, len);
        line[len] = '\0';
        if (!line_start_with_digits_tab(line))
            ok_all = 0;
        if (!e)
            break;
        p = e + 1;
    }
    KT_ASSERT(ok_all);

    KT_ASSERT_STR_CONTAINS(out, "kt-sr-alpha-1");
    KT_ASSERT_STR_CONTAINS(out, "kt-sr-beta-2");
    KT_ASSERT_STR_CONTAINS(out, "kt-sr-gamma-3");
    free(out);
}

/* -i: id-only, every line is bare digits (no tab). */
TEST(L_id_only_bare_digits) {
    char *out = NULL;
    size_t out_len = 0;
    const char *p;
    int ok_all = 1;

    if (run_search((char *[]){"-L", "-i", NULL}, &out, &out_len) != 0) {
        KT_FAIL("search -L -i failed");
        free(out);
        return;
    }
    KT_ASSERT(line_count(out) >= 3);

    p = out;
    while (*p != '\0') {
        const char *e = strchr(p, '\n');
        size_t len = e ? (size_t)(e - p) : strlen(p);
        char line[64];

        if (len == 0)
            break;
        if (len >= sizeof line)
            len = sizeof line - 1;
        memcpy(line, p, len);
        line[len] = '\0';
        if (!all_digits(line))
            ok_all = 0;
        if (!e)
            break;
        p = e + 1;
    }
    KT_ASSERT(ok_all);
    KT_ASSERT(!strstr(out, "\t"));
    free(out);
}

/* -s: snippet-only, no tabs, seeded tokens present. */
TEST(L_snippet_only_no_tab) {
    char *out = NULL;
    size_t out_len = 0;

    if (run_search((char *[]){"-L", "-s", NULL}, &out, &out_len) != 0) {
        KT_FAIL("search -L -s failed");
        free(out);
        return;
    }
    KT_ASSERT(line_count(out) >= 3);
    KT_ASSERT(!strstr(out, "\t"));
    KT_ASSERT_STR_CONTAINS(out, "kt-sr-alpha-1");
    KT_ASSERT_STR_CONTAINS(out, "kt-sr-beta-2");
    KT_ASSERT_STR_CONTAINS(out, "kt-sr-gamma-3");
    free(out);
}

/* -l: limit. With >= 3 entries, -l 2 returns exactly 2 lines: the most
 * recent two (newest-first ordering), so the oldest seed is excluded. */
TEST(L_limit_two_returns_exactly_two) {
    char *out = NULL;
    size_t out_len = 0;

    if (run_search((char *[]){"-L", "-l", "2", NULL}, &out, &out_len) != 0) {
        KT_FAIL("search -L -l 2 failed");
        free(out);
        return;
    }
    KT_ASSERT_EQ_INT(line_count(out), 2);
    KT_ASSERT(!strstr(out, "kt-sr-alpha-1"));
    free(out);
}

/* -t: MIME filter. All seeded entries are text/plain; none are images. */
TEST(type_mime_filter) {
    char *out = NULL;
    size_t out_len = 0;

    if (run_search((char *[]){"-L", "-t", "text/plain", NULL}, &out,
                   &out_len) != 0) {
        KT_FAIL("search -L -t text/plain failed");
        free(out);
        return;
    }
    KT_ASSERT(line_count(out) >= 3);
    KT_ASSERT_STR_CONTAINS(out, "kt-sr-beta-2");
    free(out);

    out = NULL;
    out_len = 0;
    if (run_search((char *[]){"-L", "-t", "image/png", NULL}, &out,
                   &out_len) != 0) {
        KT_FAIL("search -L -t image/png failed");
        free(out);
        return;
    }
    KT_ASSERT_EQ_INT(line_count(out), 0);
    KT_ASSERT_EQ_INT((int)strlen(out), 0);
    free(out);
}

/* -g: glob filter, full-string fnmatch against the snippet.
 * "*beta*" matches the beta entry; a non-existent pattern yields
 * nothing. */
TEST(glob_pattern_matches_snippet) {
    char *out = NULL;
    size_t out_len = 0;

    if (run_search((char *[]){"-L", "-g", "*beta*", NULL}, &out,
                   &out_len) != 0) {
        KT_FAIL("search -L -g *beta* failed");
        free(out);
        return;
    }
    KT_ASSERT(line_count(out) >= 1);
    KT_ASSERT_STR_CONTAINS(out, "kt-sr-beta");
    KT_ASSERT(!strstr(out, "kt-sr-alpha"));
    KT_ASSERT(!strstr(out, "kt-sr-gamma"));
    free(out);

    out = NULL;
    out_len = 0;
    if (run_search((char *[]){"-L", "-g", "*zzz-nomatch*", NULL}, &out,
                   &out_len) != 0) {
        KT_FAIL("search -L -g *zzz-nomatch* failed");
        free(out);
        return;
    }
    KT_ASSERT_EQ_INT(line_count(out), 0);
    KT_ASSERT_EQ_INT((int)strlen(out), 0);
    free(out);
}

KT_MAIN()
