/* test_picker.c - picker suite: locks the picker contract and seeds the
 * future `kapc search | wmenu | kapc copy/paste` C picker skeleton.
 *
 * Pipeline being locked (future):
 *     kapc search '' -L -l 100 | wmenu -c -l 10 -p 'clip:'
 *     -> first tab field is the id
 *     -> image lines (snippet contains "image/png") route to a sixel
 *        preview (`kapc paste -i <id>` -> chafa --format sixel) 
 *     -> text lines route to `kapc copy -i <id>`
 *
 * The pure helpers at the bottom (kt_pick_*) are dependency-free on
 * purpose: they are the seed for the future real picker binary. This
 * suite only stubs the wmenu selection (first line) and never spawns
 * wmenu/chafa - it locks the parsing and routing.
 *
 * All state lives in the isolated scratch db from util.h; the real
 * history db is never touched. Seeding uses `kapc copy` (forking a
 * clipboard-owner child - kt_kapc's 3s idle timeout reaps it; we do
 * NOT assert on its exit status, only on search output).
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
 * kt_kapd_start() in util.c can never succeed: its readiness probe runs
 * `kapc -D <db> search -l 1`, but kapc requires options AFTER the
 * subcommand, so the probe is parse-rejected and readiness never
 * succeeds. This suite spawns kapd itself (fork+execv) with a
 * correct-order readiness probe, as test_daemon.c does. */

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

/* --- picker core helpers (pure, dependency-free; seed for the future
 * real picker binary) --- */

/* Parse one `kapc search -L` output line. The first tab-separated field
 * is the entry id: it must be 1+ ASCII digits immediately followed by a
 * tab, else -1. Sets *is_image = 1 iff the line contains "image/png".
 * Copies the id (NUL-terminated) into id_out; -1 if it does not fit.
 * Returns 0 on success. */
int kt_pick_parse_line(const char *line, char *id_out, size_t id_len,
                       int *is_image) {
    size_t d = 0;

    if (!line || !id_out || !is_image)
        return -1;
    *is_image = (strstr(line, "image/png") != NULL) ? 1 : 0;

    while (line[d] >= '0' && line[d] <= '9')
        d++;
    if (d == 0 || line[d] != '\t')
        return -1;
    if (id_len == 0 || d >= id_len)
        return -1; /* no room for the id + NUL */

    memcpy(id_out, line, d);
    id_out[d] = '\0';
    return 0;
}

/* 1 if the line's snippet marks it as an image ("image/png"), else 0. */
int kt_pick_is_image(const char *line) {
    return line && strstr(line, "image/png") ? 1 : 0;
}

/* Build the copy command for a chosen id: "kapc copy -i <id>".
 * Returns 0 on success, -1 if buf is too small (or n == 0).
 * noinline: keeps GCC from inlining the constant test buffers and
 * proving truncation (-Wformat-truncation); the truncation check below
 * is the runtime contract. */
#if defined(__GNUC__)
__attribute__((noinline))
#endif
int kt_pick_build_copy_cmd(const char *id, char *buf, size_t n) {
    int len;

    if (!id || !buf || n == 0)
        return -1;
    len = snprintf(buf, n, "kapc copy -i %s", id);
    if (len < 0 || (size_t)len >= n)
        return -1;
    return 0;
}

/* Build the preview command for a chosen id: "kapc paste -i <id>".
 * The real pipeline feeds this into `chafa --format sixel --size 120x45`;
 * the skeleton locks the id plumbing only. Returns 0/-1. */
#if defined(__GNUC__)
__attribute__((noinline))
#endif
int kt_pick_build_preview_cmd(const char *id, char *buf, size_t n) {
    int len;

    if (!id || !buf || n == 0)
        return -1;
    len = snprintf(buf, n, "kapc paste -i %s", id);
    if (len < 0 || (size_t)len >= n)
        return -1;
    return 0;
}

/* --- tests --- */

