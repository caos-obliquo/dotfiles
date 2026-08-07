/* test_reverse.c - reverse-search copy suite: kapc copy -r <snippet>.
 *
 * Observed kapc 'Kaprica Pre-release' /usr/local/bin behavior (probed
 * against an isolated scratch db before writing these tests):
 *
 *   - `copy -r <snippet>` resolves the snippet against the history db and
 *     copies the entry to the clipboard. It REQUIRES -D <db>: without -D
 *     it looks in the default db path and prints "Snippet not found"
 *     (rc 0) even though the running kapd serves another db.
 *   - Matching is EXACT, against the snippet as rendered by `search -L`
 *     (multiline entries render embedded newlines as a literal backslash:
 *     "line1\line2"). A prefix/substring does NOT match. Tabs are kept
 *     raw in both the snippet and the -r query.
 *   - On success -r copies the entry and REPLACES it in the db (entry id
 *     bumps, still one line), while plain `copy` dedupes identical
 *     content - duplicate entries never accumulate, which the duplicates
 *     test locks.
 *   - `copy -r` itself is a copy command: it forks a clipboard-owner
 *     child, so the harness's ~3s idle select() timeout kills the spawned
 *     process. Its exit status is therefore NOT asserted (0 or 137).
 *     Verification is via `paste -n` / `search -L` instead.
 *   - Image detection lives in -t/-x, not -r: a text-only db never shows
 *     an "image/png" line in `search -L` and `search -L -t image/png`
 *     yields nothing.
 *
 * Copy-family call budget: 8 (t1: 2, t2: 2, t3: 2, t4: 2; t5 reuses
 * t1's seed entry and only searches).
 */

/* util.h FIRST: it defines _DEFAULT_SOURCE, which must precede every
 * system header (test.h pulls in stdio/string/stdlib and would otherwise
 * process glibc features.h without it, hiding PATH_MAX/usleep). */
#include "util.h"
#include "test.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

kt_test kt_tests[KT_MAX_TESTS];
int kt_test_count = 0;
int kt_pass = 0;
int kt_fail = 0;

static char g_db[PATH_MAX];
static char *g_saved_clip;
static pid_t g_kapd;

/* Run kapc with NULL-terminated argv; returns exit status and (malloc'd)
 * stdout via *out. */
static int kt_capture(char *const argv[], char **out) {
    size_t out_len = 0;
    return kt_kapc_simple(argv, out, &out_len);
}

/* Count '\n'-delimited lines of hay that contain needle. */
static int kt_count_lines_containing(const char *hay, const char *needle) {
    if (hay == NULL || needle == NULL) return 0;
    int n = 0;
    const char *p = hay;
    while (*p != '\0') {
        const char *eol = strchr(p, '\n');
        size_t len = (eol != NULL) ? (size_t)(eol - p) : strlen(p);
        char *line = (char *)malloc(len + 1);
        if (line == NULL) return n;
        memcpy(line, p, len);
        line[len] = '\0';
        if (strstr(line, needle) != NULL) n++;
        free(line);
        if (eol == NULL) break;
        p = eol + 1;
    }
    return n;
}


static void kt_rv_roundtrip(void);
static void kt_rv_duplicates(void);
static void kt_rv_tab_snippet(void);
static void kt_rv_multiline(void);
static void kt_rv_image_detection(void);

/* test.h's TEST(name) macro is broken (`.name` is macro-substituted, so
 * TEST(foo) expands to `.foo`); register manually instead. */
__attribute__((constructor)) static void kt_reg_tests(void) {
    kt_tests[kt_test_count++] = (kt_test){ "kt_rv_roundtrip", kt_rv_roundtrip };
    kt_tests[kt_test_count++] = (kt_test){ "kt_rv_duplicates", kt_rv_duplicates };
    kt_tests[kt_test_count++] = (kt_test){ "kt_rv_tab_snippet", kt_rv_tab_snippet };
    kt_tests[kt_test_count++] = (kt_test){ "kt_rv_multiline", kt_rv_multiline };
    kt_tests[kt_test_count++] = (kt_test){ "kt_rv_image_detection", kt_rv_image_detection };
}

/* Count '\n'-delimited lines of hay that contain both a and b. */
static int kt_count_lines_containing_both(const char *hay, const char *a,
                                          const char *b) {
    if (hay == NULL || a == NULL || b == NULL) return 0;
    int n = 0;
    const char *p = hay;
    while (*p != '\0') {
        const char *eol = strchr(p, '\n');
        size_t len = (eol != NULL) ? (size_t)(eol - p) : strlen(p);
        char *line = (char *)malloc(len + 1);
        if (line == NULL) return n;
        memcpy(line, p, len);
        line[len] = '\0';
        if (strstr(line, a) != NULL && strstr(line, b) != NULL) n++;
        free(line);
        if (eol == NULL) break;
        p = eol + 1;
    }
    return n;
}

/* Start kapd on the isolated scratch db. kt_kapd_start() cannot be used:
 * its readiness poll passes "-D <db>" BEFORE the subcommand, which kapc
 * rejects with a usage error, so it always fails. Fork/exec kapd directly
 * and poll readiness with the accepted arg order (`search -l 1 -D db`;
 * rc 0 on an empty db too). Retries up to ~10s for kapd-singleton
 * contention with parallel agents. */
