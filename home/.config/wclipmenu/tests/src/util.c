/* util.c - runtime for the kaprica CLI integration-test harness.
 *
 * libc only. Every kapc/kapd invocation goes against an isolated scratch
 * database under /tmp/kaprica-tests-<pid>/; the user's real history db at
 * ~/.local/share/kaprica/history.db is never touched.
 *
 * Spawn discipline:
 *   - kapc stdout is read with an idle select() timeout (~3s). `kapc copy`
 *     (without -f) forks a clipboard-owner child that inherits stdout; if
 *     the parent never closes stdout we SIGKILL it instead of hanging.
 *   - kt_cleanup (atexit) kills recorded children, then scans
 *     /proc/<pid>/cmdline for anything referencing the scratch dir (catches
 *     forked clipboard owners), then removes the scratch dir.
 */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "util.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define KT_KAPC_IDLE_MS 3000   /* idle select() timeout while reading kapc */
#define KT_READY_TIMEOUT 5     /* seconds to wait for kapd readiness */

/* ------------------------------------------------------------------ */
/* scratch dir                                                         */

static char g_scratch[128];
static int g_scratch_ready = 0;

static const char *kt_scratch_path(void) {
    if (!g_scratch_ready) {
        snprintf(g_scratch, sizeof g_scratch, "/tmp/kaprica-tests-%ld",
                 (long)getpid());
        g_scratch_ready = 1;
    }
    return g_scratch;
}

const char *kt_scratch_dir(void) {
    const char *p = kt_scratch_path();
    if (mkdir(p, 0700) != 0 && errno != EEXIST) {
        fprintf(stderr, "kt: cannot create scratch dir %s: %s\n", p,
                strerror(errno));
    }
    return p;
}

