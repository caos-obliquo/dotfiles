/* test_image.c - image suite: kapc copy/paste of image mime types, sixel
 * output (`kapc search -x` DCS sequence starting with 0x1b 'P'), and the
 * wl-paste image/png roundtrip through the live Wayland clipboard.
 *
 * All kapc/kapd calls go through the harness (util.h) so the isolated db
 * under /tmp/kaprica-tests-<pid>/ is always used; the real history db is
 * never touched. The PNG fixture is generated in-file with hand-rolled
 * CRC32/Adler32 and DEFLATE stored blocks (no zlib dependency).
 */

/* util.h must come first: it defines _DEFAULT_SOURCE before any system
 * header, which glibc's features.h locks in -std=c11 strict mode
 * (PATH_MAX, usleep, ssize_t...). */
#include "util.h"
#include "test.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

/* test.h's TEST(name) is broken: the parameter `name` collides with the
 * kt_test struct member `.name`, so `kt_tests[i].name = #name` expands to
 * `kt_tests[i].<testname> = "..."`. Shadow it here with the corrected
 * expansion (parameter renamed); test.h stays untouched. */
#undef TEST
#define TEST(tn)                                                       \
    static void tn(void);                                              \
    __attribute__((constructor)) static void kt_reg_##tn(void) {       \
        if (kt_test_count < KT_MAX_TESTS) {                            \
            kt_tests[kt_test_count].name = #tn;                        \
            kt_tests[kt_test_count].fn = tn;                           \
            kt_test_count++;                                           \
        }                                                              \
    }                                                                  \
    static void tn(void)

kt_test kt_tests[KT_MAX_TESTS];
int kt_test_count = 0;
int kt_pass = 0;
int kt_fail = 0;

/* ------------------------------------------------------------------ */
/* fixture + suite lifecycle                                           */

static char g_db[PATH_MAX];
static char *g_saved_clip;
static pid_t g_kapd;

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
 * kt_kapd_start() in util.c can never succeed (its readiness probe runs
 * `kapc -D <db> search -l 1`, but kapc requires options AFTER the
 * subcommand), so spawn kapd ourselves with a correct-order probe.
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
        if (p > 0) return p;
        usleep(200000);
    }
    return -1;
}

/* kapc talks over IPC to whichever kapd owns the shared socket. A sibling
 * suite may start its own kapd right after ours and steal the socket
 * (TOCTOU on the harness's "any kapd running" check), so kapc calls would
 * hit the wrong daemon. Prove ownership: copy a unique probe via IPC, then
 * scan our own db file for it. Returns 1 if our kapd served the copy. */
static int kapd_serves_own_db(void) {
    static char probe[64];
    char *out = NULL;
    size_t olen = 0;
    unsigned char buf[65536];
    size_t i, n;
    int found = 0;

    snprintf(probe, sizeof probe, "kt-img-probe-%ld", (long)getpid());
    {
        char *argv[] = {"kapc", "copy", "-t", "text/plain", probe, "-D",
                        g_db, NULL};
        int st = kt_kapc(argv, NULL, 0, &out, &olen);
        free(out);
        (void)st;
    }
    usleep(500000);
    {
        FILE *f = fopen(g_db, "rb");
        if (f != NULL) {
            n = fread(buf, 1, sizeof buf, f);
            fclose(f);
        } else {
            n = 0;
        }
    }
    for (i = 0; i + strlen(probe) <= n && !found; i++)
        if (memcmp(buf + i, probe, strlen(probe)) == 0) found = 1;
    return found;
}

__attribute__((constructor)) static void suite_init(void) {
    /* kt_db_path() alone does not create the scratch dir (util.c bug:
     * it uses kt_scratch_path(), not kt_scratch_dir()); kapd then fails
     * with "unable to open database file". mkdir first. */
    kt_scratch_dir();
    if (kt_db_path(g_db, sizeof g_db) != 0) exit(2);
    for (int attempt = 0; attempt < 20; attempt++) {
        g_kapd = start_kapd(NULL);
        if (g_kapd > 0) {
            if (kapd_serves_own_db()) break;
            kt_kapd_stop(g_kapd);
            g_kapd = -1;
        }
    }
    if (g_kapd <= 0) exit(2);
    kt_clipboard_snapshot(&g_saved_clip);
}