/* Text line: id is the first tab field; not an image. */
TEST(parse_text_line_extracts_id) {
    char id[64];
    int is_image = -1;

    KT_ASSERT_EQ_INT(kt_pick_parse_line("1234\thello world", id,
                                        sizeof id, &is_image), 0);
    KT_ASSERT_STR_EQ(id, "1234");
    KT_ASSERT_EQ_INT(is_image, 0);
    KT_ASSERT_EQ_INT(kt_pick_is_image("1234\thello world"), 0);
}

/* Image line: "image/png" in the snippet marks it; id still first tab
 * field. */
TEST(parse_image_line_extracts_id_and_flags) {
    char id[64];
    int is_image = -1;

    KT_ASSERT_EQ_INT(kt_pick_parse_line(
                         "4925\tWed Jun 17 10:33:50 2026 image/png", id,
                         sizeof id, &is_image), 0);
    KT_ASSERT_STR_EQ(id, "4925");
    KT_ASSERT_EQ_INT(is_image, 1);
    KT_ASSERT_EQ_INT(kt_pick_is_image("4925\tWed Jun 17 10:33:50 2026 image/png"),
                     1);
}

/* Malformed lines: no tab, empty, non-digit id. All reject with -1. */
TEST(parse_rejects_malformed_lines) {
    char id[64];
    int is_image = 0;

    KT_ASSERT_EQ_INT(kt_pick_parse_line("no-tab-here", id, sizeof id,
                                        &is_image), -1);
    KT_ASSERT_EQ_INT(kt_pick_parse_line("", id, sizeof id, &is_image), -1);
    KT_ASSERT_EQ_INT(kt_pick_parse_line("abc\ttext", id, sizeof id,
                                        &is_image), -1);
    /* id must be terminated by a tab: digits followed by junk is not an
     * id field */
    KT_ASSERT_EQ_INT(kt_pick_parse_line("1234abc\ttext", id, sizeof id,
                                        &is_image), -1);
    /* NULL guard */
    KT_ASSERT_EQ_INT(kt_pick_parse_line(NULL, id, sizeof id, &is_image), -1);
    /* is_image is still set even for rejected lines */
    KT_ASSERT_EQ_INT(is_image, 0);
}

/* Truncation: an id that does not fit the buffer rejects with -1. */
TEST(parse_rejects_truncated_id_buffer) {
    char tiny[3];
    int is_image = 0;

    KT_ASSERT_EQ_INT(kt_pick_parse_line("1234\thello", tiny, sizeof tiny,
                                        &is_image), -1);
    KT_ASSERT_EQ_INT(kt_pick_parse_line("1234\thello", tiny, 0, &is_image),
                     -1);
}

/* Routing: text lines -> copy command; image lines -> preview command.
 * Both carry the chosen id through. */
TEST(routing_builds_copy_and_preview_cmds) {
    char cmd[128];

    KT_ASSERT_EQ_INT(kt_pick_build_copy_cmd("1234", cmd, sizeof cmd), 0);
    KT_ASSERT_STR_EQ(cmd, "kapc copy -i 1234");

    KT_ASSERT_EQ_INT(kt_pick_build_preview_cmd("4925", cmd, sizeof cmd), 0);
    KT_ASSERT_STR_EQ(cmd, "kapc paste -i 4925");

    /* truncated command buffers reject: the buffer is big enough that
     * GCC cannot prove truncation at compile time (no -Wformat-truncation),
     * but too small for the fixed prefix + this long id, so snprintf
     * returns the needed length >= n and we reject at runtime */
    {
        char b[16];
        KT_ASSERT_EQ_INT(kt_pick_build_copy_cmd("12345678901234567890",
                                                b, sizeof b), -1);
        KT_ASSERT_EQ_INT(kt_pick_build_preview_cmd("12345678901234567890",
                                                   b, sizeof b), -1);
        KT_ASSERT_EQ_INT(kt_pick_build_copy_cmd("1234", b, 0), -1);
        KT_ASSERT_EQ_INT(kt_pick_build_preview_cmd("4925", b, 0), -1);
    }
}