int kt_db_path(char *buf, size_t n) {
    int r = snprintf(buf, n, "%s/history.db", kt_scratch_path());
    if (r < 0 || (size_t)r >= n) return -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* child bookkeeping (for kt_cleanup)                                  */

#define KT_MAX_CHILDREN 128
static pid_t g_children[KT_MAX_CHILDREN];
static int g_nchildren = 0;

static void kt_remember_pid(pid_t pid) {
    if (g_nchildren < KT_MAX_CHILDREN) g_children[g_nchildren++] = pid;
}

/* ------------------------------------------------------------------ */
/* process runner                                                      */

struct kt_run {
    int status;        /* WEXITSTATUS, or 128+WTERMSIG for signal deaths */
    char *out;         /* malloc'd, NUL-terminated */
    size_t out_len;
};

/* Spawn argv[0]; optionally feed stdin_data (write-then-close); capture
 * stdout with an idle select() timeout. On idle timeout the child is
 * SIGKILLed. Returns the child pid (>= 0) or -1; run->status holds the
 * child's status, run->out/out_len the captured stdout. */
static pid_t kt_run_capture(char *const argv[], const void *stdin_data,
                            size_t stdin_len, long idle_ms,
                            struct kt_run *run) {
    int outpipe[2];
    int inpipe[2];
    int have_in = (stdin_data != NULL);

    if (pipe(outpipe) != 0) return -1;
    if (have_in && pipe(inpipe) != 0) {
        close(outpipe[0]);
        close(outpipe[1]);
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(outpipe[0]);
        close(outpipe[1]);
        if (have_in) {
            close(inpipe[0]);
            close(inpipe[1]);
        }
        return -1;
    }

    if (pid == 0) {
        /* child */
        int dn = open("/dev/null", O_RDWR);
        if (dn < 0) _exit(127);
        if (have_in) {
            close(inpipe[1]);
            if (dup2(inpipe[0], STDIN_FILENO) < 0) _exit(127);
            close(inpipe[0]);
        } else {
            if (dup2(dn, STDIN_FILENO) < 0) _exit(127);
        }
        close(outpipe[0]);
        if (dup2(outpipe[1], STDOUT_FILENO) < 0) _exit(127);
        close(outpipe[1]);
        if (dn > 2) close(dn);
        execvp(argv[0], argv);
        fprintf(stderr, "kt: exec %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    /* parent */
    close(outpipe[1]);
    if (have_in) {
        close(inpipe[0]);
        size_t off = 0;
        while (off < stdin_len) {
            ssize_t w = write(inpipe[1],
                              (const char *)stdin_data + off, stdin_len - off);
            if (w < 0) {
                if (errno == EINTR) continue;
                break; /* EPIPE: child closed stdin early */
            }
            off += (size_t)w;
        }
        close(inpipe[1]);
    }
    kt_remember_pid(pid);

    /* capture stdout with an idle select() timeout */
    size_t cap = 8192;
    size_t n = 0;
    char *buf = (char *)malloc(cap);
    int kill_child = (buf == NULL) ? 1 : 0;
    while (buf != NULL) {
        fd_set rf;
        FD_ZERO(&rf);
        FD_SET(outpipe[0], &rf);
        struct timeval tv;
        tv.tv_sec = idle_ms / 1000;
        tv.tv_usec = (idle_ms % 1000) * 1000;
        int s = select(outpipe[0] + 1, &rf, NULL, NULL, &tv);
        if (s < 0) {
            if (errno == EINTR) continue;
            kill_child = 1;
            break;
        }
        if (s == 0) { /* idle timeout: kill the spawned child */
            kill_child = 1;
            break;
        }
        if (n + 4096 > cap - 1) {
            size_t ncap = cap * 2;
            char *nb = (char *)realloc(buf, ncap);
            if (nb == NULL) {
                kill_child = 1;
                break;
            }
            buf = nb;
            cap = ncap;
        }
        ssize_t rn = read(outpipe[0], buf + n, cap - n - 1);
        if (rn < 0) {
            if (errno == EINTR) continue;
            kill_child = 1;
            break;
        }
        if (rn == 0) break; /* EOF */
        n += (size_t)rn;
    }
    if (kill_child) kill(pid, SIGKILL);
    close(outpipe[0]);

    if (buf == NULL) {
        int st;
        while (waitpid(pid, &st, 0) < 0 && errno == EINTR) {}
        run->out = NULL;
        run->out_len = 0;
        run->status = -1;
        return -1;
    }
    buf[n] = '\0';

    int st = 0;
    while (waitpid(pid, &st, 0) < 0) {
        if (errno != EINTR) break;
    }
    run->out = buf;
    run->out_len = n;
    if (WIFEXITED(st)) run->status = WEXITSTATUS(st);
    else if (WIFSIGNALED(st)) run->status = 128 + WTERMSIG(st);
    else run->status = -1;
    return pid;
}

int kt_kapc(char *const argv[], const void *stdin_data, size_t stdin_len,
            char **out, size_t *out_len) {
    struct kt_run run;
    run.out = NULL;
    run.out_len = 0;
    run.status = -1;
    pid_t pid = kt_run_capture(argv, stdin_data, stdin_len, KT_KAPC_IDLE_MS,
                               &run);
    if (pid < 0) {
        free(run.out);
        if (out) *out = NULL;
        if (out_len) *out_len = 0;
        return -1;
    }
    if (out) *out = run.out;
    else free(run.out);
    if (out_len) *out_len = run.out_len;
    return run.status;
}

int kt_kapc_simple(char *const argv[], char **out, size_t *out_len) {
    return kt_kapc(argv, NULL, 0, out, out_len);
}

/* ------------------------------------------------------------------ */
/* clipboard                                                           */

int kt_clipboard_snapshot(char **text) {
    char *args[] = { "wl-paste", "--no-newline", NULL };
    struct kt_run run;
    run.out = NULL;
    run.out_len = 0;
    pid_t pid = kt_run_capture(args, NULL, 0, KT_KAPC_IDLE_MS, &run);
    if (pid < 0) {
        free(run.out);
        return -1;
    }
    if (run.status == 0) {
        *text = run.out;
        return 0;
    }
    free(run.out);
    if (run.status == 1) return 1; /* empty clipboard */
    return -1;
}

int kt_clipboard_restore(const char *text) {
    if (text == NULL) return -1;
    int pin[2];
    if (pipe(pin) != 0) return -1;
    pid_t pid = fork();
    if (pid < 0) {
        close(pin[0]);
        close(pin[1]);
        return -1;
    }
    if (pid == 0) {
        /* child: wl-copy, stdin from pipe, stdout/stderr to /dev/null */
        close(pin[1]);
        if (dup2(pin[0], STDIN_FILENO) < 0) _exit(127);
        close(pin[0]);
        int dn = open("/dev/null", O_RDWR);
        if (dn >= 0) {
            dup2(dn, STDOUT_FILENO);
            dup2(dn, STDERR_FILENO);
            if (dn > 2) close(dn);
        }
        char *args[] = { "wl-copy", NULL };
        execvp(args[0], args);
        _exit(127);
    }
    close(pin[0]);
    size_t len = strlen(text);
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(pin[1], text + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            break;
        }
        off += (size_t)w;
    }
    close(pin[1]);
    /* wl-copy either forks a clipboard-owner child and exits, or stays in
     * the foreground owning the clipboard itself. Either way it has our
     * text; wait briefly for the parent, then leave it alone. */
    for (int i = 0; i < 50; i++) {
        int st;
        pid_t r = waitpid(pid, &st, WNOHANG);
        if (r == pid) return 0;
        if (r < 0 && errno == ECHILD) return 0;
        usleep(20000);
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* SHA-256 (hand-rolled, FIPS 180-4, public-domain style)              */

static const uint32_t K256[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

static inline uint32_t kt_rotr32(uint32_t x, unsigned n) {
    return (x >> n) | (x << (32u - n));
}

static void kt_sha256_block(uint32_t H[8], const unsigned char *p) {
    uint32_t w[64];
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)p[i * 4] << 24) | ((uint32_t)p[i * 4 + 1] << 16) |
               ((uint32_t)p[i * 4 + 2] << 8) | (uint32_t)p[i * 4 + 3];
    }
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = kt_rotr32(w[i - 15], 7) ^ kt_rotr32(w[i - 15], 18) ^
                      (w[i - 15] >> 3);
        uint32_t s1 = kt_rotr32(w[i - 2], 17) ^ kt_rotr32(w[i - 2], 19) ^
                      (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
    uint32_t e = H[4], f = H[5], g = H[6], h = H[7];
    for (int i = 0; i < 64; i++) {
        uint32_t S1 = kt_rotr32(e, 6) ^ kt_rotr32(e, 11) ^ kt_rotr32(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t t1 = h + S1 + ch + K256[i] + w[i];
        uint32_t S0 = kt_rotr32(a, 2) ^ kt_rotr32(a, 13) ^ kt_rotr32(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = S0 + maj;
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    H[0] += a; H[1] += b; H[2] += c; H[3] += d;
    H[4] += e; H[5] += f; H[6] += g; H[7] += h;
}

void kt_sha256_hex(const unsigned char *data, size_t len, char out[65]) {
    uint32_t H[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };
    size_t off = 0;
    while (off + 64 <= len) {
        kt_sha256_block(H, data + off);
        off += 64;
    }

    /* tail: remaining bytes + 0x80 + zeros until 56 mod 64 + 8-byte length */
    unsigned char tail[128];
    size_t rem = len - off;
    memcpy(tail, data + off, rem);
    tail[rem] = 0x80u;
    size_t tl = rem + 1;
    while ((tl % 64) != 56) tail[tl++] = 0x00u;
    uint64_t bits = (uint64_t)len * 8u;
    for (int i = 7; i >= 0; i--) {
        tail[tl + i] = (unsigned char)(bits & 0xffu);
        bits >>= 8;
    }
    tl += 8;
    for (size_t i = 0; i < tl; i += 64) kt_sha256_block(H, tail + i);

    static const char hexd[] = "0123456789abcdef";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            unsigned char byte = (unsigned char)(H[i] >> (24 - 8 * j));
            out[i * 8 + j * 2] = hexd[byte >> 4];
            out[i * 8 + j * 2 + 1] = hexd[byte & 0x0fu];
        }
    }
    out[64] = '\0';
}

/* ------------------------------------------------------------------ */
/* kapd lifecycle                                                      */

static int kt_kapd_running(void) {
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

static int kt_wait_kapd_ready(pid_t pid, const char *db_path) {
    time_t t0 = time(NULL);
    while (time(NULL) - t0 < KT_READY_TIMEOUT) {
        int st;
        pid_t r = waitpid(pid, &st, WNOHANG);
        if (r == pid || (r < 0 && errno == ECHILD)) return 0; /* died early */
        char *out = NULL;
        size_t out_len = 0;
        char *args[] = { "/usr/local/bin/kapc", "-D", (char *)db_path,
                         "search", "-l", "1", NULL };
        int s = kt_kapc_simple(args, &out, &out_len);
        free(out);
        if (s == 0) return 1;
        usleep(200000);
    }
    return 0;
}

pid_t kt_kapd_start(const char *db_path, char *const extra_args[]) {
    if (kt_kapd_running()) {
        fprintf(stderr, "another kapd is running; stop it before running tests\n");
        return -1;
    }
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        /* child: exec kapd with stdin/stdout detached, stderr kept */
        char *argv[64];
        int n = 0;
        argv[n++] = (char *)"kapd";
        argv[n++] = (char *)"-D";
        argv[n++] = (char *)db_path;
        if (extra_args != NULL) {
            for (int i = 0; extra_args[i] != NULL && n < 62; i++)
                argv[n++] = extra_args[i];
        }
        argv[n] = NULL;
        int dn = open("/dev/null", O_RDWR);
        if (dn >= 0) {
            dup2(dn, STDIN_FILENO);
            dup2(dn, STDOUT_FILENO);
            if (dn > 2) close(dn);
        }
        execv("/usr/local/bin/kapd", argv);
        fprintf(stderr, "kt: exec kapd: %s\n", strerror(errno));
        _exit(127);
    }
    kt_remember_pid(pid);
    usleep(100000);
    if (!kt_wait_kapd_ready(pid, db_path)) {
        kill(pid, SIGKILL);
        while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {}
        return -1;
    }
    return pid;
}

void kt_kapd_stop(pid_t pid) {
    if (pid <= 0) return;
    kill(pid, SIGTERM);
    for (int i = 0; i < 100; i++) { /* 10ms x 100 = 1s */
        int st;
        pid_t r = waitpid(pid, &st, WNOHANG);
        if (r == pid) return;
        if (r < 0 && errno == ECHILD) return;
        usleep(10000);
    }
    kill(pid, SIGKILL);
    while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {}
}

/* ------------------------------------------------------------------ */
/* cleanup                                                             */

static void kt_kill_matching(const char *needle, int sig) {
    DIR *d = opendir("/proc");
    if (d == NULL) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (!isdigit((unsigned char)e->d_name[0])) continue;
        long lpid = strtol(e->d_name, NULL, 10);
        if (lpid <= 0 || (pid_t)lpid == getpid()) continue;
        char path[64];
        snprintf(path, sizeof path, "/proc/%ld/cmdline", lpid);
        FILE *f = fopen(path, "r");
        if (f == NULL) continue;
        char buf[4096];
        size_t n = fread(buf, 1, sizeof buf - 1, f);
        fclose(f);
        if (n == 0) continue;
        buf[n] = '\0';
        for (size_t i = 0; i < n; i++)
            if (buf[i] == '\0') buf[i] = ' ';
        if (strstr(buf, needle) != NULL) {
            kill((pid_t)lpid, sig);
        }
    }
    closedir(d);
}

void kt_cleanup(void) {
    static int done = 0;
    if (done) return;
    done = 1;

    /* 1. kill recorded children (kapc/kapd we spawned) */
    for (int i = 0; i < g_nchildren; i++) {
        if (g_children[i] > 0) kill(g_children[i], SIGKILL);
    }

    /* 2. kill orphans whose cmdline references the scratch dir
     *    (forked clipboard owners keep the -D <scratch> argv) */
    const char *scratch = kt_scratch_path();
    kt_kill_matching(scratch, SIGTERM);
    usleep(100000);
    kt_kill_matching(scratch, SIGKILL);

    /* 3. remove the scratch dir */
    char *args[] = { "rm", "-rf", (char *)scratch, NULL };
    pid_t pid = fork();
    if (pid == 0) {
        int dn = open("/dev/null", O_RDWR);
        if (dn >= 0) {
            dup2(dn, STDOUT_FILENO);
            dup2(dn, STDERR_FILENO);
            if (dn > 2) close(dn);
        }
        execvp("rm", args);
        _exit(127);
    }
    if (pid > 0) {
        for (int i = 0; i < 50; i++) { /* up to ~1s */
            int st;
            pid_t r = waitpid(pid, &st, WNOHANG);
            if (r == pid || (r < 0 && errno == ECHILD)) return;
            usleep(20000);
        }
        kill(pid, SIGKILL);
        while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {}
    }
}

#if defined(__GNUC__)
__attribute__((constructor))
static void kt_util_ctor(void) {
    signal(SIGPIPE, SIG_IGN);
    atexit(kt_cleanup);
}
#endif