__attribute__((destructor)) static void suite_fini(void) {
    kt_clipboard_restore(g_saved_clip);
    kt_kapd_stop(g_kapd);
}

/* ------------------------------------------------------------------ */
/* minimal PNG generator (no zlib: DEFLATE stored blocks)              */

static unsigned int kt_crc32(const unsigned char *data, size_t len) {
    unsigned int crc = 0xFFFFFFFFu;
    size_t i;
    int b;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (b = 0; b < 8; b++)
            crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320u) : (crc >> 1);
    }
    return ~crc;
}

static unsigned int kt_adler32(const unsigned char *data, size_t len) {
    unsigned int a = 1, b = 0;
    size_t i;
    for (i = 0; i < len; i++) {
        a = (a + data[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

/* DEFLATE with stored (uncompressed) blocks; returns bytes written. */
static size_t kt_deflate_stored(unsigned char *out, const unsigned char *raw,
                                size_t rawlen) {
    size_t off = 0, pos = 0;
    while (pos < rawlen) {
        size_t chunk = rawlen - pos;
        if (chunk > 65535) chunk = 65535;
        out[off++] = (unsigned char)((pos + chunk >= rawlen) ? 0x01 : 0x00);
        out[off++] = (unsigned char)(chunk & 0xFF);
        out[off++] = (unsigned char)((chunk >> 8) & 0xFF);
        out[off++] = (unsigned char)((~chunk) & 0xFF);
        out[off++] = (unsigned char)((~chunk >> 8) & 0xFF);
        memcpy(out + off, raw + pos, chunk);
        off += chunk;
        pos += chunk;
    }
    return off;
}

/* Append one PNG chunk (length, type, data, CRC32) at *off. */
static void kt_png_chunk(unsigned char *buf, size_t *off, const char type[4],
                         const unsigned char *data, size_t dlen) {
    unsigned char tmp[512];
    unsigned int crc;
    buf[(*off)++] = (unsigned char)(dlen >> 24);
    buf[(*off)++] = (unsigned char)(dlen >> 16);
    buf[(*off)++] = (unsigned char)(dlen >> 8);
    buf[(*off)++] = (unsigned char)dlen;
    memcpy(tmp, type, 4);
    if (dlen > 0) memcpy(tmp + 4, data, dlen);
    crc = kt_crc32(tmp, dlen + 4);
    memcpy(buf + *off, type, 4);
    *off += 4;
    if (dlen > 0) {
        memcpy(buf + *off, data, dlen);
        *off += dlen;
    }
    buf[(*off)++] = (unsigned char)(crc >> 24);
    buf[(*off)++] = (unsigned char)(crc >> 16);
    buf[(*off)++] = (unsigned char)(crc >> 8);
    buf[(*off)++] = (unsigned char)crc;
}

/* Solid-color RGBA PNG (8-bit, colortype 6). buf must hold the result. */
static void make_png(unsigned char *buf, size_t *len, int w, int h,
                     unsigned char r, unsigned char g, unsigned char b,
                     unsigned char a) {
    static const unsigned char sig[8] = {0x89, 'P', 'N', 'G',
                                         0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char ihdr[13];
    unsigned char raw[1024];
    unsigned char comp[1024];
    size_t off = 0, row, x;
    unsigned int z;
    size_t rawlen = (size_t)h * ((size_t)w * 4 + 1);
    size_t clen;

    for (row = 0; row < (size_t)h; row++) {
        raw[off++] = 0; /* filter: none */
        for (x = 0; x < (size_t)w; x++) {
            raw[off++] = r;
            raw[off++] = g;
            raw[off++] = b;
            raw[off++] = a;
        }
    }
    off = 0;

    ihdr[0] = (unsigned char)(w >> 24); ihdr[1] = (unsigned char)(w >> 16);
    ihdr[2] = (unsigned char)(w >> 8);  ihdr[3] = (unsigned char)w;
    ihdr[4] = (unsigned char)(h >> 24); ihdr[5] = (unsigned char)(h >> 16);
    ihdr[6] = (unsigned char)(h >> 8);  ihdr[7] = (unsigned char)h;
    ihdr[8] = 8;  /* bit depth */
    ihdr[9] = 6;  /* colortype: RGBA */
    ihdr[10] = 0; /* compression */
    ihdr[11] = 0; /* filter */
    ihdr[12] = 0; /* interlace */

    memcpy(buf + off, sig, 8);
    off += 8;
    kt_png_chunk(buf, &off, "IHDR", ihdr, sizeof ihdr);

    clen = kt_deflate_stored(comp + 2, raw, rawlen) + 2;
    comp[0] = 0x78; /* CMF: deflate, 32K window */
    comp[1] = 0x01; /* FLG: FCHECK for 0x7801, stored-only */
    z = kt_adler32(raw, rawlen);
    comp[clen++] = (unsigned char)(z >> 24);
    comp[clen++] = (unsigned char)(z >> 16);
    comp[clen++] = (unsigned char)(z >> 8);
    comp[clen++] = (unsigned char)z;
    kt_png_chunk(buf, &off, "IDAT", comp, clen);

    kt_png_chunk(buf, &off, "IEND", NULL, 0);
    *len = off;
}

/* ------------------------------------------------------------------ */
/* shared fixture: one 8x8 red PNG per run                             */

static unsigned char g_png[1024];
static size_t g_png_len;
static char g_id[64];

/* Run kapc <argv after "kapc">; asserts exit status is 0. */
static char *kapc_run(char *const argv[], size_t *out_len) {
    char *out = NULL;
    size_t len = 0;
    int st = kt_kapc(argv, NULL, 0, &out, &len);
    KT_ASSERT_EQ_INT(st, 0);
    if (out_len != NULL) *out_len = len;
    return out;
}

/* ------------------------------------------------------------------ */
/* tests                                                               */

TEST(image_store_and_paste_roundtrip) {
    char *out = NULL;
    size_t olen = 0;
    char *line = NULL;
    char *tab = NULL;
    size_t idlen = 0;
    char hex1[65], hex2[65];
    int i, digits;

    make_png(g_png, &g_png_len, 8, 8, 255, 0, 0, 255);
    KT_ASSERT(g_png_len > 0 && g_png_len < sizeof g_png);

    /* store: kapc copy -t image/png (stdin = png bytes). Exit status is
     * not asserted: copy forks a clipboard-owner child, so the harness
     * may reap it via the 3s idle timeout (status 0 or 137). */
    {
        char *argv[] = {"kapc", "copy", "-t", "image/png", "-D", g_db, NULL};
        int st = kt_kapc(argv, g_png, g_png_len, &out, &olen);
        free(out);
        out = NULL;
        (void)st;
    }

    /* locate the image entry: search -L, line contains "image/png" */
    {
        char *argv[] = {"kapc", "search", "-L", "-D", g_db, NULL};
        out = kapc_run(argv, &olen);
    }
    if (out == NULL) return;
    line = strstr(out, "image/png");
    KT_ASSERT(line != NULL);
    if (line == NULL) {
        free(out);
        return;
    }
    /* back up to the start of the entry line */
    while (line > out && line[-1] != '\n') line--;
    KT_ASSERT_STR_CONTAINS(line, "image/png");
    /* the -L line must not contain PNG binary bytes (0x89 = sig start) */
    {
        size_t ll = strcspn(line, "\n");
        int has_bin = 0;
        for (i = 0; i < (int)ll; i++)
            if ((unsigned char)line[i] == 0x89) has_bin = 1;
        KT_ASSERT(!has_bin);
    }
    /* extract the id: first tab-delimited field, all digits */
    tab = strchr(line, '\t');
    idlen = tab != NULL ? (size_t)(tab - line) : strlen(line);
    if (idlen >= sizeof g_id) idlen = sizeof g_id - 1;
    memcpy(g_id, line, idlen);
    g_id[idlen] = '\0';
    digits = (int)(idlen > 0);
    for (i = 0; i < (int)idlen; i++)
        if (g_id[i] < '0' || g_id[i] > '9') digits = 0;
    KT_ASSERT(digits != 0);
    free(out);

    /* roundtrip: kapc paste -i <id> must reproduce the exact bytes */
    {
        char *argv[] = {"kapc", "paste", "-i", g_id, "-D", g_db, NULL};
        out = kapc_run(argv, &olen);
    }
    if (out == NULL) return;
    KT_ASSERT_EQ_INT((int)olen, (int)g_png_len);
    if (olen == g_png_len) {
        KT_ASSERT_MEM_EQ(g_png, out, g_png_len);
        kt_sha256_hex(g_png, g_png_len, hex1);
        kt_sha256_hex((const unsigned char *)out, olen, hex2);
        KT_ASSERT_STR_EQ(hex1, hex2);
    }
    free(out);
}

TEST(image_sixel_output) {
    /* kapd is a system singleton: sibling suites that could not start
     * their own kapd write into our db instead, so the newest entry may
     * briefly be a sibling's text. Filter by mime (-t image/png) so we
     * render an image entry regardless of interleaved text polluters;
     * any image renders as sixel (DCS starting ESC 'P', 0x1b 0x50). */
    char *out = NULL;
    size_t olen = 0;
    char *argv[] = {"kapc", "search", "-x", "-t", "image/png", "-l", "1",
                    "-D", g_db, NULL};
    int ok = 0;
    for (int i = 0; i < 12 && !ok; i++) {
        free(out);
        out = kapc_run(argv, &olen);
        ok = (out != NULL && olen >= 2 && (unsigned char)out[0] == 0x1b &&
              out[1] == 'P');
        if (!ok) usleep(300000);
    }
    if (out == NULL) return;
    KT_ASSERT(ok != 0);
    KT_ASSERT(olen >= 2);
    if (olen >= 2) {
        KT_ASSERT((unsigned char)out[0] == 0x1b);
        KT_ASSERT(out[1] == 'P');
    }
    free(out);
}

TEST(image_wl_paste_roundtrip) {
    /* The PNG copied by image_store_and_paste_roundtrip is offered on the
     * live Wayland clipboard by a forked owner child that outlives the
     * copy command itself. wl-paste --type image/png must return the exact
     * original bytes. Sibling suites may briefly own the clipboard, so
     * retry; if the offer is gone, refresh it with `kapc copy -i <id>`. */
    char *argv_wl[] = {"wl-paste", "--type", "image/png", NULL};
    char *argv_re[] = {"kapc", "copy", "-i", g_id, "-D", g_db, NULL};
    int ok = 0;

    for (int pass = 0; pass < 2 && !ok; pass++) {
        for (int i = 0; i < 3 && !ok; i++) {
            char *out = NULL;
            size_t olen = 0;
            int st = kt_kapc(argv_wl, NULL, 0, &out, &olen);
            if (st == 0 && olen == g_png_len &&
                memcmp(g_png, out, g_png_len) == 0) {
                ok = 1;
            }
            free(out);
            if (!ok) usleep(300000);
        }
        if (!ok && g_id[0] != '\0') {
            char *out = NULL;
            size_t olen = 0;
            int st = kt_kapc(argv_re, NULL, 0, &out, &olen);
            free(out);
            (void)st;
        }
    }
    KT_ASSERT(ok != 0);
}

KT_MAIN()