static pid_t start_kapd(char *const extra[]) {
    for (int i = 0; i < 50; i++) {
        pid_t p = fork();
        if (p < 0) return -1;
        if (p == 0) {
            char *argv[64];
            int n = 0;
            argv[n++] = (char *)"kapd";
            argv[n++] = (char *)"-D";
            argv[n++] = g_db;
            if (extra != NULL) {
                for (int j = 0; extra[j] != NULL && n < 62; j++)
                    argv[n++] = extra[j];
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
        int ready = 0;
        time_t t0 = time(NULL);
        while (time(NULL) - t0 < 5) {
            int st;
            pid_t r = waitpid(p, &st, WNOHANG);
            if (r == p) break; /* kapd died early */
            char *out = NULL;
            size_t out_len = 0;
            char *args[] = { (char *)"kapc", (char *)"search", (char *)"-l",
                             (char *)"1", (char *)"-D", g_db, NULL };
            int s = kt_kapc_simple(args, &out, &out_len);
            free(out);
            if (s == 0) {
                ready = 1;
                break;
            }
            usleep(200000);
        }
        if (ready) return p;
        kill(p, SIGKILL);
        while (waitpid(p, NULL, 0) < 0 && errno == EINTR) {}
        usleep(200000);
    }
    return -1;
}

__attribute__((constructor)) static void suite_init(void) {
    if (kt_db_path(g_db, sizeof g_db) != 0) exit(2);
    (void)kt_scratch_dir(); /* ensure the dir exists for kapd's db file */
    g_kapd = start_kapd(NULL);
    if (g_kapd <= 0) exit(2);
    if (kt_clipboard_snapshot(&g_saved_clip) < 0) exit(2);
}

__attribute__((destructor)) static void suite_fini(void) {
    kt_clipboard_restore(g_saved_clip);
    kt_kapd_stop(g_kapd);
}

/* Roundtrip: copy -> copy -r -> paste returns the entry. Also carries the
 * image-mime contract (see kt_rv_image_detection) - its seed entry keeps
 * the -r test independent of extra copy calls. */
static void kt_rv_roundtrip(void) {
    const char *tok = "kt-rv-unique-1";
    char *out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)tok,
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)"-r",
                      (char *)tok, (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"paste", (char *)"-n",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT_STR_EQ(out, tok);
    free(out);
}

/* Duplicates: identical entries never accumulate. A second `copy` of the
 * same token is deduplicated - the -L listing still shows exactly one
 * line. (No paste check here: the Wayland clipboard is process-global and
 * parallel agents' copies race it; paste roundtrips live in the other
 * tests.) */
static void kt_rv_duplicates(void) {
    const char *tok = "kt-rv-dupe-2";
    char *out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)tok,
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"search", (char *)"-L",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT(kt_count_lines_containing(out, tok) == 1);
    free(out);
    out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)tok,
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"search", (char *)"-L",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT(kt_count_lines_containing(out, tok) == 1);
    free(out);
}

/* Tab + special chars: the tab survives the snippet (raw) and -r resolves
 * the exact string, tab intact, paste returns the full original. */
static void kt_rv_tab_snippet(void) {
    const char *full = "kt-rv-tab-3\tpart2";
    char *out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)full,
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"search", (char *)"-L",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT_STR_CONTAINS(out, full); /* raw tab in the snippet */
    free(out);
    out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)"-r",
                      (char *)full, (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"paste", (char *)"-n",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT_STR_EQ(out, full);
    free(out);
}

/* Embedded newline: search -L renders the newline as a literal backslash
 * ("line1\line2"); -r matches that RENDERED form exactly (a bare prefix
 * like "kt-rv-ml-4" is NOT found). Paste returns the full original text
 * with the real newline. */
static void kt_rv_multiline(void) {
    const char *full = "kt-rv-ml-4 line1\nline2";
    const char *rendered = "kt-rv-ml-4 line1\\line2";
    char *out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)full,
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"search", (char *)"-L",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT_STR_CONTAINS(out, rendered);
    KT_ASSERT(strstr(out, full) == NULL); /* raw newline never in -L lines */
    free(out);
    out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"copy", (char *)"-r",
                      (char *)rendered, (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
        free(out);
        out = NULL;
    }
    {
        char *a[] = { (char *)"kapc", (char *)"paste", (char *)"-n",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT_STR_EQ(out, full);
    free(out);
}

/* Image-mime line: text entries never masquerade as images - no -L line
 * carries both a test token and the "image/png" snippet form, and
 * `search -L -t image/png` never returns a test token. Image detection
 * relies on -t/-x, not on -r. (Parallel agents may pollute this scratch
 * db with real image entries via the shared kapd singleton, so the
 * asserts are contamination-robust: they only constrain test-token
 * lines.) Reuses the roundtrip seed entry (copy budget). */
static void kt_rv_image_detection(void) {
    char *out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"search", (char *)"-L",
                      (char *)"-D", g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT_STR_CONTAINS(out, "kt-rv-unique-1"); /* seed present */
    KT_ASSERT(kt_count_lines_containing_both(out, "image/png", "kt-rv-") == 0);
    free(out);
    out = NULL;
    {
        char *a[] = { (char *)"kapc", (char *)"search", (char *)"-L",
                      (char *)"-t", (char *)"image/png", (char *)"-D",
                      g_db, NULL };
        kt_capture(a, &out);
    }
    KT_ASSERT(kt_count_lines_containing(out, "kt-rv-") == 0);
    free(out);
}

KT_MAIN()