/* wmenu-stub end-to-end: seed 2 text entries, run the real search
 * pipeline (`kapc search '' -L -l 100 -D <db>`), parse every line, then
 * simulate wmenu choosing the FIRST line and route it. */
TEST(wmenu_stub_end_to_end_routing) {
    static const char *const texts[] = {
        "kt-pk-foo-1\n",
        "kt-pk-bar-2\n",
    };
    char *out = NULL;
    size_t out_len = 0;
    size_t i, nlines = 0;
    const char *p;
    char *first_line = NULL;
    char first_id[64];
    int first_is_image = 0;
    char cmd[256];

    /* seed: 2 text entries; copies fork a clipboard-owner child that
     * kt_kapc reaps on its idle timeout, so no assert on copy status */
    for (i = 0; i < 2; i++) {
        char *argv[8];
        char *c = NULL;
        size_t c_len = 0;

        argv[0] = "kapc";
        argv[1] = "copy";
        argv[2] = "-D";
        argv[3] = g_db;
        argv[4] = NULL;
        kt_kapc(argv, texts[i], strlen(texts[i]), &c, &c_len);
        free(c);
        usleep(500000); /* let kapd record before the next op */
    }

    /* the pipeline search: kapc search '' -L -l 100 -D <db> */
    {
        char *argv[] = {"kapc", "search", "", "-L", "-l", "100",
                        "-D", g_db, NULL};
        int status = kt_kapc_simple(argv, &out, &out_len);
        KT_ASSERT_EQ_INT(status, 0);
        if (status != 0) {
            free(out);
            return;
        }
    }
    KT_ASSERT_EQ_INT((int)out_len > 0, 1);

    /* split into lines; skip empty (trailing) lines; every line must
     * parse with an all-digits id */
    p = out;
    while (*p != '\0') {
        const char *e = strchr(p, '\n');
        size_t len = e ? (size_t)(e - p) : strlen(p);
        char line[4096];
        char id[64];
        int is_image = 0;

        if (len > 0) {
            if (len >= sizeof line)
                len = sizeof line - 1;
            memcpy(line, p, len);
            line[len] = '\0';

            if (kt_pick_parse_line(line, id, sizeof id, &is_image) != 0) {
                KT_FAIL("search line failed to parse");
                KT_ASSERT_STR_EQ(line, "unparseable");
            } else {
                KT_ASSERT_EQ_INT(kt_pick_is_image(line), is_image);
            }
            if (nlines == 0) {
                first_line = line;
                first_id[0] = '\0';
                if (kt_pick_parse_line(first_line, first_id,
                                       sizeof first_id,
                                       &first_is_image) == 0) {
                    /* id must be all digits (parse already enforces) */
                    KT_ASSERT_EQ_INT(first_id[0] >= '0' &&
                                         first_id[0] <= '9', 1);
                }
            }
            nlines++;
        }
        if (!e)
            break;
        p = e + 1;
    }
    KT_ASSERT(nlines >= 2); /* both seeded entries are listed */

    /* wmenu stub: pick the FIRST line and route it */
    if (nlines == 0) {
        KT_FAIL("no lines to route");
        free(out);
        return;
    }
    if (first_id[0] == '\0') {
        KT_FAIL("first line did not parse");
        free(out);
        return;
    }
    if (first_is_image) {
        KT_ASSERT_EQ_INT(kt_pick_build_preview_cmd(first_id, cmd,
                                                   sizeof cmd), 0);
        KT_ASSERT_STR_CONTAINS(cmd, "kapc paste -i ");
    } else {
        KT_ASSERT_EQ_INT(kt_pick_build_copy_cmd(first_id, cmd,
                                                sizeof cmd), 0);
        KT_ASSERT_STR_CONTAINS(cmd, "kapc copy -i ");
    }
    KT_ASSERT_STR_CONTAINS(cmd, first_id);

    free(out);
}

KT_MAIN()
